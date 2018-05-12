
//
//  TextureArrayLoader.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/1/18.
//
//

#ifndef TextureArrayLoader_hpp
#define TextureArrayLoader_hpp

#include <boost/filesystem.hpp>
#include "Renderer/Texture.hpp"
#include "Renderer/GraphicsDevice.hpp"
#include "Renderer/RendererException.hpp"

// Exception thrown when the texture fails to load from file.
class TextureArrayLoaderFailedException : public RendererException
{
public:
    template<typename... Args>
    TextureArrayLoaderFailedException(Args&&... args)
    : RendererException(std::forward<Args>(args)...)
    {}
};

// Load array textures from file.
class TextureArrayLoader
{
public:
    TextureArrayLoader(const std::shared_ptr<GraphicsDevice> &graphicsDevice);
    
    // Creates an array texture from the image with the specified filename.
    // Each element of the array texture is assumed to have a width and height
    // equal to the width of the image. The number of elements is assumed to be
    // the total height of the image divided by this width.
    std::shared_ptr<Texture> load(const boost::filesystem::path &imageFileName);
    
private:
    std::shared_ptr<GraphicsDevice> _graphicsDevice;
};

#endif /* TextureArrayLoader_hpp */
