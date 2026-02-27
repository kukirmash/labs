#include "stdio.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera/Camera.h"
#include "Shader/Shader.h"
#include "GraphicObject/GraphicObject.h"
#include "FPSCounter/FPSCounter.h"
#include "InputControl/InputControl.h"

#include <vector>

using namespace glm;

//--------------------------------------------------------------------------
// используемый шейдер (пока только один)
Camera camera;
Shader shader;
InputControl inputControl();
std::vector<GraphicObject> graphicObjects;

int oldTime = 0;
vec2 windowSize = vec2(800.0f, 600.0f);
vec2 offset = vec2(0, 0);
vec2 speed = vec2(+0.30, -0.25);
vec4 color1 = vec4(0, 0, 1, 1);
vec4 color2 = vec4(1, 0, 0, 1);

FPSCounter fpsCounter("lab_3");

//--------------------------------------------------------------------------
// функция вывода кубика с ребрами единичной длины
// каждая координата (x, y, z) меняется от -0.5 до +0.5
void DrawBox()
{
	// переменные для вывода объекта (прямоугольника из двух треугольников)
	static GLuint VAO_Index = 0; // индекс VAO-буфера
	static GLuint VBO_Index = 0; // индекс VBO-буфера
	static int VertexCount = 0;	 // количество вершин
	static bool init = true;

	if (init)
	{
		// создание и заполнение VBO
		glGenBuffers(1, &VBO_Index);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_Index);
		GLfloat Verteces[] = {
			// передняя грань (два треугольника)
			-0.5, +0.5, +0.5, -0.5, -0.5, +0.5, +0.5, +0.5, +0.5,
			+0.5, +0.5, +0.5, -0.5, -0.5, +0.5, +0.5, -0.5, +0.5,
			// задняя грань (два треугольника)
			+0.5, +0.5, -0.5, +0.5, -0.5, -0.5, -0.5, +0.5, -0.5,
			-0.5, +0.5, -0.5, +0.5, -0.5, -0.5, -0.5, -0.5, -0.5,
			// правая грань (два треугольника)
			+0.5, -0.5, +0.5, +0.5, -0.5, -0.5, +0.5, +0.5, +0.5,
			+0.5, +0.5, +0.5, +0.5, -0.5, -0.5, +0.5, +0.5, -0.5,
			// левая грань (два треугольника)
			-0.5, +0.5, +0.5, -0.5, +0.5, -0.5, -0.5, -0.5, +0.5,
			-0.5, -0.5, +0.5, -0.5, +0.5, -0.5, -0.5, -0.5, -0.5,
			// верхняя грань (два треугольника)
			-0.5, +0.5, -0.5, -0.5, +0.5, +0.5, +0.5, +0.5, -0.5,
			+0.5, +0.5, -0.5, -0.5, +0.5, +0.5, +0.5, +0.5, +0.5,
			// нижняя грань (два треугольника)
			-0.5, -0.5, +0.5, -0.5, -0.5, -0.5, +0.5, -0.5, +0.5,
			+0.5, -0.5, +0.5, -0.5, -0.5, -0.5, +0.5, -0.5, -0.5};
		glBufferData(GL_ARRAY_BUFFER, sizeof(Verteces), Verteces, GL_STATIC_DRAW);

		// создание VAO
		glGenVertexArrays(1, &VAO_Index);
		glBindVertexArray(VAO_Index);

		// инициализация VAO
		glBindBuffer(GL_ARRAY_BUFFER, VBO_Index);
		int location = 0;
		glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(location);

		// "отвязка" буфера VAO на всякий случай, чтоб случайно не испортить
		glBindVertexArray(0);

		// указание количество вершин
		VertexCount = 6 * 6;
		init = false;
	}

	// вывод модели кубика на экран
	glBindVertexArray(VAO_Index);
	glDrawArrays(GL_TRIANGLES, 0, VertexCount);
}

//--------------------------------------------------------------------------
// функция вызывается при перерисовке окна
// в том числе и принудительно, по команде glutPostRedisplay
void Display()
{
	// очистка буфера кадра
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// включение теста глубины (на всякий случай)
	glEnable(GL_DEPTH_TEST);

	// вывод полигонов в виде линий с отсечением нелицевых граней
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Активируем шейдер
	shader.Activate();

	// Забираем матрицу проекции и вида напрямую из нашей камеры
	mat4 projectionMatrix = camera.GetProjectionMatrix();
	mat4 viewMatrix = camera.GetViewMatrix();

	// Передаем матрицу проекции (она одинакова для всех кубиков)
	shader.SetUniform("projectionMatrix", projectionMatrix);

	// Выводим все графические объекты из вектора [cite: 442-444]
	for (auto &grobj : graphicObjects)
	{
		// Вычисляем матрицу наблюдения-модели: Camera * Model
		// ВАЖНО: Порядок перемножения имеет значение!
		glm::mat4 modelViewMatrix = viewMatrix * grobj.GetModelMatrix();

		// Передаем итоговую матрицу в шейдер
		shader.SetUniform("modelViewMatrix", modelViewMatrix);

		// Устанавливаем цвет текущего кубика
		shader.SetUniform("color", grobj.GetColor());

		// Выводим модель кубика на экран
		DrawBox();
	}

	// Обновляем счетчик FPS
	fpsCounter.Update();

	// смена переднего и заднего буферов
	glutSwapBuffers();
}

//--------------------------------------------------------------------------
// функция, вызываемая при изменении размеров окна
void Reshape(int w, int h)
{
	// установить новую область просмотра, равную всей области окна
	glViewport(0, 0, w, h);
	// устанавливаем матрицу проекции
	camera.SetProjectionMatrix(35.0f, (float)w / h, 1.0f, 500.0f);
}

//--------------------------------------------------------------------------
float GetSimulationTime()
{
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	int deltaTime = currentTime - oldTime;
	oldTime = currentTime;

	return deltaTime / 1000.0f;
}

//--------------------------------------------------------------------------
// функция вызывается, когда процессор простаивает, т.е. максимально часто
void Simulation()
{
	float simTime = GetSimulationTime(); // Ваша функция расчета времени из лабы 2

	float speed = 5.0f; // Скорость передвижения
	float distance = speed * simTime;

	float dx = 0.0f;
	float dz = 0.0f;

	// Проверяем зажатые стрелочки
	if (InputControl::specialKeys[GLUT_KEY_UP])
		dz -= distance; // Вперед
	if (InputControl::specialKeys[GLUT_KEY_DOWN])
		dz += distance; // Назад
	if (InputControl::specialKeys[GLUT_KEY_LEFT])
		dx -= distance; // Влево
	if (InputControl::specialKeys[GLUT_KEY_RIGHT])
		dx += distance; // Вправо

	// Перемещаем камеру
	if (dx != 0.0f || dz != 0.0f)
		camera.MoveOXZ(dx, dz);

	glutPostRedisplay(); // Перерисовываем экран
}

//--------------------------------------------------------------------------
void InitScene()
{
	// Очищаем массив объектов на случай перезапуска
	graphicObjects.clear();

	// Задаем "пиксельные" координаты для буквы W
	// Стоять она будет в плоскости X и Y (Z у всех равен 0)
	glm::vec3 positions[] = {
		glm::vec3(-2.0f, 2.0f, 0.0f),  // 1. Левый верхний
		glm::vec3(-2.0f, 1.0f, 0.0f),  // 2. Левый центр-верх
		glm::vec3(-2.0f, 0.0f, 0.0f),  // 3. Левый центр-низ
		glm::vec3(-1.0f, -1.0f, 0.0f), // 4. Левый нижний угол
		glm::vec3(0.0f, 0.0f, 0.0f),   // 5. Центр низ
		glm::vec3(0.0f, 1.0f, 0.0f),   // 6. Центр верх
		glm::vec3(1.0f, -1.0f, 0.0f),  // 7. Правый нижний угол
		glm::vec3(2.0f, 0.0f, 0.0f),   // 8. Правый центр-низ
		glm::vec3(2.0f, 1.0f, 0.0f),   // 9. Правый центр-верх
		glm::vec3(2.0f, 2.0f, 0.0f)	   // 10. Правый верхний
	};

	// Задаем 10 разных цветов (RGBA)
	glm::vec4 colors[] = {
		glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), // Красный
		glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), // Зеленый
		glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), // Синий
		glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), // Желтый
		glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), // Голубой (Cyan)
		glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), // Пурпурный (Magenta)
		glm::vec4(1.0f, 0.5f, 0.0f, 1.0f), // Оранжевый
		glm::vec4(0.5f, 0.0f, 1.0f, 1.0f), // Фиолетовый
		glm::vec4(1.0f, 0.5f, 0.5f, 1.0f), // Розовый
		glm::vec4(0.5f, 1.0f, 0.0f, 1.0f)  // Салатовый
	};

	// Коэффициент расстояния (чтобы кубики не слипались гранями)
	// Так как размер кубика 1х1, расстояние 1.2f оставит красивый зазор
	float spacing = 1.2f;

	// В цикле создаем 10 объектов
	for (int i = 0; i < 10; i++)
	{
		GraphicObject obj;

		// Умножаем базовые координаты на spacing
		glm::vec3 pos = positions[i] * spacing;

		obj.SetPosition(pos);
		obj.SetColor(colors[i]);

		// Добавляем готовый кубик в наш вектор
		graphicObjects.push_back(obj);
	}
}

//--------------------------------------------------------------------------
// основная функция
int main(int argc, char **argv)
{
	InitScene(); // Создаем кубики

	InputControl::camera = &camera;

	// инициализация библиотеки GLUT
	glutInit(&argc, argv);
	// инициализация дисплея (формат вывода)
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
	// требования к версии OpenGL (версия 3.3 без поддержки обратной совместимости)
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	// устанавливаем верхний левый угол окна
	glutInitWindowPosition(300, 100);
	// устанавливаем размер окна
	glutInitWindowSize(800, 800);
	// создание окна
	glutCreateWindow("lab_3");

	fpsCounter.Reset();

	// инициализация GLEW
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Glew error: %s\n", glewGetErrorString(err));
		return 0;
	}

	// определение текущей версии OpenGL
	printf("OpenGL Version = %s\n\n", glGetString(GL_VERSION));
	// загрузка шейдера
	shader.Load("../src/SHADER/Example.vsh", "../src/SHADER/Example.fsh");

	// устанавливаем функцию, которая будет вызываться для перерисовки окна
	glutDisplayFunc(Display);
	// устанавливаем функцию, которая будет вызываться при изменении размеров окна
	glutReshapeFunc(Reshape);
	// устанавливаем функцию, которая вызывается всякий раз, когда процессор простаивает
	glutIdleFunc(Simulation);

	// Новые регистрации для ввода (вместо WinAPI)
	glutSpecialFunc(InputControl::SpecialKeyDown);
	glutSpecialUpFunc(InputControl::SpecialKeyUp);
	glutMouseFunc(InputControl::MouseClick);
	glutMotionFunc(InputControl::MouseMotion);
	glutMouseWheelFunc(InputControl::MouseWheel);

	// основной цикл обработки сообщений ОС
	glutMainLoop();
}

//--------------------------------------------------------------------------