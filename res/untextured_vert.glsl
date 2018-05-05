#version 330

layout (location=0) in vec4 vp;
layout (location=1) in vec4 vc;

out vec4 color;

layout (std140) uniform UntexturedUniforms
{
    mat4 view, proj;
};

void main()
{
    color = vc;
    gl_Position = proj * view * vp;
}
