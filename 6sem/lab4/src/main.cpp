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
#include "ResourceManager/ResourceManager.h"

#include <vector>

using namespace glm;

//--------------------------------------------------------------------------
// используемый шейдер (пока только один)
Camera camera;
Shader shader;
InputControl inputControl;
std::vector<GraphicObject> graphicObjects;

int oldTime = 0;
vec2 windowSize = vec2(800.0f, 600.0f);
vec2 offset = vec2(0, 0);
vec2 speed = vec2(+0.30, -0.25);
vec4 color1 = vec4(0, 0, 1, 1);
vec4 color2 = vec4(1, 0, 0, 1);

FPSCounter fpsCounter("lab_4");

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

	// активируем шейдер, используемый для вывода объекта
	shader.Activate();

	// устанавливаем матрицу проекции
	mat4 &projectionMatrix = camera.GetProjectionMatrix();
	shader.SetUniform("projectionMatrix", projectionMatrix);

	// получяем матрицу камеры
	mat4 &viewMatrix = camera.GetViewMatrix();

	// выводим все объекты
	for (auto &graphicObject : graphicObjects)
	{

		// устанавливаем матрицу наблюдения модели
		mat4 modelViewMatrix = viewMatrix * graphicObject.GetModelMatrix();
		shader.SetUniform("modelViewMatrix", modelViewMatrix);

		// устанавливаем цвет
		shader.SetUniform("color", graphicObject.GetColor());

		// выводим меш
		int meshId = graphicObject.GetMeshId();
		Mesh *mesh = ResourceManager::instance().GetMesh(meshId);
		if (mesh != nullptr)
			mesh->DrawOne();
	}

	fpsCounter.Update();

	// меняем передний и задний буферы цвета
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
void InitGraphicObjects()
{
	// ссылка на менеджер ресурсов (для удобства)
	ResourceManager &rm = ResourceManager::instance();
	// временная переменная для хранения идентификаторов меша
	int meshId = -1;
	// временная переменная для представления графического объекта
	GraphicObject graphicObject;

	// добавление графического объекта
	meshId = rm.LoadMesh("../src/MESHES/buildings/house_2.obj");
	graphicObject.SetMeshId(meshId);
	graphicObject.SetColor(vec4(0.2, 0.2, 0.2, 1));
	graphicObject.SetPosition(vec3(0, 0, 0));
	graphicObject.SetAngle(0.0);
	graphicObjects.push_back(graphicObject);

	// добавление графического объекта
	meshId = rm.LoadMesh("../src/MESHES/natures/big_tree.obj");
	graphicObject.SetMeshId(meshId);
	graphicObject.SetColor(vec4(0.2, 0.8, 0.2, 1));
	graphicObject.SetPosition(vec3(7.5, -0.75, 2.5));
	graphicObject.SetAngle(0.0);
	graphicObjects.push_back(graphicObject);

	// добавление графического объекта
	meshId = rm.LoadMesh("../src/MESHES/natures/big_tree.obj");
	graphicObject.SetMeshId(meshId);
	graphicObject.SetColor(vec4(0.2, 0.8, 0.2, 1));
	graphicObject.SetPosition(vec3(-7.5, -0.75, 2.5));
	graphicObject.SetAngle(0.0);
	graphicObjects.push_back(graphicObject);

	// добавление графического объекта
	meshId = rm.LoadMesh("../src/MESHES/vehicles/police_car.obj");
	graphicObject.SetMeshId(meshId);
	graphicObject.SetColor(vec4(0.2, 0.2, 1.0, 1));
	graphicObject.SetPosition(vec3(+4.5, -2.15, +6.5));
	graphicObject.SetAngle(-115.0);
	graphicObjects.push_back(graphicObject);

	// добавление графического объекта
	meshId = rm.LoadMesh("../src/MESHES/vehicles/police_car.obj");
	graphicObject.SetMeshId(meshId);
	graphicObject.SetColor(vec4(0.23, 0.23, 1.0, 1));
	graphicObject.SetPosition(vec3(+4.25, -2.15, +10.5));
	graphicObject.SetAngle(+105.0);
	graphicObjects.push_back(graphicObject);

	// добавление графического объекта
	meshId = rm.LoadMesh("../src/MESHES/vehicles/jeep.obj");
	graphicObject.SetMeshId(meshId);
	graphicObject.SetColor(vec4(0.95, 0.13, 0.13, 1));
	graphicObject.SetPosition(vec3(-1.25, -2.15, +9.0));
	graphicObject.SetAngle(+170.0);
	graphicObjects.push_back(graphicObject);
}

//--------------------------------------------------------------------------
// основная функция
int main(int argc, char **argv)
{
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
	glutCreateWindow("lab_4");

	fpsCounter.Reset();

	// инициализация GLEW
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Glew error: %s\n", glewGetErrorString(err));
		return 0;
	}

	InitGraphicObjects();

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