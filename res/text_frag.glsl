#version 410
in vec2 texCoord;
out vec4 color;

uniform sampler2D tex;

layout(std140) uniform TextUniforms
{
	vec4 textColor;
};

void main()
{    
    vec4 texel = vec4(1.0, 1.0, 1.0, texture(tex, texCoord).r);
    color = textColor * texel;
}
