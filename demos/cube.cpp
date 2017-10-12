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

DEF_REFLECTABLE(CubeVertex,
    (glm::vec3, vPos)
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

    window = glfwCreateWindow(480, 480, "Texture example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);
    
    glespp::buffer_object<CubeVertex> verticies = {
        { { -1.f, -1.f, -1.f } }, // <- 0
        { { -1.f,  1.f, -1.f } }, // <- 1
        { {  1.f, -1.f, -1.f } }, // <- 2
        { {  1.f,  1.f, -1.f } }, // <- 3
        { { -1.f, -1.f,  1.f } }, // <- 4
        { { -1.f,  1.f,  1.f } }, // <- 5
        { {  1.f, -1.f,  1.f } }, // <- 6
        { {  1.f,  1.f,  1.f } }, // <- 7
    };

    glespp::buffer_object<uint16_t> indices = {
        0, 1, 2,   2, 1, 3, // face-0
        1, 5, 3,   3, 5, 7, // face-1
        5, 4, 6,   6, 7, 5, // face-2
        4, 0, 6,   6, 2, 0, // face-3
        4, 5, 1,   1, 0, 4, // face-4
        2, 3, 6,   6, 7, 3  // face-5
    };

    glespp::program<CubeVertex, MyUniform> pr(
        assets::open("/shaders/cube-vertex.glsl"), 
        assets::open("/shaders/cube-fragment.glsl")
    );
    
    MyUniform uniform;

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = width / (float)height;


        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        uniform.MVP = glm::mat4(1.0);
        uniform.MVP = glm::translate(uniform.MVP, glm::vec3(0.0, 0.0, -10.f));
        uniform.MVP = glm::rotate(uniform.MVP, (float)glfwGetTime() / 1.f, glm::vec3(0.0, 1.0, 0.0));
        uniform.MVP = glm::rotate(uniform.MVP, (float)glfwGetTime() / 2.f, glm::vec3(1.0, 0.0, 0.0));
        uniform.MVP = glm::rotate(uniform.MVP, (float)glfwGetTime() / 4.f, glm::vec3(0.0, 0.0, 1.0));
        uniform.MVP = glm::perspective(45.0f, aspect, .1f, 100.f) * uniform.MVP;
        
                
        pr.set_attribs(verticies);
        pr.set_uniform(uniform);

        pr.execute(glespp::geom_topology::triangles, indices, 0, indices.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
