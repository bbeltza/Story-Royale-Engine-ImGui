#ifndef SRE_IMGUI_H
#define SRE_IMGUI_H

#ifdef IMGUI_DISABLE
#undef SRE_IMGUI
#else

#include <Core/Event.hpp>
#include <imgui.h>

#include <Datatypes/UDim.hpp>
#include <Datatypes/Rect.h>

namespace sre
{
    namespace imgui
    {
        bool init(/* bool installevents=true, bool installrenderevents=false */);
        void shutdown();

        // Set custom user viewport, along with optionally, a user-set scale. A scale of 0 (which is the default) will just use the global scale
        void set_viewport(sre::rect2Dut area, sre::unit scale=0);

        sre::rect2Dut get_viewport_area(bool processed=true);
        sre::unit get_viewport_scale(bool processed=true);

        // To be called inside sre::onEvent manually if `installevents` in `init()` is unset
        // Handles all events
        bool on_event(const sre::Event& event);

        // To call usually in sre::beforeRender, but always before any imgui command
        void newframe();
        // To call after imgui commands, preferably in sre::afterRender
        void renderdrawdata(ImDrawData* drawdata);

        // Here in case you want to use it manually, but used in renderdrawdata().
        // Update a texture 
        void updatetexture(ImTextureData* tex);

        // Use this if you ever want to make sre keycodes (equivalent to the SDL ones) to ImGui keys.
        ImGuiKey mapkeycode(sre::keyCode code, sre::scanCode scancode);

        // Helper render function wrappers to avoid confusion
        inline void begin() {
            newframe();
        }
        inline void end(ImDrawData* drawdata=ImGui::GetDrawData()) {
            renderdrawdata(drawdata);
        }
    }    
}

namespace sre
{
    namespace imgui // Engine datatype imgui helpers (WIP)
    {
        bool DragUDimN(const char* label, sre::udim* v, int components, sre::udim v_speed = {0.05, 5.0}, sre::udim v_min = {0.0, 0.0}, sre::udim v_max = {1.0f, 0.0}, const char* format = "%.3f", ImGuiSliderFlags flags = 0);

        bool DragUDim (const char* label, sre::udim*  v, sre::udim v_speed = {0.05, 5.0}, sre::udim v_min = {0.0, 0.0}, sre::udim v_max = {1.0f, 0.0}, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
        bool DragUDim2(const char* label, sre::udim2* v, sre::udim v_speed = {0.05, 5.0}, sre::udim v_min = {0.0, 0.0}, sre::udim v_max = {1.0f, 0.0}, const char* format = "%.3f", ImGuiSliderFlags flags = 0);

        inline bool DragVec2T(const char* label, void* p_data, ImGuiDataType data_type, float v_speed = 1.0f, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, ImGuiSliderFlags flags = 0) {
            return ImGui::DragScalarN(label, data_type, p_data, 2, v_speed, p_min, p_max, format, flags);
        }
        inline bool DragVec2I(const char* label, sre::vec2i* v, float v_speed = 1.0f, sre::vec2i v_min = 0, sre::vec2i v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0) {
            return DragVec2T(label, v, ImGuiDataType_S32, v_speed, &v_min, &v_max, format, flags);
        }
        inline bool DragVec2F(const char* label, sre::vec2f* v, float v_speed = 1.0f, sre::vec2f v_min = 0, sre::vec2f v_max = 0, const char* format = "%.3f", ImGuiSliderFlags flags = 0) {
            return DragVec2T(label, v, ImGuiDataType_Float, v_speed, &v_min, &v_max, format, flags);
        }
        inline bool DragVec2D(const char* label, sre::vec2d* v, float v_speed = 1.0f, sre::vec2d v_min = 0, sre::vec2d v_max = 0, const char* format = "%.3f", ImGuiSliderFlags flags = 0) {
            return DragVec2T(label, v, ImGuiDataType_Double, v_speed, &v_min, &v_max, format, flags);
        }

        inline bool DragRect2DT(const char* label, void* p_data, ImGuiDataType data_type, float v_speed = 1.0f, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, ImGuiSliderFlags flags = 0) {
            return ImGui::DragScalarN(label, data_type, p_data, 4, v_speed, p_min, p_max, format, flags);
        }

        inline bool DragRect2DI(const char* label, sre::rect2Di* v, float v_speed = 1.0f, sre::rect2Di v_min = { 0, 0 }, sre::rect2Di v_max = { 0, 0 }, const char* format = "%d",   ImGuiSliderFlags flags = 0) {
            return DragRect2DT(label, v, ImGuiDataType_S32, v_speed, &v_min, &v_max, format, flags);
        }
        inline bool DragRect2DF(const char* label, sre::rect2Df* v, float v_speed = 1.0f, sre::rect2Df v_min = { 0, 0 }, sre::rect2Df v_max = { 0, 0 }, const char* format = "%.3f", ImGuiSliderFlags flags = 0) {
            return DragRect2DT(label, v, ImGuiDataType_Float, v_speed, &v_min, &v_max, format, flags);
        }
        inline bool DragRect2DD(const char* label, sre::rect2Dd* v, float v_speed = 1.0f, sre::rect2Dd v_min = { 0, 0 }, sre::rect2Dd v_max = { 0, 0 }, const char* format = "%.3f", ImGuiSliderFlags flags = 0) {
            return DragRect2DT(label, v, ImGuiDataType_Double, v_speed, &v_min, &v_max, format, flags);
        }
    
    }
}

#endif

#endif