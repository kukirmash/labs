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
#include "Scene/Scene.h"

#include <vector>

using namespace glm;

//--------------------------------------------------------------------------
// используемый шейдер (пока только один)
Camera camera;
Shader shader;
InputControl inputControl;
std::vector<GraphicObject> graphicObjects;
int oldTime = 0;
Scene scene;
Light mainLight;
FPSCounter fpsCounter("lab_8");

//--------------------------------------------------------------------------
// функция вызывается при перерисовке окна
// в том числе и принудительно, по команде glutPostRedisplay
void Display()
{
	RenderManager &renderManager = RenderManager::instance();
	renderManager.Start();

	scene.Draw(); // Сцена сама отсеет невидимые объекты

	renderManager.Finish();
	glutSwapBuffers();

	// Обновленный заголовок со статистикой [cite: 269-274]
	char windowTitle[256];
	sprintf(windowTitle, "Laba_08 [%d FPS][%s][materials: %d, textures: %d, draw calls: %d]",
			(int)fpsCounter.GetFps(),
			scene.GetSceneDescription().c_str(),
			renderManager.GetMaterialChangeCount(),
			renderManager.GetTextureChangeCount(),
			renderManager.GetDrawCallCount());
	glutSetWindowTitle(windowTitle);
	glutSetWindowTitle(windowTitle);

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

	// Сообщаем RenderManager'у, что нужно пересоздать "холсты" под новый размер
	RenderManager::instance().ResizeFBOs(w, h);
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

	float speed = 20.0f; // Скорость передвижения
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
	glutCreateWindow("lab_7");

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

	scene.Init("../src/DATA/models.json");
	scene.LoadFromJson("../src/DATA/scenes/big_scene.json");

	scene.SetCamera(&camera);

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