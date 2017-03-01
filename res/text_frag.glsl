#version 410
in vec2 texCoord;
out vec4 color;

uniform sampler2D tex;
uniform vec3 textColor;

void main()
{    
    vec4 texel = vec4(1.0, 1.0, 1.0, texture(tex, texCoord).r);
    color = vec4(textColor, 1.0) * texel;
}
