#include <glad/glad.h>
#include <lenny/gui/Shaders.h>
#include "TestApp.h"

#include <lenny/gui/Guizmo.h>
#include <lenny/gui/ImGui.h>

#include <stb_image.h>

#include <glm/gtc/matrix_transform.hpp>

namespace lenny {

uint loadTexture(const std::string &filePath) {
    uint textureID;
    glGenTextures(1, &textureID);

    stbi_set_flip_vertically_on_load(1);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = 0;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        LENNY_LOG_WARNING("Failed to load texture from path `%s`", filePath.c_str());
    }
    stbi_image_free(data);

    return textureID;
}

TestApp::TestApp() : gui::Application("TestApp") {
    //Setup scene
    const auto [width, height] = getCurrentWindowSize();
    scenes.emplace_back(std::make_shared<gui::Scene>("Scene-1", width, height));
    scenes.back()->f_drawScene = [&]() -> void { drawScene(); };
    scenes.back()->f_mouseButtonCallback = [&](double xPos, double yPos, Ray ray, int button, int action) -> void {
        mouseButtonCallback(xPos, yPos, ray, button, action);
    };
    scenes.back()->f_fileDropCallback = [&](int count, const char** fileNames) -> void { fileDropCallback(count, fileNames); };

    //Load environment textures
    loadSkybox();
}

void TestApp::loadSkybox() {
    //Skybox textures
    std::vector<std::string> filenames = {
        LENNY_GUI_TESTAPP_FOLDER "/config/envmap/top.png",
        LENNY_GUI_TESTAPP_FOLDER "/config/envmap/left.png",
        LENNY_GUI_TESTAPP_FOLDER "/config/envmap/front.png",
        LENNY_GUI_TESTAPP_FOLDER "/config/envmap/right.png",
        LENNY_GUI_TESTAPP_FOLDER "/config/envmap/back.png",
        LENNY_GUI_TESTAPP_FOLDER "/config/envmap/bottom.png"
    };

    //Vertices for a square side of the skybox, each containing position, normal, texCoords
    std::vector<gui::Model::Mesh::Vertex> vertices = {
        {glm::vec3(-1, -1, 1), glm::vec3(0, 0, 1), glm::vec2(0, 0)},
        {glm::vec3(+1, -1, 1), glm::vec3(0, 0, 1), glm::vec2(1, 0)},
        {glm::vec3(+1, +1, 1), glm::vec3(0, 0, 1), glm::vec2(1, 1)},
        {glm::vec3(-1, +1, 1), glm::vec3(0, 0, 1), glm::vec2(0, 1)},
    };

    //Indices for 2 triangles per skybox side
    std::vector<uint> indices = {
        0, 1, 2,
        0, 2, 3
    };

    //Create 6 skybox meshes for each side of the skybox
    for (std::string& filename : filenames) {
        //Create a material by loading the corresponding texture
        gui::Model::Mesh::Material material;
        material.texture_diffuse = loadTexture(filename);

        //Append a new mesh to the skybox
        skybox.emplace_back(vertices, indices, material);
    }
}

void TestApp::drawSkybox() const {
    //Skybox size
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(50));

    gui::Shaders::activeShader->activate();
    gui::Shaders::activeShader->setFloat("objectAlpha", 1.0f);

    //Draw top side
    glm::mat4 transform = glm::rotate(scale, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, glm::radians(180.0f), glm::vec3(0, 0, 1));
    gui::Shaders::activeShader->setMat4("modelPose", transform);
    skybox[0].draw(std::nullopt);

    //Draw left side
    transform = glm::rotate(scale, glm::radians(90.0f), glm::vec3(0, 1, 0));
    gui::Shaders::activeShader->setMat4("modelPose", transform);
    skybox[1].draw(std::nullopt);

    //Draw front side
    transform = glm::rotate(scale, glm::radians(180.0f), glm::vec3(0, 1, 0));
    gui::Shaders::activeShader->setMat4("modelPose", transform);
    skybox[2].draw(std::nullopt);

    //Draw right side
    transform = glm::rotate(scale, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    gui::Shaders::activeShader->setMat4("modelPose", transform);
    skybox[3].draw(std::nullopt);

    //Draw back side
    transform = scale;
    gui::Shaders::activeShader->setMat4("modelPose", transform);
    skybox[4].draw(std::nullopt);

    //Draw bottom side
    transform = glm::rotate(scale, glm::radians(90.0f), glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, glm::radians(180.0f), glm::vec3(0, 0, 1));
    gui::Shaders::activeShader->setMat4("modelPose", transform);
    skybox[5].draw(std::nullopt);
}

void TestApp::drawScene() const {
    //Draw skybox
    drawSkybox();

    //Draw models
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
