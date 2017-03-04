#version 410
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 texCoord;
out vec4 vertexColor;

layout (std140) uniform StringUniforms
{
    vec4 textColor;
    mat4 projection;
};

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    texCoord = vertex.zw;
    vertexColor = textColor;
}
