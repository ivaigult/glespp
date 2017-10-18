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

DEF_REFLECTABLE(cube_vertex,
    (glm::vec3, vPos),
    (glm::vec3, vColor)
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

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
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
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_CW);
    glFrontFace(GL_FRONT);

    glespp::buffer_object<cube_vertex> verticies = {
        // positive-x (pink)
        { { 1.f,  1.f,  1.f },{ 0.f, 1.f, 1.f } },
        { { 1.f, -1.f, -1.f },{ 0.f, 1.f, 1.f } },
        { { 1.f,  1.f, -1.f },{ 0.f, 1.f, 1.f } },
        { { 1.f, -1.f, -1.f },{ 0.f, 1.f, 1.f } },
        { { 1.f,  1.f,  1.f },{ 0.f, 1.f, 1.f } },
        { { 1.f, -1.f,  1.f },{ 0.f, 1.f, 1.f } },
        // negative-x (light-blue)
        { {-1.f, -1.f, -1.f}, { 1.f, 0.f, 1.f} },
        { {-1.f, -1.f,  1.f}, { 1.f, 0.f, 1.f} },
        { {-1.f,  1.f,  1.f}, { 1.f, 0.f, 1.f} },
        { {-1.f, -1.f, -1.f}, { 1.f, 0.f, 1.f} },
        { {-1.f,  1.f,  1.f}, { 1.f, 0.f, 1.f} },
        { {-1.f,  1.f, -1.f}, { 1.f, 0.f, 1.f} },
        // postivie-y (yellow)
        { { 1.f,  1.f,  1.f}, { 1.f, 1.f, 0.f} },
        { { 1.f,  1.f, -1.f}, { 1.f, 1.f, 0.f} },
        { {-1.f,  1.f, -1.f}, { 1.f, 1.f, 0.f} },
        { { 1.f,  1.f,  1.f}, { 1.f, 1.f, 0.f} },
        { {-1.f,  1.f, -1.f}, { 1.f, 1.f, 0.f} },
        { {-1.f,  1.f,  1.f}, { 1.f, 1.f, 0.f} },
        // negative-y (red)
        { { 1.f, -1.f,  1.f}, { 1.f, 0.f, 0.f} },
        { {-1.f, -1.f, -1.f}, { 1.f, 0.f, 0.f} },
        { { 1.f, -1.f, -1.f}, { 1.f, 0.f, 0.f} },
        { { 1.f, -1.f,  1.f}, { 1.f, 0.f, 0.f} },
        { {-1.f, -1.f,  1.f}, { 1.f, 0.f, 0.f} },
        { {-1.f, -1.f, -1.f}, { 1.f, 0.f, 0.f} },
        // positive-z (green)
        { {-1.f,  1.f,  1.f}, { 0.f, 1.f, 0.f} },
        { {-1.f, -1.f,  1.f}, { 0.f, 1.f, 0.f} },
        { { 1.f, -1.f,  1.f}, { 0.f, 1.f, 0.f} },
        { { 1.f,  1.f,  1.f}, { 0.f, 1.f, 0.f} },
        { {-1.f,  1.f,  1.f}, { 0.f, 1.f, 0.f} },
        { { 1.f, -1.f,  1.f}, { 0.f, 1.f, 0.f} },
        // negative-z (blue)
        { { 1.f,  1.f, -1.f}, { 0.f, 0.f, 1.f} },
        { {-1.f, -1.f, -1.f}, { 0.f, 0.f, 1.f} },
        { {-1.f,  1.f, -1.f}, { 0.f, 0.f, 1.f} },
        { { 1.f,  1.f, -1.f}, { 0.f, 0.f, 1.f} },
        { { 1.f, -1.f, -1.f}, { 0.f, 0.f, 1.f} },
        { {-1.f, -1.f, -1.f}, { 0.f, 0.f, 1.f} },
    };

    glespp::program<cube_vertex, MyUniform> pr(
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        uniform.MVP = glm::mat4(1.0);
        uniform.MVP = glm::translate(uniform.MVP, glm::vec3(0.0, 0.0, -10.f));
        uniform.MVP = glm::rotate(uniform.MVP, (float)glfwGetTime() / 1.f, glm::vec3(0.0, 1.0, 0.0));
        uniform.MVP = glm::rotate(uniform.MVP, (float)glfwGetTime() / 2.f, glm::vec3(1.0, 0.0, 0.0));
        uniform.MVP = glm::rotate(uniform.MVP, (float)glfwGetTime() / 4.f, glm::vec3(0.0, 0.0, 1.0));
        uniform.MVP = glm::perspective(45.0f, aspect, .1f, 100.f) * uniform.MVP;
                
        pr.set_attribs(verticies);
        pr.set_uniform(uniform);

        pr.execute(glespp::geom_topology::triangles, 0, verticies.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
