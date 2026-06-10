#include <sre/ImGui.hpp>

#if SRE_IMGUI

#include <Core/Runtime.hpp>
#include <Core/Display.hpp>

#include <SDL_events.h>

struct sreImGuiBackend
{

};

using namespace sre;

bool imgui::init(bool installevents, bool installrenderevents)
{
    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformUserData = IM_ALLOC(sizeof(sreImGuiBackend));
    io.BackendPlatformName = "imgui_impl_sre";
    io.BackendFlags = ImGuiBackendFlags_None;

    return true;
}

void imgui::shutdown()
{
    ImGuiIO& io = ImGui::GetIO();
    
    IM_FREE(io.BackendPlatformUserData);
}

void imgui::newframe()
{
    sre::vec2ut displaysize = sre::display_size();

    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = sre::dt;
    io.DisplaySize = { displaysize.x, displaysize.y };
}

void imgui::on_event(const sre::Event& event)
{
    ImGuiIO& io = ImGui::GetIO();

    switch (event.type())
    {
        case sre::EVENT_MOUSEBUTTON: {
            auto& mousebutton = event.get<events::MouseButton>();
            io.AddMouseButtonEvent(mousebutton.button, mousebutton.pressed);
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
        } break;
        case sre::EVENT_MOUSEMOVE: {
            auto& mousemove = event.get<events::MouseMove>();
            io.AddMousePosEvent(mousemove.position.x, mousemove.position.y);
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
        } break;
        case sre::EVENT_MOUSEWHEEL: {
            auto& mousewheel = event.get<events::MouseWheel>();
            io.AddMouseWheelEvent(
                static_cast<float>(mousewheel.amount.x),
                static_cast<float>(mousewheel.amount.y)
            );
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
        } break;

        case sre::EVENT_KEYPRESS: {
            auto& keypress = event.get<events::Key>();
            io.AddKeyEvent(ImGuiKey_None /* WIP */, keypress.press != sre::KEY_RELEASED);
        } break;
        default:
            break;
    }
}

#endif