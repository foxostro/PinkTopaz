#version 410
in vec2 texCoord;
in vec4 vertexColor;
out vec4 color;

uniform sampler2D tex;

void main()
{    
    vec4 texel = vec4(1.0, 1.0, 1.0, texture(tex, texCoord).r);
    color = vertexColor * texel;
}
