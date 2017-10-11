#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "glespp.hpp"
#include <assets.h>

#include <glm/gtc/matrix_transform.hpp>

#include <cstdlib>
#include <cstdio>
#include <fstream>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

DEF_REFLECTABLE(MyVertex,
    (glm::vec2, vPos),
    (glm::vec3, vCol)
);

DEF_REFLECTABLE(MyUniform,
    (glm::mat4, MVP)
);

int main(void)
{
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(640, 480, "Texture example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);


    glespp::buffer_object<MyVertex> verticies = {
        { {-0.6f, -0.4f }, { 1.f, 0.f, 0.f} },
        { {0.6f,  -0.4f }, { 0.f, 1.f, 0.f} },
        { {0.f,    0.6f }, { 0.f, 0.f, 1.f} }
    };

    glespp::texture<glespp::pixel_format::rgb888> texture(2, 2);
    glespp::pixel_format::rgb888 data[] = { {1, 0, 0},{ 0, 1, 0 },{ 0, 0, 1 },{ 1, 0, 1 } };
    texture.update(0, 0, 0, 2, 2, data);

    glespp::program<MyVertex, MyUniform> pr(assets::open("/shaders/vertex.glsl"), assets::open("/shaders/fragment.glsl"));
    MyUniform uniform;

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float ratio = width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        uniform.MVP = glm::ortho(-ratio, ratio, -1.f, 1.f);

        pr.set_attribs(verticies);
        pr.set_uniform(uniform);
        pr.execute(glespp::geom_topology::triangles, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
