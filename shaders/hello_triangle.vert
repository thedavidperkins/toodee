#version 460

const vec2 verts[] = 
{
	{ -.5, .5 }, { .5, .5 }, { 0, -.5 },
};
const vec3 colors[] = 
{
	{ 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 0.f, 0.f, 1.f },
};
layout( location = 0 ) out vec3 colorOut;


void main()
{
	colorOut = colors[gl_VertexIndex];
	gl_Position = vec4( verts[gl_VertexIndex], 0.f, 1.f );
}
