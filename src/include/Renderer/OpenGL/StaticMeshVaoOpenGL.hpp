//
//  StaticMeshVaoOpenGL.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/4/16.
//
//

#ifndef StaticMeshVaoOpenGL_hpp
#define StaticMeshVaoOpenGL_hpp

#include <vector>
#include <atomic>

#include "Renderer/StaticMeshVao.hpp"
#include "Renderer/StaticMesh.hpp"
#include "Renderer/OpenGL/opengl.hpp"
#include "Renderer/OpenGL/CommandQueue.hpp"

namespace PinkTopaz::Renderer::OpenGL {
    
    class StaticMeshVaoOpenGL : public StaticMeshVao
    {
    public:
        StaticMeshVaoOpenGL(CommandQueue &queue, const std::shared_ptr<StaticMesh> &mesh);
        virtual ~StaticMeshVaoOpenGL();
        virtual size_t getNumVerts() const override;
        
        inline GLuint getHandle() const { return _vao; }

    private:
        std::atomic<GLuint> _vao;
        std::atomic<GLsizei> _numVerts;
        CommandQueue &_commandQueue;
    };

} // namespace PinkTopaz::Renderer::OpenGL

#endif /* StaticMeshVaoOpenGL_hpp */
