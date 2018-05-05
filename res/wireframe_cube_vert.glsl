#version 330

layout (location=0) in vec4 vp;

out vec4 color;

layout (std140) uniform WireframeCubeUniforms
{
    mat4 view, proj;
    vec4 colorParameter;
};

void main()
{
    color = colorParameter;
    gl_Position = proj * view * vp;
}
