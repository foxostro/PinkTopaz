//
//  StaticMeshSerializer.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/14/17.
//
//

#ifndef StaticMeshSerializer_hpp
#define StaticMeshSerializer_hpp

#include "Renderer/StaticMesh.hpp"
#include "Exception.hpp"
#include <boost/filesystem.hpp>

// Exception thrown when an error occurs while serializing a static mesh.
class StaticMeshSerializerException : public Exception
{
public:
    template<typename... Args>
    StaticMeshSerializerException(Args&&... args)
    : Exception(std::forward<Args>(args)...)
    {}
};

// Exception for when the serialized static mesh magic number is incorrect.
class StaticMeshSerializerMagicNumberException : public StaticMeshSerializerException
{
public:
    StaticMeshSerializerMagicNumberException(uint32_t actual, uint32_t expected)
    : StaticMeshSerializerException("Unexpected magic number in "
                                    "geometry data: found {} but "
                                    "expected {}",
                                    actual, expected)
    {}
};

// Exception for when the serialized static mesh version number is incorrect.
class StaticMeshSerializerVersionNumberException : public StaticMeshSerializerException
{
public:
    StaticMeshSerializerVersionNumberException(uint32_t actual, uint32_t expected)
    : StaticMeshSerializerException("Unexpected version number in "
                                    "geometry data: found {} but "
                                    "expected {}",
                                    actual, expected)
    {}
};

// Exception for when the length of serialized static mesh data is unexpected.
class StaticMeshSerializerBadLengthException : public StaticMeshSerializerException
{
public:
    StaticMeshSerializerBadLengthException(size_t actual, size_t expected)
    : StaticMeshSerializerException("Unexpected number of bytes in "
                                    "geometry data: found {} but "
                                    "expected {}",
                                    actual, expected)
    {}
};

// Takes a static mesh and serializes it to/from a stream of bytes.
class StaticMeshSerializer
{
public:
    struct FileVertex
    {
        float position[3];
        unsigned char color[4];
        float texCoord[3];
    };
    
    struct Header
    {
        uint32_t magic;
        uint32_t version;
        uint32_t w, h, d;
        int32_t numVerts;
        uint32_t len;
        FileVertex vertices[0];
    };
    
    StaticMeshSerializer();
    ~StaticMeshSerializer() = default;
    
    StaticMesh load(const std::vector<uint8_t> &bytes);
    std::vector<uint8_t> save(const StaticMesh &mesh);
    
private:
    const uint32_t GEO_MAGIC, GEO_VERSION;
    
    void convert(const FileVertex &input, TerrainVertex &output) const;
    void convert(const TerrainVertex &input, FileVertex &output) const;
};

#endif /* StaticMeshSerializer_hpp */
