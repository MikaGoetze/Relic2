#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 color;

layout(location = 0) in vec2 fragCoord;

layout(set = 1, binding = 1) uniform sampler2D texSampler;

void main()
{
    color = texture(texSampler, fragCoord);
//    color = vec4(fragCoord, 0.0, 1.0);
}