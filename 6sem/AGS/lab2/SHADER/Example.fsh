#version 330 core

uniform vec4 color1;
uniform vec4 color2;
uniform vec2 windowSize;

in vec2 position; // получено от вершинного шейдера
out vec4 fragColor;

void main ()
{
	// нормализация координат для градиента
	float aspect = windowSize.x / windowSize.y;
	vec2 normPos = position;
	normPos *= aspect;

	// градиент по диагонали с учётом пропорций
	float gradient = (position.x + 0.5 + position.y + 0.5) / 2.0;
	gradient = clamp(gradient, 0, 1);

	fragColor = mix(color1, color2, gradient);
}
