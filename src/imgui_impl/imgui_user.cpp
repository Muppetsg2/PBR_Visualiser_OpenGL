#include <imgui_impl/imgui_user.h>

IMGUI_API void ImGui::ShowErrorPopup(const char* title, const char* message)
{
    if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_NoResize)) {
        ImGui::SetWindowSize(ImVec2(500, 0), ImGuiCond_Once);
        float available_width = ImGui::GetContentRegionAvail().x;

        float text_width = ImGui::CalcTextSize(message).x;
        ImGui::SetCursorPosX((available_width - text_width) * 0.5f);
        ImGui::Text("%s", message);

        std::string detailMessage = "For more detail info check console.";
        text_width = ImGui::CalcTextSize(detailMessage.c_str()).x;
        ImGui::SetCursorPosX((available_width - text_width) * 0.5f);
        ImGui::Text("%s", detailMessage.c_str());

        float button_width = ImGui::CalcTextSize("OK").x + 20.0f;
        ImGui::SetCursorPosX((available_width - button_width) * 0.5f);
        if (ImGui::Button("OK", ImVec2(button_width, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

IMGUI_API void ImGui::ShowPopup(const char* title, const char* message)
{
    if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_NoResize)) {
        ImGui::SetWindowSize(ImVec2(300, 0), ImGuiCond_Once);
        float available_width = ImGui::GetContentRegionAvail().x;

        std::istringstream stream(message);
        std::string line;
        while (std::getline(stream, line)) {
            float text_width = ImGui::CalcTextSize(line.c_str()).x;
            ImGui::SetCursorPosX((available_width - text_width) * 0.5f);
            ImGui::Text("%s", line.c_str());
        }

        float button_width = ImGui::CalcTextSize("OK").x + 20.0f;
        float offsetX = (ImGui::GetWindowSize().x - button_width) * 0.5f;
        ImGui::SetCursorPosX(offsetX);

        if (ImGui::Button("OK", ImVec2(button_width, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}