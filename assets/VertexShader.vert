#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 LightPos0;
out vec3 VertPos0;

uniform mat4 MV;
uniform mat4 MVP;
uniform mat4 NormalMtx;
uniform mat4 view;
uniform vec3 lightPos;

void main()
{
    gl_Position = MVP * vec4(position, 1.0);
    VertPos0 = vec3(MV * vec4(position, 1.0));
    TexCoord0 = texCoord;
    Normal0 = normalize(mat3(NormalMtx) * normal);
    LightPos0 = lightPos;
}
