#include <sre/imgui.hpp>

#if SRE_IMGUI

#include <Core/Render.h>
#include <Core/Runtime.hpp>
#include <Core/Window.hpp>

#include <SDL_events.h>

#include <Base/Log.h>

struct sreImGuiBackend
{
    sre::rect2Dut vprect{ 0.0_ut, 0.0_ut };
    sre::unit vpscale{ 0.0_ut };
    sre::unit vpscaleratio{ 0.0_ut };
};

using namespace sre;

bool imgui::init(/*bool installevents, bool installrenderevents*/)
{
    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformUserData = IM_NEW(sreImGuiBackend);
    io.BackendPlatformName = "imgui_impl_sre";
    io.BackendFlags = ImGuiBackendFlags_None;

    io.BackendRendererName = "imgui_impl_sre_render";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

    io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines;

    return true;
}

void imgui::shutdown()
{
    ImGuiIO& io = ImGui::GetIO();
    
    IM_FREE(io.BackendPlatformUserData);
    io.BackendPlatformUserData = NULL;
}

void imgui::set_viewport(sre::rect2Dut area, sre::unit scale)
{
    IM_ASSERT(scale >= 0);

    ImGuiIO& io = ImGui::GetIO();
    auto backend = static_cast<sreImGuiBackend*>(io.BackendPlatformUserData);
    IM_ASSERT(backend != NULL);

    backend->vprect = area;
    backend->vpscale = scale;
    backend->vpscaleratio = scale ? 1/scale : 0;
}

sre::rect2Dut sre::imgui::get_viewport_area(bool processed) {
    ImGuiIO& io = ImGui::GetIO();
    auto backend = static_cast<sreImGuiBackend*>(io.BackendPlatformUserData);
    IM_ASSERT(backend != NULL);

    return !processed ? backend->vprect : sre::rect2Dut{
        backend->vprect.position,
        sre::calc_viewport_size(backend->vprect, backend->vpscale)
    };
}

sre::unit sre::imgui::get_viewport_scale(bool processed) {
    ImGuiIO& io = ImGui::GetIO();
    auto backend = static_cast<sreImGuiBackend*>(io.BackendPlatformUserData);
    IM_ASSERT(backend != NULL);

    return !processed || backend->vpscale ? backend->vpscale : sre::window_getscale();
}

void imgui::newframe()
{
    ImGuiIO& io = ImGui::GetIO();
    auto backend = static_cast<sreImGuiBackend*>(io.BackendPlatformUserData);
    IM_ASSERT(backend != NULL);

    sre::vec2ut displaysize = sre::calc_viewport_size(backend->vprect, backend->vpscale);
    displaysize = displaysize.ceil();
    
    io.DeltaTime = sre::dt;
    io.DisplaySize = { displaysize.x, displaysize.y };
}

void imgui::renderdrawdata(ImDrawData* drawdata)
{
    static thread_local std::vector<sre::RenderPoint> vtx_buf;

    ImGuiIO& io = ImGui::GetIO();
    ImGuiPlatformIO& plat_io = ImGui::GetPlatformIO();
    auto backend = static_cast<sreImGuiBackend*>(io.BackendPlatformUserData);
    IM_ASSERT(backend != NULL);

    if (drawdata->Textures != nullptr)
        for (ImTextureData* tex : *drawdata->Textures)
            if (tex->Status != ImTextureStatus_OK)
                imgui::updatetexture(tex);

    if (!sre::render::has_begun())
        sre::render::begin(sre::BLACK, sre::vec2ut::ZERO);

    sre::rect2Dut old_viewport;
    sre::unit old_scale;
    sre_render_get_viewport(&old_viewport, NULL, &old_scale);
    
    sre::blendMode old_blendmode = sre::render::get_blendmode();
    sre::rect2Dut old_scissor = sre::render::get_scissors_area();

    sre::render::set_viewport(backend->vprect, backend->vpscale);
    sre::render::set_blendmode(SRE_BLEND_BLEND);

    for (const ImDrawList* drawlist : drawdata->CmdLists)
    {
        for (const ImDrawCmd& pcmd : drawlist->CmdBuffer) {
            if (pcmd.UserCallback)
            {
                if (pcmd.UserCallback != plat_io.DrawCallback_ResetRenderState)
                    pcmd.UserCallback(drawlist, &pcmd);
            }
            else
            {
                vtx_buf.reserve(pcmd.ElemCount);
                vtx_buf.clear();

                for (unsigned i = 0; i < pcmd.ElemCount; i++) {
                    const ImDrawVert& vert = drawlist->VtxBuffer[drawlist->IdxBuffer[pcmd.IdxOffset + i]];
                    vtx_buf.push_back({
                        { vert.pos.x, vert.pos.y },
                        { vert.uv.x, vert.uv.y },
                        reinterpret_cast<const sre::col4&>(vert.col)
                    });
                }

                sre::rect2Dut r{
                    pcmd.ClipRect.x,
                    pcmd.ClipRect.y,
                    pcmd.ClipRect.z - pcmd.ClipRect.x,
                    pcmd.ClipRect.w - pcmd.ClipRect.y
                };
                r.position += backend->vprect.position;

                sre::render::set_scissors(r);
                sre::render::draw2(0, vtx_buf.data(),vtx_buf.size(), SRE_PRIMITIVE_TRIANGLES, reinterpret_cast<sre::Texture*>(pcmd.GetTexID()));
            }
        }
    }

    sre::render::set_viewport(old_viewport, old_scale);
    sre::render::set_scissors(old_scissor);
    sre::render::set_blendmode(old_blendmode);
}

void imgui::updatetexture(ImTextureData* tex)
{
    if (tex->Status == ImTextureStatus_WantCreate) {
        sre::Texture* sretexture = sre::texture(SDL_PIXELFORMAT_RGBA32, tex->Width, tex->Height);
        sretexture->update(tex->GetPixels(), tex->GetPitch());

        tex->SetTexID(reinterpret_cast<ImTextureID>(sretexture));
        tex->SetStatus(ImTextureStatus_OK);
    }
    if (tex->Status == ImTextureStatus_WantUpdates) {
        sre::Texture* sretexture = reinterpret_cast<sre::Texture*>(tex->GetTexID());
        assert(sretexture != NULL);

        for (ImTextureRect& r : tex->Updates)
        {
            sre::rect2Di region = {
                r.x,
                r.y,
                r.w,
                r.h,
            };
            
            sretexture->update(&region, tex->GetPixelsAt(r.x, r.y), tex->GetPitch());
        }

        tex->SetStatus(ImTextureStatus_OK);
    }
    if (tex->Status == ImTextureStatus_WantDestroy) {
        sre::Texture* sretexture = reinterpret_cast<sre::Texture*>(tex->GetTexID());
        assert(sretexture != NULL);

        sretexture->release();

        tex->SetTexID(ImTextureID_Invalid);
        tex->SetStatus(ImTextureStatus_Destroyed);
    }
}

bool imgui::on_event(const sre::Event& event)
{
    ImGuiIO& io = ImGui::GetIO();
    auto backend = static_cast<sreImGuiBackend*>(io.BackendPlatformUserData);
    IM_ASSERT(backend != NULL);

    bool ishandled = true;

    switch (event.type())
    {
        case sre::EVENT_MOUSEBUTTON: {
            auto& mousebutton = event.get<events::MouseButton>();
            int button = ImGuiMouseButton_COUNT;
            switch (mousebutton.button)
            {
                case sre::MB_LEFT: button = ImGuiMouseButton_Left; break;
                case sre::MB_RIGHT: button = ImGuiMouseButton_Right; break;
                case sre::MB_MIDDLE: button = ImGuiMouseButton_Middle; break;
                default: break;
            }

            if (button == ImGuiMouseButton_COUNT)
                break;

            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(button, mousebutton.pressed);
        } break;
        case sre::EVENT_MOUSEMOVE: {
            auto& mousemove = event.get<events::MouseMove>();
            sre::vec2ut pos = mousemove.process_position(backend->vprect.position, backend->vpscale);
            io.AddMousePosEvent(pos.x, pos.y);
        } break;
        case sre::EVENT_MOUSEWHEEL: {
            auto& mousewheel = event.get<events::MouseWheel>();
            io.AddMouseWheelEvent(
                static_cast<float>(mousewheel.amount.x),
                static_cast<float>(mousewheel.amount.y)
            );
        } break;

        case sre::EVENT_KEYPRESS: {
            auto& keypress = event.get<events::Key>();
            ImGuiKey key = imgui::mapkeycode(keypress.keycode, keypress.scancode);
            io.AddKeyEvent(key, keypress.press != sre::KEY_RELEASED);
            io.SetKeyEventNativeData(key, keypress.keycode, keypress.scancode);
        } break;
        case sre::EVENT_TEXTINPUT: {
            auto& textinput = event.get<events::TextInput>();
            io.AddInputCharacter(textinput.codepoint);
        } break;
        default:
            ishandled = false;
            break;
    }

    return ishandled;
}

ImGuiKey imgui::mapkeycode(sre::keyCode code, sre::scanCode scancode) {
    switch (code)
    {
        case KB_RETURN: return ImGuiKey_Enter;
        case KB_ESCAPE: return ImGuiKey_Escape;
        case KB_BACKSPACE: return ImGuiKey_Backspace;
        case KB_TAB: return ImGuiKey_Tab;
        case KB_SPACE: return ImGuiKey_Space;

        case KB_0: return ImGuiKey_0;
        case KB_1: return ImGuiKey_1;
        case KB_2: return ImGuiKey_2;
        case KB_3: return ImGuiKey_3;
        case KB_4: return ImGuiKey_4;
        case KB_5: return ImGuiKey_5;
        case KB_6: return ImGuiKey_6;
        case KB_7: return ImGuiKey_7;
        case KB_8: return ImGuiKey_8;
        case KB_9: return ImGuiKey_9;

        case KB_a: return ImGuiKey_A;
        case KB_b: return ImGuiKey_B;
        case KB_c: return ImGuiKey_C;
        case KB_d: return ImGuiKey_D;
        case KB_e: return ImGuiKey_E;
        case KB_f: return ImGuiKey_F;
        case KB_g: return ImGuiKey_G;
        case KB_h: return ImGuiKey_H;
        case KB_i: return ImGuiKey_I;
        case KB_j: return ImGuiKey_J;
        case KB_k: return ImGuiKey_K;
        case KB_l: return ImGuiKey_L;
        case KB_m: return ImGuiKey_M;
        case KB_n: return ImGuiKey_N;
        case KB_o: return ImGuiKey_O;
        case KB_p: return ImGuiKey_P;
        case KB_q: return ImGuiKey_Q;
        case KB_r: return ImGuiKey_R;
        case KB_s: return ImGuiKey_S;
        case KB_t: return ImGuiKey_T;
        case KB_u: return ImGuiKey_U;
        case KB_v: return ImGuiKey_V;
        case KB_w: return ImGuiKey_W;
        case KB_x: return ImGuiKey_X;
        case KB_y: return ImGuiKey_Y;
        case KB_z: return ImGuiKey_Z;
        case KB_CAPSLOCK: return ImGuiKey_CapsLock;
        case KB_F1: return ImGuiKey_F1;
        case KB_F2: return ImGuiKey_F2;
        case KB_F3: return ImGuiKey_F3;
        case KB_F4: return ImGuiKey_F4;
        case KB_F5: return ImGuiKey_F5;
        case KB_F6: return ImGuiKey_F6;
        case KB_F7: return ImGuiKey_F7;
        case KB_F8: return ImGuiKey_F8;
        case KB_F9: return ImGuiKey_F9;
        case KB_F10: return ImGuiKey_F10;
        case KB_F11: return ImGuiKey_F11;
        case KB_F12: return ImGuiKey_F12;
        case KB_PRINTSCREEN: return ImGuiKey_PrintScreen;
        case KB_SCROLLLOCK: return ImGuiKey_ScrollLock;
        case KB_PAUSE: return ImGuiKey_Pause;
        case KB_INSERT: return ImGuiKey_Insert;
        case KB_HOME: return ImGuiKey_Home;
        case KB_PAGEUP: return ImGuiKey_PageUp;
        case KB_DELETE: return ImGuiKey_Delete;
        case KB_END: return ImGuiKey_End;
        case KB_PAGEDOWN: return ImGuiKey_PageDown;
        case KB_RIGHT: return ImGuiKey_RightArrow;
        case KB_LEFT: return ImGuiKey_LeftArrow;
        case KB_DOWN: return ImGuiKey_DownArrow;
        case KB_UP: return ImGuiKey_UpArrow;
        case KB_NUMLOCKCLEAR: return ImGuiKey_NumLock;
        case KB_KP_DIVIDE: return ImGuiKey_KeypadDivide;
        case KB_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case KB_KP_MINUS: return ImGuiKey_KeypadSubtract;
        case KB_KP_PLUS: return ImGuiKey_KeypadAdd;
        case KB_KP_ENTER: return ImGuiKey_KeypadEnter;
        case KB_KP_1: return ImGuiKey_Keypad1;
        case KB_KP_2: return ImGuiKey_Keypad2;
        case KB_KP_3: return ImGuiKey_Keypad3;
        case KB_KP_4: return ImGuiKey_Keypad4;
        case KB_KP_5: return ImGuiKey_Keypad5;
        case KB_KP_6: return ImGuiKey_Keypad6;
        case KB_KP_7: return ImGuiKey_Keypad7;
        case KB_KP_8: return ImGuiKey_Keypad8;
        case KB_KP_9: return ImGuiKey_Keypad9;
        case KB_KP_0: return ImGuiKey_Keypad0;
        case KB_KP_PERIOD: return ImGuiKey_KeypadDecimal;
        case KB_APPLICATION: return ImGuiKey_Menu;

        case KB_KP_EQUALS: return ImGuiKey_KeypadEqual;
        case KB_F13: return ImGuiKey_F13;
        case KB_F14: return ImGuiKey_F14;
        case KB_F15: return ImGuiKey_F15;
        case KB_F16: return ImGuiKey_F16;
        case KB_F17: return ImGuiKey_F17;
        case KB_F18: return ImGuiKey_F18;
        case KB_F19: return ImGuiKey_F19;
        case KB_F20: return ImGuiKey_F20;
        case KB_F21: return ImGuiKey_F21;
        case KB_F22: return ImGuiKey_F22;
        case KB_F23: return ImGuiKey_F23;
        case KB_F24: return ImGuiKey_F24;
        //case KB_MENU: return ImGuiKey_Menu;
        //case KB_KP_COMMA: return ImGuiKey_KeypadDecimal;
        //case KB_KP_DECIMAL: return ImGuiKey_KeypadDecimal;
        
        case KB_LCTRL: return ImGuiKey_LeftCtrl;
        case KB_LSHIFT: return ImGuiKey_LeftShift;
        case KB_LALT: return ImGuiKey_LeftAlt;
        case KB_LGUI: return ImGuiKey_LeftSuper;
        case KB_RCTRL: return ImGuiKey_RightCtrl;
        case KB_RSHIFT: return ImGuiKey_RightShift;
        case KB_RALT: return ImGuiKey_RightAlt;
        case KB_RGUI: return ImGuiKey_RightSuper;

        default: break;
    }

    // Fallback to scancodes
    switch (scancode)
    {
        case KBSCANCODE_GRAVEACCENT: return ImGuiKey_GraveAccent;
        case KBSCANCODE_MINUS: return ImGuiKey_Minus;
        case KBSCANCODE_EQUALS: return ImGuiKey_Equal;
        case KBSCANCODE_LEFTBRACKET: return ImGuiKey_LeftBracket;
        case KBSCANCODE_RIGHTBRACKET: return ImGuiKey_RightBracket;
        case KBSCANCODE_BACKSLASH: return ImGuiKey_Backslash;
        case KBSCANCODE_SEMICOLON: return ImGuiKey_Semicolon;
        case KBSCANCODE_APOSTROPHE: return ImGuiKey_Apostrophe;
        case KBSCANCODE_COMMA: return ImGuiKey_Comma;
        case KBSCANCODE_PERIOD: return ImGuiKey_Period;
        case KBSCANCODE_SLASH: return ImGuiKey_Slash;

        case KBSCANCODE_NONUSBACKSLASH: return ImGuiKey_Oem102;
        default: return ImGuiKey_None;
    }
}

#endif