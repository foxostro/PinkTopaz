//
//  Terrain.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#include "Terrain/Terrain.hpp"
#include "Terrain/MesherNaiveSurfaceNets.hpp"
#include "Terrain/MapRegionStore.hpp"
#include "Terrain/VoxelData.hpp"
#include "Profiler.hpp"
#include "Grid/GridIndexerRange.hpp"
#include "Grid/FrustumRange.hpp"
#include "Grid/Array3D.hpp"
#include "Renderer/TextureArrayLoader.hpp"
#include "FileUtilities.hpp"
#include <sstream>


// Random seed to use for a new journal.
constexpr unsigned InitialVoxelDataSeed = 52;

// Set to true to enable frustum culling while drawing terrain.
// Measurements show that the time spent culling -- and, specifically, time
// spent accessing elements of the grid via the get() method -- is
// counterproductive. So, this is disabled by default.
constexpr bool EnableFrustumCulling = false;


Terrain::~Terrain()
{
    _dispatcherHighPriority->shutdown();
    _meshRebuildActor.reset();
    _dispatcherVoxelData->shutdown();
}

Terrain::Terrain(const Preferences &preferences,
                 std::shared_ptr<spdlog::logger> log,
                 const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                 const std::shared_ptr<TaskDispatcher> &mainThreadDispatcher,
                 entityx::EventManager &events,
                 glm::vec3 initialCameraPosition)
 : _graphicsDevice(graphicsDevice),
   _mesher(std::make_shared<MesherNaiveSurfaceNets>(preferences)),
   _cameraPosition(initialCameraPosition),
   _log(log),
   _activeRegionSize(preferences.activeRegionSize),
   _startTime(std::chrono::steady_clock::now()),
   _drawListNeedsRebuild(false)
{
    const unsigned numberOfHardwareThreads = std::max(1u, std::thread::hardware_concurrency());
    
    _dispatcherHighPriority = std::make_shared<TaskDispatcher>("Terrain High Priority TaskDispatcher", 1);
    _dispatcherVoxelData = std::make_shared<TaskDispatcher>("Voxel Data TaskDispatcher", numberOfHardwareThreads);
    
    // Load terrain texture array from a single image.
    TextureArrayLoader textureArrayLoader(graphicsDevice);
    auto texture = textureArrayLoader.load("terrain.png");
    
    TextureSamplerDescriptor samplerDesc = {
        ClampToEdge,
        ClampToEdge,
        NearestMipMapNearest,
        Nearest
    };
    auto sampler = graphicsDevice->makeTextureSampler(samplerDesc);
    
    StaticMesh mesh; // An empty mesh still has a valid vertex format.
    auto shader = _graphicsDevice->makeShader(mesh.getVertexFormat(),
                                              "vert", "frag",
                                              false);
    
    TerrainUniforms uniforms;
    auto uniformBuffer = _graphicsDevice->makeBuffer(sizeof(uniforms),
                                                     &uniforms,
                                                     DynamicDraw,
                                                     UniformBuffer);
    uniformBuffer->addDebugMarker("Terrain Uniforms", 0, sizeof(uniforms));
    
    // We don't have vertices until the isosurface is extracted later.
    _defaultMesh = std::make_shared<RenderableStaticMesh>();
    _defaultMesh->vertexCount = 0;
    _defaultMesh->buffer = nullptr;
    _defaultMesh->uniforms = uniformBuffer;
    _defaultMesh->shader = shader;
    _defaultMesh->texture = texture;
    _defaultMesh->textureSampler = sampler;
    
    const boost::filesystem::path prefPath = getPrefPath();
    const boost::filesystem::path journalFileName = prefPath / "journal.xml";
    const boost::filesystem::path mapDirectory = prefPath / "Map";
    
    // If the journal already exists then use the seed that it provides.
    // Else, set an initial seed value ourselves.
    bool mustSetSeed = !boost::filesystem::exists(journalFileName);
    _journal = std::make_unique<TerrainJournal>(_log, journalFileName);
    if (mustSetSeed) {
        _journal->setVoxelDataSeed(InitialVoxelDataSeed);
    }
    
    // Create the map directory. If it does not exist then build the map from
    // the journal. This will allow us to avoid shipping large map region files
    // with the game. We can ship only the journal instead.
    bool mustRegenerateMap = false;
    if (boost::filesystem::create_directories(mapDirectory)) {
        mustRegenerateMap = true;
    }
    
    _voxels = createVoxelData(_dispatcherVoxelData,
                              _journal->getVoxelDataSeed(),
                              mapDirectory);
    
    const AABB meshGridBoundingBox = _voxels->boundingBox().inset(glm::vec3((float)TERRAIN_CHUNK_SIZE, (float)TERRAIN_CHUNK_SIZE, (float)TERRAIN_CHUNK_SIZE));
    const glm::ivec3 meshGridResolution = _voxels->countCellsInRegion(meshGridBoundingBox) / (int)TERRAIN_CHUNK_SIZE;
    
    // Limit the number of meshes that can be in memory at once.
    // GPU resources are relatively scarce so don't hold onto these
    // forever.
    _meshes = std::make_unique<TerrainMeshGrid>(meshGridBoundingBox, meshGridResolution);
    
    const unsigned workingSetCount = std::pow(1 + 2*_activeRegionSize / TERRAIN_CHUNK_SIZE, 3);
    _meshes->setCountLimit(2*workingSetCount);
    
    // Setup an actor to rebuild chunks.
    _meshRebuildActor = std::make_unique<TerrainRebuildActor>(_log,
                                                              numberOfHardwareThreads,
                                                              _cameraPosition,
                                                              mainThreadDispatcher,
                                                              events,
                                                              _startTime,
                                                              [=](const TerrainRebuildActor::Batch &batch){
                                                                  rebuildNextMeshBatch(batch);
                                                              });
    
    // When voxels change, we need to extract a polygonal mesh representation
    // of the isosurface. This mesh is what we actually draw.
    // For now, we extract the entire isosurface in one step.
    _voxels->onWriterTransaction.connect([&](const AABB &affectedRegion){
        rebuildMeshInResponseToChanges(affectedRegion);
    });
    
    // Build the map from the journal, if necessary.
    if (mustRegenerateMap) {
        _log->info("Rebuilding the map from the terrain journal.");
        _journal->replay([&](const std::shared_ptr<TerrainOperation> &operation){
            _voxels->writerTransaction(*operation);
        });
    }
    
    // Setup some empty draw lists and request these be rebuilt soon.
    _frontDrawList = std::make_unique<UnlockedSparseGrid<RenderableStaticMesh>>(_meshes->boundingBox(), _meshes->gridResolution());
    _backDrawList  = std::make_unique<UnlockedSparseGrid<RenderableStaticMesh>>(_meshes->boundingBox(), _meshes->gridResolution());
    requestDrawListRebuild();
}

void Terrain::update(entityx::TimeDelta dt)
{
    _horizonDistance.update(dt);
}

void Terrain::setTerrainUniforms(const TerrainUniforms &uniforms)
{
    // The uniforms referenced in the default mesh are also referenced by other
    // meshes in the terrain. Setting it once here sets it for all of them.
    _defaultMesh->uniforms->replace(sizeof(uniforms), &uniforms);
    
    // Extract the camera position from the camera transform.
    const glm::vec3 cameraPos = glm::vec3(glm::inverse(uniforms.view)[3]);
    _cameraPosition = cameraPos;
    _dispatcherHighPriority->async([this]{
        _meshRebuildActor->setSearchPoint(_cameraPosition);
    });
    
    // We'll use the MVP later to extract the camera frustum.
    {
        glm::mat4x4 modelViewProjection = uniforms.proj * uniforms.view;
        if (modelViewProjection != _modelViewProjection) {
            _modelViewProjection = modelViewProjection;
            requestDrawListRebuild();
        }
    }
}

void Terrain::draw(const std::shared_ptr<CommandEncoder> &encoder)
{
    std::scoped_lock lock(_lockFrontDrawList);
    
    encoder->setShader(_defaultMesh->shader);
    encoder->setFragmentSampler(_defaultMesh->textureSampler, 0);
    encoder->setFragmentTexture(_defaultMesh->texture, 0);
    encoder->setVertexBuffer(_defaultMesh->uniforms, 1);
    
    auto drawChunk = [&](const RenderableStaticMesh &renderable){
        if (renderable.vertexCount > 0) {
            encoder->setVertexBuffer(renderable.buffer, 0);
            encoder->drawPrimitives(Triangles, 0, renderable.vertexCount, 1);
        }
    };
    
    if constexpr (EnableFrustumCulling) {
        const Frustum frustum(_modelViewProjection);
        const AABB activeRegion = getActiveRegion();
        
        for (const glm::ivec3 &cellCoords : slice(*_frontDrawList, frustum, activeRegion)) {
            auto maybeRenderable = _frontDrawList->get(cellCoords);
            if (maybeRenderable) {
                drawChunk(*maybeRenderable);
            }
        }
    } else {
        for (const auto& [key, renderable] : *_frontDrawList) {
            drawChunk(renderable);
        }
    }
}

float Terrain::getFogDensity() const
{
    // Fog density decreases as the terrain horizon distance increases.
    // The density falls along an exponential decay curve. These particular
    // tuning values were picked because they look pretty good.
    constexpr float initialValue = 0.05f;
    constexpr float floorValue = 0.003f;
    constexpr float decayRate = -32.f;
    const float maxHorizon = glm::length(_voxels->boundingBox().extent);
    const float horizonDistance = _horizonDistance.get();
    const float t = std::min(horizonDistance / maxHorizon, 1.0f);
    const float fogDensity = initialValue * std::exp(decayRate * t) + floorValue;
    return fogDensity;
}

void Terrain::readerTransaction(const AABB &region, std::function<void(Array3D<Voxel> &&data)> fn)
{
    _voxels->readerTransaction(region, fn);
}

void Terrain::writerTransaction(const std::shared_ptr<TerrainOperation> &operation)
{
    assert(operation);
    _journal->add(operation);
    _voxels->writerTransaction(*operation);
}

void Terrain::rebuildMeshInResponseToChanges(const AABB &voxelAffectedRegion)
{
    PROFILER(TerrainRebuildMeshInResponseToChanges);
    
    // We have the region of voxels affected by the change. From this, compute
    // the associated region where the change may have invalidated meshes too..
    const glm::vec3 meshChunkSize((float)TERRAIN_CHUNK_SIZE);
    const AABB meshAffectedRegion = _meshes->boundingBox().intersect(voxelAffectedRegion.inset(-meshChunkSize));
    
    // Kick off a task to rebuild each affected mesh in the active region.
    // Meshes outside the active region are invalidated instead.
    const AABB activeRegion = getActiveRegion();
    std::vector<std::pair<Morton3, AABB>> meshCellsToRebuild;
    for (const auto cellCoords : slice(*_meshes, meshAffectedRegion)) {
        const AABB cell = _meshes->cellAtCellCoords(cellCoords);
        const Morton3 index = _meshes->indexAtCellCoords(cellCoords);
        if (doBoxesIntersect(cell, activeRegion)) {
            meshCellsToRebuild.emplace_back(index, cell);
        } else {
            _meshes->remove(index);
        }
    }
    _meshRebuildActor->push(meshCellsToRebuild);
}

void Terrain::rebuildNextMeshBatch(const TerrainRebuildActor::Batch &batch)
{
    PROFILER(TerrainRebuildNextMesh);
    
    const std::vector<TerrainRebuildActor::Cell> &requestedCells = batch.requestedCells();
    
    std::vector<AABB> voxelBoxes;
    voxelBoxes.reserve(requestedCells.size());
    
    for (const TerrainRebuildActor::Cell &cell : requestedCells) {
        cell.progress.setState(TerrainProgressEvent::WaitingOnVoxels);
        
        // We need a border of voxels around the region of the mesh in order to
        // perform surface extraction.
        const AABB voxelBox = cell.box.inset(-2.f * _voxels->cellDimensions());
        
        voxelBoxes.push_back(voxelBox);
    }
    
    _voxels->readerTransaction(voxelBoxes, [&](size_t index, Array3D<Voxel> &&voxels){
        const TerrainRebuildActor::Cell &cell = requestedCells.at(index);
        auto terrainMesh = std::make_shared<TerrainMesh>(cell.box, _defaultMesh, _graphicsDevice, _mesher);
        terrainMesh->rebuild(voxels, cell.progress);
        _meshes->set(cell.box.center, terrainMesh);
    });
    
    requestDrawListRebuild();
}

std::unique_ptr<TransactedVoxelData>
Terrain::createVoxelData(const std::shared_ptr<TaskDispatcher> &dispatcherVoxelData,
                         unsigned voxelDataSeed,
                         const boost::filesystem::path &mapDirectory)
{
    // First, we need a voxel data generator to create terrain from noise.
    auto generator = std::make_unique<VoxelDataGenerator>(voxelDataSeed);
    
    // Next, setup a map file on disk to record the shape of the terrain.
    const auto mapRegionBox = generator->boundingBox();
    const auto mapRegionRes = generator->countCellsInRegion(mapRegionBox) / (int)MAP_REGION_SIZE;
    auto mapRegionStore = std::make_unique<MapRegionStore>(_log, mapDirectory, mapRegionBox, mapRegionRes);
    
    // The sunlight data object stores the terrain shape plus sunlight.
    auto voxelData = std::make_unique<VoxelData>(_log,
                                                 std::move(generator),
                                                 TERRAIN_CHUNK_SIZE,
                                                 std::move(mapRegionStore),
                                                 dispatcherVoxelData);
    
    // Wrap in a TransactedVoxelData to implement the locking policy.
    return std::make_unique<TransactedVoxelData>(std::move(voxelData));
}

void Terrain::requestDrawListRebuild()
{
    auto rebuildIfNecessary = [=]{
        bool needsRebuild;
        {
            std::scoped_lock lock(_lockDrawListNeedsRebuild);
            needsRebuild = _drawListNeedsRebuild;
            _drawListNeedsRebuild = false;
        }
        if (needsRebuild) {
            rebuildDrawList();
        }
    };
    
    std::scoped_lock lock(_lockDrawListNeedsRebuild);
    if (!_drawListNeedsRebuild) {
        _drawListNeedsRebuild = true;
        _dispatcherHighPriority->async(rebuildIfNecessary);
    }
}

void Terrain::rebuildDrawList()
{
    const AABB activeRegion = getActiveRegion();
    TerrainMeshGrid &meshes = *_meshes;
    
    // Get meshes in the active region.
    // Figure out which meshes in the active region are missing.
    _backDrawList->clear();
    std::vector<std::pair<Morton3, AABB>> missingMeshes;
    for (const glm::ivec3 cellCoords : slice(meshes, activeRegion)) {
        const Morton3 index = meshes.indexAtCellCoords(cellCoords);
        const auto maybeTerrainMeshPtr = meshes.get(index);
        if (maybeTerrainMeshPtr) {
            _backDrawList->set(index, (*maybeTerrainMeshPtr)->getMesh());
        } else {
            missingMeshes.emplace_back(index, meshes.cellAtCellCoords(cellCoords));
        }
    }
    
    // Swap
    {
        std::scoped_lock lock(_lockFrontDrawList);
        std::swap(_backDrawList, _frontDrawList);
    }
    
    // If no meshes were missing in the active region then increase the horizon
    // distance so we can draw meshes further away next time.
    // Otherwise, queue the meshes to be fetched asynchronously so we can draw
    // them later. Meshes can disappear at any time due to cache purges. So,
    // we have to do this everytime we need a mesh and can't find it.
    if (0 == missingMeshes.size()) {
        auto [distance, didChange] = _horizonDistance.increment_clamp(_activeRegionSize);
        if (didChange) {
            _log->info("Increasing horizon distance to {}", distance);
        }
    } else {
        _log->trace("There are {} missing meshes in active region {}.",
                    missingMeshes.size(), activeRegion);
        _meshRebuildActor->push(missingMeshes);
    }
}
