#version 430 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

uniform mat4 projMatrix;

const float MAG = 10.5;

//This should be same as from Vertex Shader
in Vertex
{
    vec3 normal;
} IN[];

void GenerateLine(int index)
{
    gl_Position = projMatrix * gl_in[index].gl_Position;
    EmitVertex();
    
    gl_Position = projMatrix * (gl_in[index].gl_Position + vec4(IN[index].normal, 0.0) * MAG);
    EmitVertex();
    
    EndPrimitive();
}

void main()
{
    GenerateLine(0); // first vertex normal
    GenerateLine(1); // second vertex normal
    GenerateLine(2); // third vertex normal
}