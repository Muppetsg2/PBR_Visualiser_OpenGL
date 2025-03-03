#include <imgui_impl/imgui_style.h>
#include <imgui_internal.h>

void ImGui::StyleColorsNeon(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.19f, 0.94f); // #171730
    colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.09f, 0.19f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.10f, 0.22f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(1.00f, 0.00f, 0.29f, 0.50f); // #ff0049
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.00f, 0.69f, 0.54f); // #cc00af
    colors[ImGuiCol_FrameBgHovered] = ImVec4(1.00f, 0.00f, 0.29f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.00f, 0.93f, 1.00f, 0.67f); // #00eeff
    colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.22f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.80f, 0.00f, 0.69f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.09f, 0.09f, 0.19f, 0.51f);

    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);

    colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.93f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.93f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.00f, 0.29f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.80f, 0.00f, 0.69f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 0.00f, 0.29f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.93f, 1.00f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.80f, 0.00f, 0.69f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(1.00f, 0.00f, 0.29f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.93f, 1.00f, 1.00f);
    colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];

    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);

    colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.93f, 1.00f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.80f, 0.00f, 0.69f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 0.00f, 0.29f, 0.95f);
    colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive] = ImVec4(0.80f, 0.00f, 0.69f, 1.00f);

    colors[ImGuiCol_TabSelected] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabSelectedOverline] = colors[ImGuiCol_HeaderActive];
    colors[ImGuiCol_TabDimmed] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabDimmedSelected] = ImLerp(colors[ImGuiCol_TabSelected], colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
    colors[ImGuiCol_DockingPreview] = colors[ImGuiCol_HeaderActive] * ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextLink] = colors[ImGuiCol_HeaderActive];

    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.80f, 0.00f, 0.69f, 0.35f);

    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavCursor] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);

    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void ImGui::StyleSizesNeon(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();

    // Size
    style->WindowBorderSize = 0.0f;
    style->ScrollbarSize = 15.0f;
    style->GrabMinSize = 17.0f;

    // Rounding
    style->WindowRounding = 0.0f;
    style->ChildRounding = 7.0f;
    style->FrameRounding = 5.0f;
    style->PopupRounding = 7.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabRounding = 4.0f;
    style->TabRounding = 7.0f;

    // Widgets
    style->WindowMenuButtonPosition = ImGuiDir::ImGuiDir_None;
    style->SeparatorTextBorderSize = 3.0f;

    // Docking
    style->DockingSeparatorSize = 2.0f;
}