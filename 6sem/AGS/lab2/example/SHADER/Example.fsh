#version 330 core

uniform vec4 color1;
uniform vec4 color2;

in vec2 position;

out vec4 fragColor;

void main ()
{
	float gradient = position.x + 0.5;
	
	fragColor = color1 * gradient + color2 * (1.0 - gradient);	
}
