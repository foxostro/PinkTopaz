#version 330

in vec2 texCoord;
in vec4 vertexColor;
out vec4 color;

uniform sampler2D tex;

void main()
{
    color = texture(tex, texCoord) * vertexColor;
}
