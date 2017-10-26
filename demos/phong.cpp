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

#include <imgui.h>
#include <array>

#if defined(_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
    #define GLFW_EXPOSE_NATIVE_WGL
    #include <GLFW/glfw3native.h>
#endif

class imgui_state {
public:
    static imgui_state& instance() {
        static imgui_state s_state;
        return s_state;
    }

    static void new_frame() { instance()._new_frame(); }

private:
    DEF_REFLECTABLE(vertex_t,
        (glm::vec2,   aPosition),
        (glm::vec2,   aUV),
        (glm::u8vec4, aColor)
    );

    DEF_REFLECTABLE(uniform_t, 
        (glm::mat4          , uMVP),
        (glespp::texture_ref, uTexture)
    );

    static_assert(sizeof(vertex_t) == sizeof(ImDrawVert), "Unexpected size");
    static_assert(sizeof(uint16_t) == sizeof(ImDrawIdx), "Unexpected size");

    imgui_state()
        : _main_window(glfwGetCurrentContext())
        , _program(
            "#version 130\n"
            "uniform mat4 uMVP;\n"
            "attribute vec2 aPosition;\n"
            "attribute vec2 aUV;\n"
            "attribute vec4 aColor;\n"
            "varying vec2 vFragUV;\n"
            "varying vec4 vFragColor;\n"
            "void main()\n"
            "{\n"
            "	vFragUV = aUV;\n"
            "	vFragColor = normalize(aColor);\n"
            "	gl_Position = uMVP * vec4(aPosition.xy, 0.0, 1.0);\n"
            "}\n",
            "#version 130\n"
            "uniform sampler2D uTexture;\n"
            "varying vec2 vFragUV;\n"
            "varying vec4 vFragColor;\n"
            "void main()\n"
            "{\n"
            "	gl_FragColor = vFragColor * texture2D( uTexture, vFragUV.st);\n"
            "}\n")
        , _font_texture([]() {
            ImGuiIO& io = ImGui::GetIO();
            glespp::pixel_format::rgba8888* pixels;
            int width, height;
            io.Fonts->GetTexDataAsRGBA32((unsigned char**)&pixels, &width, &height);
            glespp::texture<glespp::pixel_format::rgba8888> result(width, height);
            result.update(0, 0, 0, width, height, pixels);
            return result;
        }())
    {
        _pso.blend.enabled = glespp::boolean::on;
        _pso.blend.equation.rgb     = _pso.blend.equation.alpha     = glespp::blend_equation::add;
        _pso.blend.function.src_rgb = _pso.blend.function.src_alpha = glespp::blend_function::src_alpha;
        _pso.blend.function.dst_rgb = _pso.blend.function.dst_alpha = glespp::blend_function::one_minus_src_alpha;

        ImGuiIO& io = ImGui::GetIO();
        io.RenderDrawListsFn  = &imgui_state::_static_draw;
        io.SetClipboardTextFn = &imgui_state::_set_clipboard;
        io.GetClipboardTextFn = &imgui_state::_get_clipboard;
        io.ClipboardUserData = _main_window;
#if defined(_WIN32)
        io.ImeWindowHandle = glfwGetWin32Window(_main_window);
#endif
        ImGui::GetIO().Fonts->TexID = &_font_texture;
    }

    static void _static_draw(ImDrawData* draw_data) 
    { instance()._draw(draw_data); }
    
    static void _set_clipboard(void* user_data, const char* text) 
    { glfwSetClipboardString((GLFWwindow*)user_data, text); }
    static const char* _get_clipboard(void* user_data)
    { return glfwGetClipboardString((GLFWwindow*)user_data); }

    void _draw(ImDrawData* draw_data) {
        _pso.apply();

        uniform_t uniforms = {};

        ImGuiIO& io = ImGui::GetIO();
        uniforms.uMVP[0] = glm::vec4(2.0f / io.DisplaySize.x, 0.0f, 0.0f, 0.0f);
        uniforms.uMVP[1] = glm::vec4(0.0f, 2.0f / -io.DisplaySize.y, 0.0f, 0.0f);
        uniforms.uMVP[2] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
        uniforms.uMVP[3] = glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f);

        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            size_t start = 0;

            vertex_t* verticies_begin = (vertex_t*)cmd_list->VtxBuffer.Data;
            vertex_t* verticies_end = verticies_begin + cmd_list->VtxBuffer.Size;
            glespp::buffer_object<vertex_t> vertex_buffer(verticies_begin, verticies_end);

            ImDrawIdx* indices_begin = cmd_list->IdxBuffer.Data;
            ImDrawIdx* indices_end = indices_begin + cmd_list->IdxBuffer.Size;
            glespp::buffer_object<ImDrawIdx> index_buffer(indices_begin, indices_end);

            _program.set_attribs(vertex_buffer);

            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback) {
                    pcmd->UserCallback(cmd_list, pcmd);
                } else {
                    uniforms.uTexture = *static_cast<glespp::texture<glespp::pixel_format::rgba8888>*>(pcmd->TextureId);
                    // glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                    _program.set_uniform(uniforms);
                    assert(start + pcmd->ElemCount <= index_buffer.size());
                    _program.execute(glespp::geom_topology::triangles, index_buffer, start, pcmd->ElemCount);
                }
                start += pcmd->ElemCount;
            }
            glFlush();
        }
    }

    void _new_frame() {
        ImGuiIO& io = ImGui::GetIO();
        
        int w, h;
        int display_w, display_h;
        glfwGetWindowSize(_main_window, &w, &h);
        glfwGetFramebufferSize(_main_window, &display_w, &display_h);
        io.DisplaySize = ImVec2((float)w, (float)h);
        io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

        // Setup time step
        double current_time = glfwGetTime();
        io.DeltaTime = static_cast<float>(current_time - _prev_time);
        _prev_time = current_time;

        // Setup inputs
        // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
        if (glfwGetWindowAttrib(_main_window, GLFW_FOCUSED))
        {
            if (io.WantMoveMouse)
            {
                glfwSetCursorPos(_main_window, (double)io.MousePos.x, (double)io.MousePos.y);   // Set mouse position if requested by io.WantMoveMouse flag (used when io.NavMovesTrue is enabled by user and using directional navigation)
            }
            else
            {
                double mouse_x, mouse_y;
                glfwGetCursorPos(_main_window, &mouse_x, &mouse_y);
                io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);   // Get mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
            }
        }
        else
        {
            io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
        }

        for (int i = 0; i < 3; i++)
        {
            // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
            io.MouseDown[i] = _mouse_just_pressed[i] || glfwGetMouseButton(_main_window, i) != 0;
            _mouse_just_pressed[i] = false;
        }

        io.MouseWheel = _mouse_wheel;
        _mouse_wheel = 0.0f;

        // Hide OS mouse cursor if ImGui is drawing it
        glfwSetInputMode(_main_window, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

        // Start the frame
        ImGui::NewFrame();
    }

    void _map_keys() {
        ImGuiIO& io = ImGui::GetIO();
        io.KeyMap[ImGuiKey_Tab]        = GLFW_KEY_TAB;
        io.KeyMap[ImGuiKey_LeftArrow]  = GLFW_KEY_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow]    = GLFW_KEY_UP;
        io.KeyMap[ImGuiKey_DownArrow]  = GLFW_KEY_DOWN;
        io.KeyMap[ImGuiKey_PageUp]     = GLFW_KEY_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown]   = GLFW_KEY_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home]       = GLFW_KEY_HOME;
        io.KeyMap[ImGuiKey_End]        = GLFW_KEY_END;
        io.KeyMap[ImGuiKey_Delete]     = GLFW_KEY_DELETE;
        io.KeyMap[ImGuiKey_Backspace]  = GLFW_KEY_BACKSPACE;
        io.KeyMap[ImGuiKey_Enter]      = GLFW_KEY_ENTER;
        io.KeyMap[ImGuiKey_Escape]     = GLFW_KEY_ESCAPE;
        io.KeyMap[ImGuiKey_A]          = GLFW_KEY_A;
        io.KeyMap[ImGuiKey_C]          = GLFW_KEY_C;
        io.KeyMap[ImGuiKey_V]          = GLFW_KEY_V;
        io.KeyMap[ImGuiKey_X]          = GLFW_KEY_X;
        io.KeyMap[ImGuiKey_Y]          = GLFW_KEY_Y;
        io.KeyMap[ImGuiKey_Z]          = GLFW_KEY_Z;
    }

    GLFWwindow*                                     _main_window;
    double                                          _prev_time;
    glespp::pipeline_state                          _pso;
    glespp::program<vertex_t, uniform_t>            _program;
    glespp::texture<glespp::pixel_format::rgba8888> _font_texture;
    std::array<bool, 3>                             _mouse_just_pressed;
    float                                           _mouse_wheel;
};

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
    
    imgui_state::instance();


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
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        uniform.uMV = glm::mat4(1.0);
        uniform.uMV = glm::translate(uniform.uMV, camera_pos);
        uniform.uMV = glm::rotate(uniform.uMV, (float)glfwGetTime() / 1.f, glm::vec3(0.0, 1.0, 0.0));
        uniform.uNormal = glm::transpose(glm::inverse(uniform.uMV));
        uniform.uMVP = glm::perspective(45.0f, aspect, .1f, 100.f) * uniform.uMV;
                        
        pr.set_attribs(verticies);
        pr.set_uniform(uniform);

        state.apply();
        pr.execute(glespp::geom_topology::triangles, indices);

        imgui_state::new_frame();
        static bool show_another_window = 1;
        static bool show_test_window = 1;
        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        {
            static float f = 0.0f;
            ImGui::Text("Hello, world!");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            if (ImGui::Button("Test Window")) show_test_window ^= 1;
            if (ImGui::Button("Another Window")) show_another_window ^= 1;
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }

        // 2. Show another simple window, this time using an explicit Begin/End pair
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello from another window!");
            ImGui::End();
        }
        
        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
        if (show_test_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
            ImGui::ShowTestWindow(&show_test_window);
        }

        ImGui::Render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
