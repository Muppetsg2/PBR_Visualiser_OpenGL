#include <CameraController.h>
#include <Camera.h>
#include <TimeManager.h>

bool CameraController::_mouseNotUsed = true;

GLfloat CameraController::_lastX = 0.f;
GLfloat CameraController::_lastY = 0.f;

float CameraController::_cameraSpeedDefault = 40.f;
float CameraController::_sensitivityDefault = 0.1f;

float CameraController::cameraSpeed = _cameraSpeedDefault;
float CameraController::sensitivity = _sensitivityDefault;

void CameraController::MoveCamera(GLFWwindow* window)
{
    // Forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        Camera::SetPosition(Camera::GetPosition() + Camera::GetFrontDir() * cameraSpeed * TimeManager::GetDeltaTime());
    }
    // Backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        Camera::SetPosition(Camera::GetPosition() - Camera::GetFrontDir() * cameraSpeed * TimeManager::GetDeltaTime());
    }
    // Left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        Camera::SetPosition(Camera::GetPosition() - Camera::GetRight() * cameraSpeed * TimeManager::GetDeltaTime());
    }
    // Right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        Camera::SetPosition(Camera::GetPosition() + Camera::GetRight() * cameraSpeed * TimeManager::GetDeltaTime());
    }
}

void CameraController::RotateCamera(double xpos, double ypos)
{
    if (_mouseNotUsed)
    {
        _lastX = xpos;
        _lastY = ypos;
        _mouseNotUsed = false;
    }

    GLfloat xoffset = xpos - _lastX;
    GLfloat yoffset = _lastY - ypos;
    _lastX = xpos;
    _lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    glm::vec3 rot = Camera::GetRotation();

    // YAW = ROT Y
    // PITCH = ROT X
    // ROLL = ROT Z

    rot.x += yoffset;

    if (rot.x > 89.f) {
        rot.x = 89.f;
    }

    if (rot.x < -89.f)
    {
        rot.x = -89.f;
    }

    Camera::SetRotation(glm::vec3(rot.x, rot.y + xoffset, rot.z));
}

void CameraController::ResetRotation()
{
	_mouseNotUsed = true;
}

void CameraController::DrawEditor(bool* open)
{
    if (!*open) return;

    if (!ImGui::Begin("CameraController", open)) {
        ImGui::End();
        return;
    }

    float totalWidth = ImGui::CalcItemWidth();
    ImVec2 textSize = ImGui::CalcTextSize("Reset");
    float padding = ImGui::GetStyle().FramePadding.x;
    float buttonWidth = textSize.x + 2 * padding;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float dragWidth = totalWidth - buttonWidth - spacing;

    ImGui::PushItemWidth(buttonWidth);
    if (ImGui::Button("Reset##Camera Speed")) {
        cameraSpeed = _cameraSpeedDefault;
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::PushItemWidth(dragWidth);
    ImGui::DragFloat("Camera Speed", &cameraSpeed, 0.001f, 0.0f, FLT_MAX, "%.3f");
    ImGui::PopItemWidth();

    ImGui::PushItemWidth(buttonWidth);
    if (ImGui::Button("Reset##Sensitivity", ImVec2(buttonWidth, 0))) {
        sensitivity = _sensitivityDefault;
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::PushItemWidth(dragWidth);
    ImGui::DragFloat("Sensitivity", &sensitivity, 0.001f, 0.001f, FLT_MAX, "%.3f");
    ImGui::PopItemWidth();

    ImGui::End();
}