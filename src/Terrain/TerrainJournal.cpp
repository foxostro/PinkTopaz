//
//  TerrainJournal.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/10/18.
//
//

#include "Terrain/TerrainJournal.hpp"
#include "SDL.h" // for SDL_Log

// Must include all types of terrain operations here in order for serialization
// to work correctly.
#include "Terrain/TerrainOperationEditPoint.hpp"

#include <cereal/types/vector.hpp>
#include <cereal/archives/xml.hpp>

TerrainJournal::TerrainJournal(boost::filesystem::path fileName)
 : _fileName(fileName),
   _voxelDataSeed(0),
   _journalFormatVersion(journalFormatVersion)
{
    std::ifstream inputStream(_fileName.c_str());
    
    if (inputStream) {
        cereal::XMLInputArchive archive(inputStream);
        try {
            serialize(archive);
        } catch(const cereal::Exception &exception) {
            throw TerrainJournalSerializationException("Failed to load the journal \"%s\" due to serialization error: %s", _fileName.c_str(), exception.what());
        }
    } else {
        SDL_Log("Creating new terrain journal \"%s\" with random seed %u",
                _fileName.c_str(), _voxelDataSeed);
        save();
    }
    
    if (_journalFormatVersion != journalFormatVersion) {
        throw TerrainJournalVersionException("Incompatible journal version %u. "
                                             "Current version is %u.",
                                             _journalFormatVersion,
                                             journalFormatVersion);
    }
}

void TerrainJournal::add(Change change)
{
    std::lock_guard<std::mutex> lock(_lock);
    _changes.emplace_back(std::move(change));
    save();
}

void TerrainJournal::replay(std::function<void(Change)> processChange)
{
    std::lock_guard<std::mutex> lock(_lock);
    for (auto change : _changes) {
        processChange(change);
    }
}

void TerrainJournal::save()
{
    std::ofstream outputStream(_fileName.c_str());
    cereal::XMLOutputArchive archive(outputStream);
    try {
        serialize(archive);
    } catch(const cereal::Exception &exception) {
        throw TerrainJournalSerializationException("Failed to save the journal due to serialization error: %s", exception.what());
    }
}
