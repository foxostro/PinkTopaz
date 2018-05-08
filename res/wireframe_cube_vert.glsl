#version 330

layout (location=0) in vec4 vp;

out vec4 color;

layout (std140) uniform WireframeCubeUniforms
{
    mat4 proj;
};

layout (std140) uniform WireframeCubeUniformsPerInstance
{
        struct Data {
        mat4 view;
        vec4 colorParameter;
    } perInstanceUniforms[819];
};

void main()
{
    color = perInstanceUniforms[gl_InstanceID].colorParameter;
    gl_Position = proj * perInstanceUniforms[gl_InstanceID].view * vp;
}
