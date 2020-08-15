#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 fragCoord;

layout(location = 0) out vec3 fragColor;

void main()
{
    fragColor = (inPosition + 0.3f) * 2.0f;
    gl_Position = vec4(inPosition, 1.0);
}