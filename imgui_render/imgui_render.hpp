#include <imgui.h>

class imgui_state {
public:
    static imgui_state& instance();

    virtual ~imgui_state() {}
    virtual void new_frame() = 0;
};

