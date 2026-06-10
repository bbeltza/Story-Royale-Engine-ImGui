#ifndef SRE_IMGUI_H
#define SRE_IMGUI_H

#ifdef IMGUI_DISABLE
#undef SRE_IMGUI
#else

#include <imgui.h>

#include <Core/Event.hpp>

namespace sre
{
    namespace imgui
    {
        bool init(bool installevents=true, bool installrenderevents=false);
        void shutdown();

        // To be called in sre::onEvent manually if `installevents` in `init()` is unset
        void on_event(const sre::Event& event);

        // To call usually in sre::beforeRender, but always before any imgui command
        void newframe();
        // To call after imgui commands, preferably in sre::afterRender
        void renderdrawdata(ImDrawData* drawdata);

        // Helper render function wrappers to avoid confusion
        inline void begin() {
            newframe();
        }
        inline void end(ImDrawData* drawdata=ImGui::GetDrawData()) {
            renderdrawdata(drawdata);
        }
    }
}

#endif

#endif