//
//  Preferences.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/12/18.
//
//

#ifndef Preferences_hpp
#define Preferences_hpp

#include <spdlog/fmt/ostr.h>
#include <cereal/archives/xml.hpp>
#include <spdlog/spdlog.h>


// Allow serializing spdlog::level::level_enum with cereal.
namespace cereal {
    template <class Archive> inline
    std::string save_minimal(const Archive&,
                             const spdlog::level::level_enum &level)
    {
        return spdlog::level::to_str(level);
    }
    
    template <class Archive> inline
    void load_minimal(const Archive &,
                      spdlog::level::level_enum &level,
                      const std::string &stringValue)
    {
        for (size_t i =0, n = sizeof(spdlog::level::level_names) / sizeof(spdlog::level::level_names[0]); i < n; ++i) {
            const std::string levelName = spdlog::level::level_names[i];
            if (stringValue == levelName) {
                level = (spdlog::level::level_enum)i;
                break;
            }
        }
    }
} // namespace cereal


// User preferences
struct Preferences
{
    bool showQueuedChunks;
    bool smoothTerrain;
    spdlog::level::level_enum logLevel;
    float activeRegionSize;
    
    Preferences()
     : showQueuedChunks(false),
       smoothTerrain(true),
       logLevel(spdlog::level::info),
       activeRegionSize(256.f)
    {}
    
    // Permits logging with spdlog.
    template<typename OStream>
    friend OStream& operator<<(OStream &os, const Preferences &prefs)
    {
        return os << "Preferences {"
                  << "\n\tshowQueuedChunks: "
                  << (prefs.showQueuedChunks ? "true" : "false")
                  << "\n\tsmoothTerrain: "
                  << (prefs.smoothTerrain ? "true" : "false")
                  << "\n\tlogLevel: "
                  << spdlog::level::to_str(prefs.logLevel)
                  << "\n\tactiveRegionSize: "
                  << prefs.activeRegionSize
                  << "\n}";
    }
    
    // Permits serialization with cereal.
    template<typename Archive>
    void serialize(Archive &archive)
    {
        archive(CEREAL_NVP(showQueuedChunks),
                CEREAL_NVP(smoothTerrain),
                CEREAL_NVP(logLevel),
                CEREAL_NVP(activeRegionSize));
    }
};

#endif /* Preferences_hpp */
