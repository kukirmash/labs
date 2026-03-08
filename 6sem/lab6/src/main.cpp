#include "stdio.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <IL/il.h> // DevIL

#include "Camera/Camera.h"
#include "Shader/Shader.h"
#include "GraphicObject/GraphicObject.h"
#include "FPSCounter/FPSCounter.h"
#include "InputControl/InputControl.h"
#include "ResourceManager/ResourceManager.h"
#include "RenderManager/RenderManager.h"

#include <vector>

using namespace glm;

//--------------------------------------------------------------------------
// используемый шейдер (пока только один)
Camera camera;
Shader shader;
InputControl inputControl;
std::vector<GraphicObject> graphicObjects;
int oldTime = 0;

FPSCounter fpsCounter("lab_6");

//--------------------------------------------------------------------------
// функция вызывается при перерисовке окна
// в том числе и принудительно, по команде glutPostRedisplay
void Display()
{
	// для удобства определяем ссылку на RenderManager
	RenderManager &renderManager = RenderManager::instance();

	// начинаем вывод нового кадра
	renderManager.Start();

	// добавляем в очередь все объекты, которые необходимо вывести
	for (auto &graphicObject : graphicObjects)
	{
		renderManager.AddToRenderQueue(graphicObject);
	}

	// завершаем построение кадра
	renderManager.Finish();

	// меняем передний и задний буферы цвета
	glutSwapBuffers();

	fpsCounter.Update();
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
// Инициализация графических объектов
void InitGraphicObjects()
{
	RenderManager &renderManager = RenderManager::instance();

	// Очищаем список на всякий случай перед заполнением
	graphicObjects.clear();

	// Получаем ссылку на наш менеджер ресурсов (Singleton)
	ResourceManager &rm = ResourceManager::instance();

	// ==========================================================
	// ОБЪЕКТ 1: Дом
	// ==========================================================
	GraphicObject house;
	int meshId = rm.LoadMesh("../src/MESHES/buildings/house_2.obj");
	house.SetMeshId(meshId);

	// Загружаем текстуру для дома
	int texId = rm.LoadTexture("../src/TEXTURES/buildings/house_2_orange.png"); // Проверь точное имя файла в папке!
	house.SetTextureId(texId);

	int matId = rm.LoadMaterial("../src/MATERIALS/dull_material.json");
	house.SetMaterialId(matId);

	house.SetColor(vec4(0.2, 0.2, 0.2, 1));
	house.SetPosition(vec3(0, 0, 0));
	house.SetAngle(0.0);

	graphicObjects.push_back(house);

	// ==========================================================
	// ОБЪЕКТ 2.1: Большое дерево
	// ==========================================================
	GraphicObject tree1;
	meshId = rm.LoadMesh("../src/MESHES/natures/big_tree.obj");
	tree1.SetMeshId(meshId);

	// Загружаем текстуру для дерева
	texId = rm.LoadTexture("../src/TEXTURES/natures/nature.png");
	tree1.SetTextureId(texId);

	matId = rm.LoadMaterial("../src/MATERIALS/dull_material.json");
	tree1.SetMaterialId(matId);

	tree1.SetColor(vec4(0.2, 0.8, 0.2, 1));
	tree1.SetPosition(vec3(7.5, -0.75, 2.5));
	tree1.SetAngle(0.0);

	graphicObjects.push_back(tree1);

	// ==========================================================
	// ОБЪЕКТ 2.2: Большое дерево
	// ==========================================================

	GraphicObject tree2;
	meshId = rm.LoadMesh("../src/MESHES/natures/big_tree.obj");
	tree2.SetMeshId(meshId);

	// Загружаем текстуру для дерева
	texId = rm.LoadTexture("../src/TEXTURES/natures/nature.png");
	tree2.SetTextureId(texId);

	matId = rm.LoadMaterial("../src/MATERIALS/dull_material.json");
	tree2.SetMaterialId(matId);

	tree2.SetColor(vec4(0.2, 0.8, 0.2, 1));
	tree2.SetPosition(vec3(-7.5, -0.75, 2.5));
	tree2.SetAngle(0.0);

	graphicObjects.push_back(tree2);

	// ==========================================================
	// ОБЪЕКТ 3: Полицейская машина 1
	// ==========================================================
	GraphicObject police1;
	meshId = rm.LoadMesh("../src/MESHES/vehicles/police_car.obj");
	police1.SetMeshId(meshId);

	// Загружаем текстуру для машины
	texId = rm.LoadTexture("../src/TEXTURES/vehicles/police_car.png");
	police1.SetTextureId(texId);

	matId = rm.LoadMaterial("../src/MATERIALS/dull_material.json");
	police1.SetMaterialId(matId);

	police1.SetColor(vec4(0.2, 0.2, 1.0, 1));
	police1.SetPosition(vec3(+4.5, -2.15, +6.5));
	police1.SetAngle(-115.0);

	graphicObjects.push_back(police1);

	// ==========================================================
	// ОБЪЕКТ 4: Джип
	// ==========================================================
	GraphicObject jeep;
	meshId = rm.LoadMesh("../src/MESHES/vehicles/jeep.obj");
	jeep.SetMeshId(meshId);

	// Загружаем текстуру для джипа
	texId = rm.LoadTexture("../src/TEXTURES/vehicles/jeep_green.png");
	jeep.SetTextureId(texId);

	matId = rm.LoadMaterial("../src/MATERIALS/dull_material.json");
	jeep.SetMaterialId(matId);

	jeep.SetColor(vec4(0.95, 0.13, 0.13, 1));
	jeep.SetPosition(vec3(-1.25, -2.15, +9.0));
	jeep.SetAngle(+170.0);

	graphicObjects.push_back(jeep);

	// ==========================================================
	// ОБЪЕКТ 5: Полицейская машина 2
	// ==========================================================
	GraphicObject police2;
	// Менеджер ресурсов увидит, что машина уже загружена, и просто вернет старые индексы!
	meshId = rm.LoadMesh("../src/MESHES/vehicles/police_car.obj");
	police2.SetMeshId(meshId);

	texId = rm.LoadTexture("../src/TEXTURES/vehicles/police_car.png");
	police2.SetTextureId(texId);

	matId = rm.LoadMaterial("../src/MATERIALS/dull_material.json");
	police2.SetMaterialId(matId);

	police2.SetColor(vec4(0.23, 0.23, 1.0, 1));
	police2.SetPosition(vec3(+4.25, -2.15, +10.5));
	police2.SetAngle(+105.0);

	graphicObjects.push_back(police2);
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
	glutCreateWindow("lab_6");

	fpsCounter.Reset();

	// инициализация GLEW
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Glew error: %s\n", glewGetErrorString(err));
		return 0;
	}

	// Инициализация DevIL (ОБЯЗАТЕЛЬНО перед загрузкой текстур!)
	ilInit();

	// Инициализируем RenderManager (загрузит шейдеры)
	RenderManager::instance().Init();
	RenderManager::instance().SetCamera(&camera);
	camera.Zoom(15.0f);
	camera.MoveOXZ(0, -10.0f);

	InitGraphicObjects(); // В этой функции теперь добавляй rm.LoadTexture()

	// определение текущей версии OpenGL
	printf("OpenGL Version = %s\n\n", glGetString(GL_VERSION));

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