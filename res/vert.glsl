#version 410

layout (location=0) in vec3 vp;
layout (location=1) in vec3 vt;
layout (location=2) in vec4 vc;

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
	gl_Position = proj * view * vec4(vp, 1.0);
}
