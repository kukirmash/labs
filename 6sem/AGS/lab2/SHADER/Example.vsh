#version 330 core

uniform vec2 offset;	 // смещение
uniform vec2 windowSize; // размеры окна

in vec2 vPosition;
out vec2 position;

void main ()
{
	// передаём во фрагментный шейдер исходные координаты
	position = vPosition;

	float aspect = windowSize.x / windowSize.y;

	// Сначала сдвигаем объект (vPosition.x + offset.x)
	// Затем делим на aspect, чтобы уместить широкую координатную сетку в экран [-1..1]
	// Теперь по X логические координаты идут от -aspect до +aspect
	gl_Position = vec4((vPosition.x + offset.x) / aspect, (vPosition.y + offset.y)/aspect, 0, 1);
}