#include "imgui_render.hpp"

#include <glespp.hpp>

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#endif

#include <array>

class imgui_state_impl : public imgui_state {
public:
    void new_frame() {
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

    static imgui_state_impl& instance() {
        static imgui_state_impl s_state;
        return s_state;
    }

private:
    DEF_REFLECTABLE(vertex_t,
        (glm::vec2, aPosition),
        (glm::vec2, aUV),
        (glm::u8vec4, aColor)
    );

    DEF_REFLECTABLE(uniform_t,
        (glm::mat4, uMVP),
        (glespp::texture_ref, uTexture)
    );

    static_assert(sizeof(vertex_t) == sizeof(ImDrawVert), "Unexpected size");
    static_assert(sizeof(uint16_t) == sizeof(ImDrawIdx), "Unexpected size");

    imgui_state_impl()
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
        _pso.blend.equation.rgb = _pso.blend.equation.alpha = glespp::blend_equation::add;
        _pso.blend.function.src_rgb = _pso.blend.function.src_alpha = glespp::blend_function::src_alpha;
        _pso.blend.function.dst_rgb = _pso.blend.function.dst_alpha = glespp::blend_function::one_minus_src_alpha;

        ImGuiIO& io = ImGui::GetIO();
        io.RenderDrawListsFn  =  &imgui_state_impl::_static_draw;
        io.SetClipboardTextFn =  &imgui_state_impl::_set_clipboard;
        io.GetClipboardTextFn =  &imgui_state_impl::_get_clipboard;
        io.ClipboardUserData  = _main_window;
#if defined(_WIN32)
        io.ImeWindowHandle = glfwGetWin32Window(_main_window);
#endif
        ImGui::GetIO().Fonts->TexID = &_font_texture;
    }

    static void _static_draw(ImDrawData* draw_data)
    {
        instance()._draw(draw_data);
    }

    static void _set_clipboard(void* user_data, const char* text)
    {
        glfwSetClipboardString((GLFWwindow*)user_data, text);
    }
    static const char* _get_clipboard(void* user_data)
    {
        return glfwGetClipboardString((GLFWwindow*)user_data);
    }

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
                }
                else {
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

    
    void _map_keys() {
        ImGuiIO& io = ImGui::GetIO();
        io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
        io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
        io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
        io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
        io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
        io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
        io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
        io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
        io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
        io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
        io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
        io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;
    }

    GLFWwindow*                                     _main_window;
    double                                          _prev_time;
    glespp::pipeline_state                          _pso;
    glespp::program<vertex_t, uniform_t>            _program;
    glespp::texture<glespp::pixel_format::rgba8888> _font_texture;
    std::array<bool, 3>                             _mouse_just_pressed;
    float                                           _mouse_wheel;
};


imgui_state& imgui_state::instance() {
    return imgui_state_impl::instance();
}

