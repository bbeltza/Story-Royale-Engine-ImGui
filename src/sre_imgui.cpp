#include <sre/imgui.hpp>
#include <imgui_internal.h>

// This is required to define since it's static inside `imgui_widgets.cpp` and is not returned from any function...
static const ImU32 GDefaultRgbaColorMarkers[4] = {
    IM_COL32(240,20,20,255), IM_COL32(20,240,20,255), IM_COL32(20,20,240,255), IM_COL32(140,140,140,255)
};

inline void IM_PUSH_ID_UDIM(int i, ImGuiSliderFlags flags, ImGuiContext& g) {
    ImGui::PushID(i);
    if (i > 0)
        ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
    if (flags & ImGuiSliderFlags_ColorMarkers)
        ImGui::SetNextItemColorMarker(GDefaultRgbaColorMarkers[i]);
}

bool sre::imgui::DragUDimN(const char* label, sre::udim* v, int components, sre::udim v_speed, sre::udim v_min, sre::udim v_max, const char* format, ImGuiSliderFlags flags) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    bool value_changed = false;
    ImGui::BeginGroup();
    ImGui::PushID(label);
    ImGui::PushMultiItemsWidths(components*2, ImGui::CalcItemWidth());
    for (int i = 0; i < components; i++)
    {
        IM_PUSH_ID_UDIM(i*2+0, flags, g);
        value_changed |= ImGui::DragFloat("", &v->scale, v_speed.scale, v_min.scale, v_max.scale, format, flags);
        ImGui::PopID();
        IM_PUSH_ID_UDIM(i*2+1, flags, g);
        value_changed |= ImGui::DragFloat("", &v->offset, v_speed.offset, v_min.offset, v_max.offset, format, flags);
        ImGui::PopID();

        ImGui::PopItemWidth();
        ImGui::PopItemWidth();
        v++;
    }
    ImGui::PopID();

    const char* label_end = ImGui::FindRenderedTextEnd(label);
    if (label != label_end)
    {
        ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
        ImGui::TextEx(label, label_end);
    }

    ImGui::EndGroup();
    return value_changed;
}

bool sre::imgui::DragUDim(const char* label, sre::udim* v, sre::udim v_speed, sre::udim v_min, sre::udim v_max, const char* format, ImGuiSliderFlags flags) {
    return DragUDimN(label, v, 1, v_speed, v_min, v_max, format, flags);
}

bool sre::imgui::DragUDim2(const char* label, sre::udim2* v, sre::udim v_speed, sre::udim v_min, sre::udim v_max, const char* format, ImGuiSliderFlags flags) {
    return DragUDimN(label, &v->x, 2, v_speed, v_min, v_max, format, flags);
}

static constexpr float F = 1/255.0f;

bool sre::imgui::Col3Edit(const char *label, sre::col3 *col, ImGuiColorEditFlags flags) {
    float fcol[3]{
        col->r * F,
        col->g * F,
        col->b * F
    };

    if (ImGui::ColorEdit3(label, fcol, flags)) {
        *col = sre::col3::fromNormalized(fcol[0], fcol[1], fcol[2]);
        return true;
    }
    
    return false;
}

bool sre::imgui::Col4Edit(const char *label, sre::col4 *col, ImGuiColorEditFlags flags) {
    float fcol[4]{
        col->r * F,
        col->g * F,
        col->b * F,
        col->a * F
    };

    if (ImGui::ColorEdit4(label, fcol, flags)) {
        *col = sre::col4::fromNormalized(fcol[0], fcol[1], fcol[2], fcol[3]);
        return true;
    }
    
    return false;
}

bool sre::imgui::AlignmentCombo(const char* label, sre::alignment* p, bool vertical)
{
    const char* const hvalues = "sre::ALIGN_LEFT\0sre::ALIGN_CENTER\0sre::ALIGN_RIGHT\0";
    const char* const vvalues = "sre::ALIGN_TOP\0sre::ALIGN_CENTER\0sre::ALIGN_BOTTOM\0";
    
    int item = *p;
    const char* values = vertical ? vvalues : hvalues;
    if (ImGui::Combo(label, &item, values)) {
        *p = static_cast<sre::alignment>(item);
        return true;
    }
    
    return false;
}
