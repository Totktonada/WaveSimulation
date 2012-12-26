#version 330 core

in vec3 position;
out vec2 vpos;

void main(void)
{
    vpos = position.xy;
    gl_Position = vec4(position.xy, 1.0, 1.0);
}
