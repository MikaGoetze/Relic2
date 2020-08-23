#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 fragCoord;

layout( push_constant ) uniform PushConstants
{
    mat4 mvp;
} pushConstants;


layout(location = 0) out vec3 fragColor;

void main()
{
    gl_Position = pushConstants.mvp * vec4(inPosition, 1.0);
    fragColor = vec3(fragCoord, 1.0f);
}