#include "GL/glew.h"	/* include GLEW and new version of GL on Windows */
#include "GL/glfw3.h" /* GLFW helper library */

#include "DelaunayTriangulation.h"
#include "Utils.h"

static vector<uint32_t> ptIndices;
static std::unique_ptr<DelaunayTriangulation> delaunyMeshPtr;

static int winSize = 800;
static GLuint vao;
static GLuint vbo, ibo;

static size_t pointCount = 20;

void extractIndices(const DelaunayTriangulation& mesh)
{
	ptIndices.clear();
	mesh.extractTriangleIndices(ptIndices);
	if (!ptIndices.empty())
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * ptIndices.size(), ptIndices.data(), GL_STATIC_DRAW);
	}
}

void mouse_button_callback(GLFWwindow* /*window*/, int button, int action, int /*mods*/)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		if (delaunyMeshPtr)
		{
			/*if (delaunyMeshPtr->keepInsertion())
			{
				extractIndices(*delaunyMeshPtr);
			}*/
		}
	}
}

int main(int argc, char* argv[])
{
	if (argc == 2)
	{
		pointCount = std::atoi(argv[1]);
	}
	else
	{
		std::cout << "Generated point count: ";
		std::cin >> pointCount;
	}

	std::vector<Vector2f> pts;
	pts.reserve(pointCount + 2);
	for (size_t i = 0; i < pointCount; i++)
	{
		pts.emplace_back(Utils::randFloatNum(pointCount),
						 Utils::randFloatNum(pointCount));
	}

	std::vector<Vector2f> renderPts(pts.begin(), pts.end());
	renderPts.emplace_back(1.0f, -100.0f);
	renderPts.emplace_back(1.0f, 100.0f);

	delaunyMeshPtr.reset(new DelaunayTriangulation(pts));
	delaunyMeshPtr->extractTriangleIndices(ptIndices);

	GLFWwindow *window = nullptr;
	const GLubyte *renderer;
	const GLubyte *version;

	const char *vertString = "#version 410\n"
		"in vec2 vp;"
		"void main () {"
		"	gl_Position = vec4 (vp, 0.0, 1.0);"
		"}";
	const char *faceFragString = "#version 410\n"
		"out vec4 frag_colour;"
		"void main () {"
		"	float r = (gl_PrimitiveID % 13 + 1) / 13.0f;"
		"	float g = (gl_PrimitiveID % 7 + 1) / 7.0f;"
		"	float b = (gl_PrimitiveID % 5 + 1) / 5.0f;"
		"	frag_colour = vec4 (r, g, b, 1.0);"
		"}";

	const char *vertexFragString = "#version 410\n"
		"out vec4 frag_colour;"
		"void main () {"
		"	frag_colour = vec4 (1.0f, 0.0f, 0.0f, 1.0);"
		"}";

	GLuint vert_shader, frag_shader, vertexFragShader;
	GLuint faceShaderProgram, vertexShaderProgram;

	/* start GL context and O/S window using the GLFW helper library */
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return 1;
	}

	window = glfwCreateWindow(winSize, winSize, "Delauday Triangle", nullptr, nullptr);
	if (!window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	//glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	/* start GLEW extension handler */
	glewExperimental = GL_TRUE;
	glewInit();

	/* get version info */
	renderer = glGetString(GL_RENDERER); /* get renderer string */
	version = glGetString(GL_VERSION);	 /* version as a string */
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	// Bind buffers
	{
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		if (!renderPts.empty())
		{
			glBufferData(GL_ARRAY_BUFFER,
						 sizeof(Vector2f) * renderPts.size(),
						 renderPts.data(),
						 GL_STATIC_DRAW);
		}

		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		if (!ptIndices.empty())
		{
			glBufferData(GL_ELEMENT_ARRAY_BUFFER,
						 sizeof(uint32_t) * ptIndices.size(),
						 ptIndices.data(),
						 GL_STATIC_DRAW);
		}

		// Bind VAO
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	}

	// Bind shaders
	{
		vert_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vert_shader, 1, &vertString, nullptr);
		glCompileShader(vert_shader);
		frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(frag_shader, 1, &faceFragString, nullptr);
		glCompileShader(frag_shader);
		vertexFragShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(vertexFragShader, 1, &vertexFragString, nullptr);
		glCompileShader(vertexFragShader);

		faceShaderProgram = glCreateProgram();
		glAttachShader(faceShaderProgram, frag_shader);
		glAttachShader(faceShaderProgram, vert_shader);
		glLinkProgram(faceShaderProgram);


		vertexShaderProgram = glCreateProgram();
		glAttachShader(vertexShaderProgram, vertexFragShader);
		glAttachShader(vertexShaderProgram, vert_shader);
		glLinkProgram(vertexShaderProgram);

	}

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.6f, 0.6f, 0.6f, 0.0f);
		glUseProgram(faceShaderProgram);
		glBindVertexArray(vao);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLES, ptIndices.size(), GL_UNSIGNED_INT, 0);

		glUseProgram(vertexShaderProgram);
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINTS);
		glPointSize(10.0f);
		glDrawArrays(GL_POINTS, 0, renderPts.size());

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}