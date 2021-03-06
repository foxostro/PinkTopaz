#version 330

uniform sampler2DArray tex;

in vec3 texCoord;
in vec4 color;
in float vertexFogDensity;
out vec4 fragColor;

const vec4 fogColor = vec4(0.2, 0.4, 0.5, 1.0);

vec4 fog(vec4 color, vec4 fcolor, float depth, float density)
{
    const float e = 2.71828182845904523536028747135266249;
    float f = pow(e, -pow(depth*density, 2.0));
    return mix(fcolor, color, f);
}

void main(void)
{
    vec4 texcolor = texture(tex, texCoord.stp);
    fragColor = fog(texcolor * color, fogColor, gl_FragCoord.z / gl_FragCoord.w, vertexFogDensity);
}
