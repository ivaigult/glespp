#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "glespp.hpp"
#include <assets.h>

#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp> 
#include <assimp/scene.h> 
#include <assimp/postprocess.h>

#include <cstdlib>
#include <cstdio>
#include <fstream>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

glm::vec3 camera_pos = glm::vec3(0.0, -0.10, -.5f);

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    case GLFW_KEY_W:
        camera_pos.z += 0.025f;
        break;
    case GLFW_KEY_S:
        camera_pos.z -= 0.025f;
        break;
    case GLFW_KEY_A:
        camera_pos.x -= 0.025f;
        break;
    case GLFW_KEY_D:
        camera_pos.x += 0.025f;
        break;
    }

    std::cout << camera_pos.x <<  ", " << camera_pos.y << ", " << camera_pos.z  << std::endl;
}

DEF_REFLECTABLE(my_vertex,
    (glm::vec3, aPos),
    (glm::vec3, aNorm)
);

DEF_REFLECTABLE(light,
    (glm::vec4, position),
    (glm::vec4, ambient),
    (glm::vec4, diffuse),
    (glm::vec4, specular)
);

DEF_REFLECTABLE(material,
    (glm::vec4, ambient),
    (glm::vec4, diffuse),
    (glm::vec4, specular)
);

DEF_REFLECTABLE(MyUniform,
    (glm::mat4, uMVP),
    (glm::mat4, uMV),
    (glm::mat4, uNormal),
    (material,  uMaterial),
    (light,     uLight)
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

    window = glfwCreateWindow(640, 480, "Phong example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);
    
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(ASSETS_ROOT "/meshes/bunny.obj", aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals);

    std::vector<my_vertex> cpu_verticies;

    assert(1 == scene->mNumMeshes);
    aiMesh* mesh = scene->mMeshes[0];

    cpu_verticies.reserve(mesh->mNumVertices);
    for (size_t ii = 0; ii < mesh->mNumVertices; ++ii) {
        aiVector3D pos = mesh->mVertices[ii];
        aiVector3D normal = mesh->mNormals[ii];
        my_vertex vertex = { glm::vec3(pos.x, pos.y, pos.z), glm::vec3(normal.x, normal.y, normal.z) };
        cpu_verticies.push_back(vertex);
    }

    glespp::buffer_object<my_vertex> verticies(cpu_verticies.begin(), cpu_verticies.end());

    std::vector<uint16_t>  cpu_indicies;
    cpu_indicies.reserve(mesh->mNumFaces * 3);
    for (size_t ii = 0; ii < mesh->mNumFaces; ++ii) {
        const aiFace& face = mesh->mFaces[ii];
        cpu_indicies.push_back(face.mIndices[0]);
        cpu_indicies.push_back(face.mIndices[1]);
        cpu_indicies.push_back(face.mIndices[2]);
    }

    glespp::buffer_object<uint16_t> indices(cpu_indicies.begin(), cpu_indicies.end());


    glespp::pipeline_state state;
    state.depth.enabled         = glespp::boolean::on;
    state.blend.enabled         = glespp::boolean::off;
    state.rasterization.enabled = glespp::boolean::on;
    state.apply();
            
    glespp::program<my_vertex, MyUniform> pr(
        assets::open("/shaders/phong-vertex.glsl"), 
        assets::open("/shaders/phong-fragment.glsl")
    );
    
    MyUniform uniform = {};
    uniform.uLight.position    = glm::vec4(0.0, 0.0, 0.0, 1.0);
    uniform.uMaterial.diffuse  = glm::vec4(0.4, 0.4, 0.4, 1.0);
    uniform.uMaterial.ambient  = glm::vec4(0.1, 0.1, 0.3, 1.0);
    uniform.uMaterial.specular = glm::vec4(0.5, 0.1, 0.1, 1.0);

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        uniform.uMV = glm::mat4(1.0);
        uniform.uMV = glm::translate(uniform.uMV, camera_pos);
        uniform.uMV = glm::rotate(uniform.uMV, (float)glfwGetTime() / 1.f, glm::vec3(0.0, 1.0, 0.0));
        uniform.uNormal = glm::transpose(glm::inverse(uniform.uMV));
        uniform.uMVP = glm::perspective(45.0f, aspect, .1f, 100.f) * uniform.uMV;
                        
        pr.set_attribs(verticies);
        pr.set_uniform(uniform);

        pr.execute(glespp::geom_topology::triangles, indices);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
