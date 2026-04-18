#include "stdio.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader/Shader.h"
#include "FPSCounter/FPSCounter.h"

using namespace glm;

//*--------------------------------------------------------------------------
// ������������ ������ (���� ������ ����)
Shader shader;

int oldTime = 0;
vec2 windowSize = vec2(800.0f, 600.0f);
vec2 offset = vec2(0, 0);
vec2 speed = vec2(+0.30, -0.25);
vec4 color1 = vec4(0, 0, 1, 1);
vec4 color2 = vec4(1, 0, 0, 1);

FPSCounter fpsCounter("laba_02");

//*--------------------------------------------------------------------------
// ������� ��� ������ �������� � ������� ������� ������� (�� -0.5 �� +0.5)
void DrawObject()
{
	// ���������� ��� ������ ������� (�������������� �� ���� �������������)
	static bool init = true;
	static GLuint VAO_Index = 0; // ������ VAO-������
	static GLuint VBO_Index = 0; // ������ VBO-������
	static int VertexCount = 0;	 // ���������� ������

	// ��� ������ ������ �������������� VBO � VAO
	if (init)
	{
		init = false;
		// �������� � ���������� VBO
		glGenBuffers(1, &VBO_Index);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_Index);

		// �������
		GLfloat Verteces[] = {
			-0.5, +0.5,
			-0.5, -0.5,
			+0.5, +0.5,

			+0.5, +0.5,
			-0.5, -0.5,
			+0.5, -0.5};

		glBufferData(GL_ARRAY_BUFFER, sizeof(Verteces), Verteces, GL_STATIC_DRAW);

		// �������� VAO
		glGenVertexArrays(1, &VAO_Index);
		glBindVertexArray(VAO_Index);
		// ���������� VAO
		glBindBuffer(GL_ARRAY_BUFFER, VBO_Index);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
		// "�������" ������ VAO, ���� �������� �� ���������
		glBindVertexArray(0);
		// �������� ���������� ������
		VertexCount = 6;
	}
	// ������� �������������
	glBindVertexArray(VAO_Index);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

//*--------------------------------------------------------------------------
// ������� ���������� ��� ����������� ����
// � ��� ����� � �������������, �� ������� glutPostRedisplay
void Display()
{
	// ������� ����� (�������� ����� �����)
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST); // �������� ���� ������� (�� ������ ������)

	// ���������� ������
	shader.Activate();
	shader.SetUniform("offset", offset);
	shader.SetUniform("color1", color1);
	shader.SetUniform("color2", color2);
	shader.SetUniform("windowSize", windowSize);

	// ������� �������������
	DrawObject();

	// ��������� ������� FPS
	fpsCounter.Update();

	// ����� ��������� � ������� �������
	glutSwapBuffers();
}

//*--------------------------------------------------------------------------
// �������, ���������� ��� ��������� �������� ����
void Reshape(int w, int h)
{
	// ���������� ����� ������� ���������, ������ ���� ������� ����
	glViewport(0, 0, w, h);

	windowSize = vec2((float)w, (float)h);
}

//*--------------------------------------------------------------------------
float GetSimulationTime()
{
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	int deltaTime = currentTime - oldTime;
	oldTime = currentTime;

	return deltaTime / 1000.0f;
}

//*--------------------------------------------------------------------------
// ������� ����������, ����� ��������� �����������, �.�. ����������� �����
void Simulation()
{
	float simTime = GetSimulationTime();
	offset += speed * simTime; // offset - ����� ��������

	float aspect = windowSize.x / windowSize.y;

	float limitY = aspect - 0.5f;

	if (offset.y > limitY)
	{
		offset.y = limitY;
		speed.y = -speed.y;
	}
	if (offset.y < -limitY)
	{
		offset.y = -limitY;
		speed.y = -speed.y;
	}

	float limitX = aspect - 0.5f;

	if (offset.x > limitX)
	{
		offset.x = limitX;
		speed.x = -speed.x;
	}
	if (offset.x < -limitX)
	{
		offset.x = -limitX;
		speed.x = -speed.x;
	}

	glutPostRedisplay();
}

//*--------------------------------------------------------------------------
// �������� �������
int main(int argc, char **argv)
{
	// ������������� ���������� GLUT
	glutInit(&argc, argv);
	// ������������� ������� (������ ������)
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
	// ���������� � ������ OpenGL (������ 3.3 ��� ��������� �������� �������������)
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	// ������������� ������� ����� ���� ����
	glutInitWindowPosition(300, 100);
	// ������������� ������ ����
	glutInitWindowSize(800, 800);
	// �������� ����
	glutCreateWindow("laba_02");

	fpsCounter.Reset();

	// ������������� GLEW
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Glew error: %s\n", glewGetErrorString(err));
		return 0;
	}

	// ����������� ������� ������ OpenGL
	printf("OpenGL Version = %s\n\n", glGetString(GL_VERSION));
	// �������� �������
	shader.Load("../SHADER/Example.vsh", "../SHADER/Example.fsh");

	// ������������� �������, ������� ����� ���������� ��� ����������� ����
	glutDisplayFunc(Display);
	// ������������� �������, ������� ����� ���������� ��� ��������� �������� ����
	glutReshapeFunc(Reshape);
	// ������������� �������, ������� ���������� ������ ���, ����� ��������� �����������
	glutIdleFunc(Simulation);
	// �������� ���� ��������� ��������� ��
	glutMainLoop();
}

//*--------------------------------------------------------------------------