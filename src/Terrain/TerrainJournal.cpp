//
//  TerrainJournal.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/10/18.
//
//

#include "Terrain/TerrainJournal.hpp"

// Must include all types of terrain operations here in order for serialization
// to work correctly.
#include "Terrain/TerrainOperationEditPoint.hpp"

#include <cereal/types/vector.hpp>
#include <cereal/archives/xml.hpp>

TerrainJournal::TerrainJournal(std::shared_ptr<spdlog::logger> log,
                               boost::filesystem::path fileName)
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
            throw TerrainJournalSerializationException("Failed to load the journal \"{}\" due to serialization error: {}", _fileName.string(), exception.what());
        }
    } else {
        log->info("Creating new terrain journal \"{}\" with random seed {}",
                  _fileName.string(), _voxelDataSeed);
        save();
    }
    
    if (_journalFormatVersion != journalFormatVersion) {
        throw TerrainJournalVersionException("Incompatible journal version {}. "
                                             "Current version is {}.",
                                             _journalFormatVersion,
                                             journalFormatVersion);
    }
}

void TerrainJournal::add(Change change)
{
    std::scoped_lock lock(_lock);
    _changes.emplace_back(std::move(change));
    save();
}

void TerrainJournal::replay(std::function<void(Change)> processChange)
{
    std::scoped_lock lock(_lock);
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
        throw TerrainJournalSerializationException("Failed to save the journal due to serialization error: {}", exception.what());
    }
}
