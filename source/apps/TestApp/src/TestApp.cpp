#include "TestApp.h"

#include <lenny/gui/Guizmo.h>
#include <lenny/gui/ImGui.h>

namespace lenny {

TestApp::TestApp() : gui::Application("TestApp") {
    //Setup scene
    const auto [width, height] = getCurrentWindowSize();
    scenes.emplace_back(std::make_shared<gui::Scene>("Scene-1", width, height));
    scenes.back()->f_drawScene = [&]() -> void { drawScene(); };
    scenes.back()->f_mouseButtonCallback = [&](double xPos, double yPos, Ray ray, int button, int action) -> void {
        mouseButtonCallback(xPos, yPos, ray, button, action);
    };
    scenes.back()->f_fileDropCallback = [&](int count, const char** fileNames) -> void { fileDropCallback(count, fileNames); };
}

void TestApp::drawScene() const {
    std::optional<Eigen::Vector3d> modelColor = std::nullopt;
    if (!showMaterials)
        modelColor = rendererColor.segment(0, 3);
    for (const AppModel& model : models)
        model.mesh.draw(model.position, model.orientation, model.scale, modelColor, rendererColor[3]);
}

void TestApp::drawGui() {
    ImGui::Begin("Menu");

    ImGui::Checkbox("Show Materials", &showMaterials);
    if (!showMaterials)
        ImGui::ColorPicker4("Renderer Color", rendererColor);

    ImGui::End();
}

void TestApp::drawGuizmo() {
    if (selectedModel)
        gui::Guizmo::useWidget(selectedModel->position, selectedModel->orientation, selectedModel->scale);
}

void TestApp::mouseButtonCallback(double xPos, double yPos, Ray ray, int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        selectedModel = nullptr;
        for (AppModel& model : models) {
            const auto hitInfo = model.mesh.hitByRay(model.position, model.orientation, model.scale, ray);
            if (hitInfo.has_value()) {
                selectedModel = &model;
                break;
            }
        }
    }
}

void TestApp::fileDropCallback(int count, const char** fileNames) {
    models.emplace_back(fileNames[count - 1], Eigen::Vector3d::Zero(), Eigen::QuaternionD::Identity(), 1.0);
}

}  // namespace lenny
