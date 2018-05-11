//
//  TerrainJournal.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/10/18.
//
//

#ifndef TerrainJournal_hpp
#define TerrainJournal_hpp

#include "Terrain/TerrainOperation.hpp"
#include "Exception.hpp"
#include <boost/filesystem.hpp>
#include <mutex>

// Exception thrown when trying to load an incompatible journal version.
class TerrainJournalVersionException : public Exception
{
public:
    template<typename... Args>
    TerrainJournalVersionException(Args&&... args)
     : Exception(std::forward<Args>(args)...)
    {}
};

// Exception thrown when an error occurs while serializing the journal.
class TerrainJournalSerializationException : public Exception
{
public:
    template<typename... Args>
    TerrainJournalSerializationException(Args&&... args)
     : Exception(std::forward<Args>(args)...)
    {}
};

// A record of edits made to the terrain.
class TerrainJournal
{
public:
    using Change = std::shared_ptr<TerrainOperation>;
    
    // A version number for the journal file format on disk.
    static constexpr unsigned journalFormatVersion = 1;
    
    // Default destructor.
    ~TerrainJournal() = default;
    
    // No default constructor.
    TerrainJournal() = delete;
    
    // Constructor.
    // fileName -- The path to the journal file.
    TerrainJournal(boost::filesystem::path fileName);
    
    // Add an edit to the journal.
    void add(Change change);
    
    // Replays changes made to the journal.
    // For each entry, call `processChange' with the operation.
    void replay(std::function<void(Change)> processChange);
    
    // Returns a pseudorandom seed for procedurally generated voxels.
    inline unsigned getVoxelDataSeed() const
    {
        return _voxelDataSeed;
    }
    
    // Sets a pseudorandom seed for procedurally generated voxels.
    inline void setVoxelDataSeed(unsigned seed)
    {
        _voxelDataSeed = seed;
    }
    
    template<typename Archive>
    void serialize(Archive &archive)
    {
        archive(cereal::make_nvp("journalFormatVersion", _journalFormatVersion),
                cereal::make_nvp("voxelDataSeed", _voxelDataSeed),
                cereal::make_nvp("changes", _changes));
    }
    
private:
    std::mutex _lock;
    boost::filesystem::path _fileName;
    std::vector<Change> _changes;
    unsigned _voxelDataSeed;
    unsigned _journalFormatVersion;
    
    void save();
};

#endif /* TerrainJournal_hpp */
