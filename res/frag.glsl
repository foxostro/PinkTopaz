#version 410

in vec3 texCoord;
in vec4 color;
out vec4 fragColor;

const vec4 fogColor = vec4(0.7, 0.7, 0.7, 1.0);
const float fogDensity = 0.003;

vec4 fog(vec4 color, vec4 fcolor, float depth, float density)
{
	const float e = 2.71828182845904523536028747135266249;
	float f = pow(e, -pow(depth*density, 2.0));
	return mix(fcolor, color, f);
}

void main(void)
{
    //fragColor = fog(color, fogColor, gl_FragCoord.z / gl_FragCoord.w, fogDensity);
    fragColor = vec4(texCoord.xyz, 1);
}
