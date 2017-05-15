#include "ConvexHullApp.h"


void resetPoints()
{
#if 1
	std::cout << "Generated point count (>=3): ";
	std::cin >> pointCount;
#else
	pointCount = 20;
#endif

	if (pointCount < 3)
	{
		pointCount = 3;
	}
	std::vector<Vector2f> pts;
	pts.reserve(pointCount);

	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<> dis(-0.8, 0.8);

	for (size_t i = 0; i < pointCount; i++)
	{
		pts.emplace_back(dis(gen), dis(gen));
	}

	triangulationPtr.reset(new PolygonTriangulation(pts));
	triangulationPtr->extractLineSegmentIndices(ptIndices);
}

int main(int argc, char* argv[])
{
	std::cout << "Operations:\n"
		"\tLeft Mouse Button: Move forward step by step\n"\
		"\tENTER: Finish constructing Convex Hull\n"\
		"\tR: Reset points\n"\
		"\n";

	resetPoints();

	const GLubyte *renderer;
	const GLubyte *version;

	/* start GL context and O/S window using the GLFW helper library */
	if (!glfwInit())
	{
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return 1;
	}

	window = glfwCreateWindow(winSize, winSize, "Convex Hull", nullptr, nullptr);
	if (!window)
	{
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	//glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetWindowRefreshCallback(window, window_refresh_callback);
	glfwSetKeyCallback(window, key_callback);
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

	bindBuffers();
	bindShaders();

	while (!glfwWindowShouldClose(window))
	{
		window_refresh_callback(window);

		glfwPollEvents();
	}

	/* close GL context and any other GLFW resources */
	glfwTerminate();
	return 0;
}