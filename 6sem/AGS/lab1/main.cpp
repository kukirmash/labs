#include <windows.h>
#include "stdio.h"

#include "glew.h"
#include "GL/freeglut.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

// ������������ ������������ ���� (��� ��������)
using namespace glm;
using namespace std;

// ������������ ������ (���� ������ ����)
Shader shader;

// ������� ��� ������ �������� � ������� ������� ������� (�� -0.5 �� +0.5)
void drawObject();

// ������� ���������� ��� ����������� ����
// � ��� ����� � �������������, �� ������� glutPostRedisplay
void display()
{
	// �������� ����� �����
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// �������� ���� ������� (�� ������ ������)
	glEnable(GL_DEPTH_TEST);

	// ���������� ������
	shader.activate();

	// ������� �������������
	drawObject();

	// ����� ��������� � ������� �������
	glutSwapBuffers();
}

// �������, ���������� ��� ��������� �������� ����
void reshape(int w, int h)
{
	// ���������� ����� ������� ���������, ������ ���� ������� ����
	glViewport(0, 0, w, h);
}

// ������� ���������� ����� ��������� �����������, �.�. ����������� �����
void simulation()
{
	// ������������ ����
	glutPostRedisplay();
};

// �������� �������
void main(int argc, char **argv)
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
	glutInitWindowSize(800, 600);
	// �������� ����
	glutCreateWindow("laba_01");

	// ������������� GLEW
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Glew error: %s\n", glewGetErrorString(err));
		return;
	}

	// ����������� ������� ������ OpenGL
	printf("OpenGL Version = %s\n\n", glGetString(GL_VERSION));

	// �������� �������
	shader.load("SHADER\\Example.vsh", "SHADER\\Example.fsh");

	// ������������� �������, ������� ����� ���������� ��� ����������� ����
	glutDisplayFunc(display);
	// ������������� �������, ������� ����� ���������� ��� ��������� �������� ����
	glutReshapeFunc(reshape);
	// ������������� ������� ������� ���������� ������ ���, ����� ��������� �����������
	glutIdleFunc(simulation);
	// �������� ���� ��������� ��������� ��
	glutMainLoop();
	return;
}

// ������� ��� ������ �������� � ������� ������� ������� (�� -0.5 �� +0.5)
void drawObject()
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