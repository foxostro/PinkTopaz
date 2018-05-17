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
#include "Profiler.hpp"
#include "Grid/GridIndexerRange.hpp"
#include "Grid/FrustumRange.hpp"
#include "Grid/Array3D.hpp"
#include "Renderer/TextureArrayLoader.hpp"
#include "FileUtilities.hpp"
#include <sstream>


// Random seed to use for a new journal.
constexpr unsigned InitialVoxelDataSeed = 52;


Terrain::~Terrain()
{
    _meshRebuildActor.reset();
    _dispatcher->shutdown();
}

Terrain::Terrain(std::shared_ptr<spdlog::logger> log,
                 const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                 const std::shared_ptr<TaskDispatcher> &dispatcher,
                 const std::shared_ptr<TaskDispatcher> &mainThreadDispatcher,
                 entityx::EventManager &events,
                 glm::vec3 initialCameraPosition)
 : _graphicsDevice(graphicsDevice),
   _dispatcher(dispatcher),
   _mesher(std::make_shared<MesherNaiveSurfaceNets>()),
   _cameraPosition(initialCameraPosition),
   _log(log)
{
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
    const boost::filesystem::path voxelsDirectory = mapDirectory / "Voxels";
    const boost::filesystem::path sunlightDirectory = mapDirectory / "Sunlight";
    
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
    if (boost::filesystem::create_directories(voxelsDirectory)) {
        mustRegenerateMap = true;
    }
    if (boost::filesystem::create_directories(sunlightDirectory)) {
        mustRegenerateMap = true;
    }
    
    _voxels = createVoxelData(_journal->getVoxelDataSeed(),
                              voxelsDirectory,
                              sunlightDirectory);
    
    const AABB meshGridBoundingBox = _voxels->boundingBox().inset(glm::vec3((float)TERRAIN_CHUNK_SIZE, (float)TERRAIN_CHUNK_SIZE, (float)TERRAIN_CHUNK_SIZE));
    const glm::ivec3 meshGridResolution = _voxels->countCellsInRegion(meshGridBoundingBox) / (int)TERRAIN_CHUNK_SIZE;
    _meshes = std::make_unique<TerrainMeshGrid>(meshGridBoundingBox, meshGridResolution);

    // Limit the sparse grids to the number of chunks in the active region.
    // Now, the active region can change size over time but it will never be
    // larger than this size.
    _voxels->setWorkingSet({glm::vec3(0.f), glm::vec3((float)ACTIVE_REGION_SIZE + TERRAIN_CHUNK_SIZE)});
    
    const unsigned workingSetCount = std::pow(1 + 2*ACTIVE_REGION_SIZE / TERRAIN_CHUNK_SIZE, 3);
    _meshes->setCountLimit(2*workingSetCount);
    
    // Setup an actor to rebuild chunks.
    const unsigned numMeshRebuildThreads = 2*std::max(1u, std::thread::hardware_concurrency());
    _meshRebuildActor = std::make_unique<TerrainRebuildActor>(_log,
                                                              numMeshRebuildThreads,
                                                              _cameraPosition,
                                                              mainThreadDispatcher,
                                                              events,
                                                              [=](const AABB &cell, TerrainProgressTracker &progress){
                                                                  rebuildNextMesh(cell, progress);
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
            _voxels->writerTransaction(operation);
        });
    }
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
    _dispatcher->async([this]{
        if (_meshRebuildActor) {
            _meshRebuildActor->setSearchPoint(_cameraPosition);
        }
    });
    
    // We'll use the MVP later to extract the camera frustum.
    _modelViewProjection = uniforms.proj * uniforms.view;
}

void Terrain::draw(const std::shared_ptr<CommandEncoder> &encoder)
{
    Frustum frustum(_modelViewProjection);
    
    encoder->setShader(_defaultMesh->shader);
    encoder->setFragmentSampler(_defaultMesh->textureSampler, 0);
    encoder->setFragmentTexture(_defaultMesh->texture, 0);
    encoder->setVertexBuffer(_defaultMesh->uniforms, 1);
    
    const AABB activeRegion = getActiveRegion();
    TerrainMeshGrid &meshes = *_meshes;
    
    // Draw meshes in the camera frustum.
    for (const glm::ivec3 cellCoords : slice(meshes, frustum, activeRegion)) {
        const Morton3 index = meshes.indexAtCellCoords(cellCoords);
        boost::optional<std::shared_ptr<TerrainMesh>> maybeTerrainMeshPtr = meshes.getIfExists(index);
        
        if (maybeTerrainMeshPtr) {
            assert(*maybeTerrainMeshPtr);
            const RenderableStaticMesh &drawThis = (**maybeTerrainMeshPtr).getMesh();
            if (drawThis.vertexCount > 0) {
                encoder->setVertexBuffer(drawThis.buffer, 0);
                encoder->drawPrimitives(Triangles, 0, drawThis.vertexCount, 1);
            }
        }
    }
    
    // Figure out which meshes in the active region are missing.
    static std::vector<std::pair<Morton3, AABB>> missingMeshes;
    missingMeshes.clear();
    for (const glm::ivec3 cellCoords : slice(meshes, activeRegion)) {
        const Morton3 index = meshes.indexAtCellCoords(cellCoords);
        const auto maybeTerrainMeshPtr = meshes.getIfExists(index);
        if (!maybeTerrainMeshPtr) {
            missingMeshes.emplace_back(std::make_pair(index, meshes.cellAtCellCoords(cellCoords)));
        }
    }
    
    // If no meshes were missing in the active region then increase the horizon
    // distance so we can draw meshes further away next time.
    // Otherwise, queue the meshes to be fetched asynchronously so we can draw
    // them later. Meshes can disappear at any time due to cache purges. So,
    // we have to do this everytime we need a mesh and can't find it.
    if (0 == missingMeshes.size()) {
        auto [distance, didChange] = _horizonDistance.increment_clamp(ACTIVE_REGION_SIZE);
        if (didChange) {
            _log->info("Increasing horizon distance to {}", distance);
        }
    } else {
        _log->trace("There are {} missing meshes in active region {}.",
                    missingMeshes.size(), activeRegion);
        _meshRebuildActor->push(missingMeshes);
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

void Terrain::readerTransaction(const AABB &region, std::function<void(const Array3D<Voxel> &data)> fn)
{
    _voxels->readerTransaction(region, fn);
}

void Terrain::writerTransaction(const std::shared_ptr<TerrainOperation> &operation)
{
    assert(operation);
    _journal->add(operation);
    _voxels->writerTransaction(operation);
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
            meshCellsToRebuild.push_back(std::make_pair(index, cell));
        } else {
            _meshes->invalidate(index);
        }
    }
    _meshRebuildActor->push(meshCellsToRebuild);
}

void Terrain::rebuildNextMesh(const AABB &cell, TerrainProgressTracker &progress)
{
    // Rebuild the mesh and then stick it in the grid.
    PROFILER(TerrainRebuildNextMesh);
    auto terrainMesh = std::make_shared<TerrainMesh>(cell, _defaultMesh, _graphicsDevice, _mesher, _voxels);
    terrainMesh->rebuild(progress);
    _meshes->set(cell.center, terrainMesh);
}

std::shared_ptr<VoxelDataSource>
Terrain::createVoxelData(unsigned voxelDataSeed,
                         const boost::filesystem::path &voxelsDirectory,
                         const boost::filesystem::path &sunlightDirectory)
{
    // First, we need a voxel data generator to create terrain from noise.
    auto generator = std::make_unique<VoxelDataGenerator>(voxelDataSeed);
    
    // Next, setup a map file on disk to record the shape of the terrain.
    const auto mapRegionBox = generator->boundingBox();
    const auto mapRegionRes = generator->countCellsInRegion(mapRegionBox) / (int)MAP_REGION_SIZE;
    auto mapRegionStore = std::make_unique<MapRegionStore>(_log, voxelsDirectory, mapRegionBox, mapRegionRes);
    
    // The voxel data object stores the shape of the terrain.
    auto voxelData = std::make_unique<VoxelData>(_log,
                                                 std::move(generator),
                                                 TERRAIN_CHUNK_SIZE,
                                                 std::move(mapRegionStore));
    
    // Setup a map file on disk to record lighting computations from sunlight.
    auto sunlightRegionStore = std::make_unique<MapRegionStore>(_log, sunlightDirectory,
                                                                mapRegionBox, mapRegionRes);
    
    // The sunlight data object stores the terrain shape plus sunlight.
    auto sunlightData = std::make_shared<SunlightData>(_log,
                                                       std::move(voxelData),
                                                       TERRAIN_CHUNK_SIZE,
                                                       std::move(sunlightRegionStore));
    
    return sunlightData;
}
