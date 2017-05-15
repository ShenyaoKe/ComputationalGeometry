#include "GL/glew.h"	/* include GLEW and new version of GL on Windows */
#include "GL/glfw3.h" /* GLFW helper library */

#include "ConvexHull.h"
#include "Utils.h"

static vector<uint32_t> ptIndices;

static std::unique_ptr<ConvexHull> convexHullPtr;
static int winSize = 800;
static GLuint vao;
static GLuint vbo, ibo;

static GLuint vertShader, hullFragShader, vertFragShader;
static GLuint hullShaderProgram, vertShaderProgram;
static GLFWwindow* window = nullptr;

static size_t pointCount = 3;

void window_refresh_callback(GLFWwindow* window);

void resetPoints();

void updateVBO()
{
	if (pointCount > 0 && convexHullPtr)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER,
					 sizeof(Vector2f) * pointCount,
					 convexHullPtr->data(),
					 GL_STATIC_DRAW);
	}
}

void updateIBO()
{
	if (!ptIndices.empty())
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					 sizeof(uint32_t) * ptIndices.size(),
					 ptIndices.data(),
					 GL_STATIC_DRAW);
	}
}

void extractIndices(const ConvexHull& mesh)
{
	mesh.extractLineSegmentIndices(ptIndices);
	if (!ptIndices.empty())
	{
		updateIBO();
	}
}

void mouse_button_callback(GLFWwindow* /*window*/, int button, int action, int /*mods*/)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		if (convexHullPtr)
		{
			convexHullPtr->advance();
			extractIndices(*convexHullPtr);
		}
	}
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
	{
		if (convexHullPtr)
		{
			while (convexHullPtr->advance())
			{
				extractIndices(*convexHullPtr);
				window_refresh_callback(window);
			}
		}
	}
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		resetPoints();
		updateVBO();
		updateIBO();
		window_refresh_callback(window);
	}
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
	glfwSetWindowSize(window, width, height);
	glViewport(0, 0, width, height);
}

void window_refresh_callback(GLFWwindow* window)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(vao);

	//glDisable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	glUseProgram(hullShaderProgram);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_LINE_LOOP, ptIndices.size(), GL_UNSIGNED_INT, 0);

	glUseProgram(vertShaderProgram);
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINTS);
	glPointSize(5.0f);
	glDrawArrays(GL_POINTS, 0, pointCount);

	//glfwPollEvents();
	glfwSwapBuffers(window);
}

void bindBuffers()
{
	glGenBuffers(1, &vbo);
	updateVBO();

	glGenBuffers(1, &ibo);
	updateIBO();

	// Bind VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

void bindShaders()
{
	const char *vertShaderString = "#version 410\n"
		"in vec2 vp;"
		"void main () {"
		"	gl_Position = vec4 (vp, 0.0, 1.0);"
		"}";
	const char *hullFragString = "#version 410\n"
		"out vec4 frag_colour;"
		"void main () {"
		"	frag_colour = vec4 (1.0, 1, 1, 1.0);"
		"}";
	const char *vertFragString = "#version 410\n"
		"out vec4 frag_colour;"
		"void main () {"
		"	frag_colour = vec4 (1.0, 0, 0, 1.0);"
		"}";

	vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vertShaderString, nullptr);
	glCompileShader(vertShader);
	hullFragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(hullFragShader, 1, &hullFragString, nullptr);
	glCompileShader(hullFragShader);
	vertFragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertFragShader, 1, &vertFragString, nullptr);
	glCompileShader(vertFragShader);

	hullShaderProgram = glCreateProgram();
	glAttachShader(hullShaderProgram, hullFragShader);
	glAttachShader(hullShaderProgram, vertShader);
	glLinkProgram(hullShaderProgram);

	vertShaderProgram = glCreateProgram();
	glAttachShader(vertShaderProgram, vertFragShader);
	glAttachShader(vertShaderProgram, vertShader);
	glLinkProgram(vertShaderProgram);
	
}