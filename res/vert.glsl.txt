#version 410

uniform mat4 view, proj;

in vec3 vp;

void main()
{
	gl_Position = proj * view * vec4(vp, 1.0);
}
