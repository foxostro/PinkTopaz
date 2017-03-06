#version 410

layout (location=0) in vec4 vp;
layout (location=1) in vec4 vc;
layout (location=2) in vec3 vt;

out vec3 texCoord;
out vec4 color;

layout (std140) uniform TerrainUniforms
{
    mat4 view, proj;
};

void main()
{
	texCoord = vt;
	color = vc / 255.0;
	gl_Position = proj * view * vp;
}
