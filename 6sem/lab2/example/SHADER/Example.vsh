#version 330 core

uniform vec2 offset;

in vec2 vPosition;

out vec2 position;

void main ()
{
	position = vPosition; 
	
	gl_Position = vec4 (vPosition + offset, 0, 1);
}

