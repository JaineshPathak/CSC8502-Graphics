#version 410 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in Vertex
{
    vec4 colour;
} IN[];

/*
out GS_OUT {
    
} gs_out;
*/

void main()
{
    gl_Position = gl_in[0].gl_Position;

    EmitVertex();
    EndPrimitive();
}