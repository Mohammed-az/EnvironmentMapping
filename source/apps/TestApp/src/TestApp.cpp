#include "TestApp.h"

#include <glad/glad.h>
#include <lenny/gui/Guizmo.h>
#include <lenny/gui/ImGui.h>
#include <lenny/gui/Renderer.h>
#include <lenny/gui/Shaders.h>
#include <stb_image.h>

#include <glm/gtc/matrix_transform.hpp>

namespace lenny {

uint loadTexture(const std::string& filePath) {
    //Create a new 2D texture
    uint textureID;
    glGenTextures(1, &textureID);

    //2D textures should be upside-down
    stbi_set_flip_vertically_on_load(1);

    //Load the image
    int width, height, nrComponents;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = 0;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        //Create 2D texture from image
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        //Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        LENNY_LOG_WARNING("Failed to load texture from path `%s`", filePath.c_str());
    }
    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}

uint loadCubemapTexture(std::vector<std::string>& filenames) {
    //Create a new cubemap texture
    uint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    //Cubemap textures should not be upside-down
    stbi_set_flip_vertically_on_load(0);

    for (int i = 0; i < 6; i++) {
        //Load the image
        int width, height, nrComponents;
        unsigned char* data = stbi_load(filenames[i].c_str(), &width, &height, &nrComponents, 0);
        if (data) {
            GLenum format = 0;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            //Create one side of cubemap texture from image
            GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
            glTexImage2D(target, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

            stbi_image_free(data);
        } else {
            LENNY_LOG_WARNING("Failed to load texture from path `%s`", filenames[i].c_str());
        }

        //Set texture parameters
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
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
        //Use ordering of cube sides according to cubemap constants in glad.h
        LENNY_GUI_TESTAPP_FOLDER "/config/envmap/right.png",   // GL_TEXTURE_CUBE_MAP_POSITIVE_X
        LENNY_GUI_TESTAPP_FOLDER "/config/envmap/left.png",    // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
        LENNY_GUI_TESTAPP_FOLDER "/config/envmap/top.png",     // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
        LENNY_GUI_TESTAPP_FOLDER "/config/envmap/bottom.png",  // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
        LENNY_GUI_TESTAPP_FOLDER "/config/envmap/front.png",   // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
        LENNY_GUI_TESTAPP_FOLDER "/config/envmap/back.png"     // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    //Vertices for a square side of the skybox, each containing position, normal, texCoords
    std::vector<gui::Model::Mesh::Vertex> vertices = {
        {glm::vec3(-1, -1, 1), glm::vec3(0, 0, -1), glm::vec2(0, 0)},
        {glm::vec3(+1, -1, 1), glm::vec3(0, 0, -1), glm::vec2(1, 0)},
        {glm::vec3(+1, +1, 1), glm::vec3(0, 0, -1), glm::vec2(1, 1)},
        {glm::vec3(-1, +1, 1), glm::vec3(0, 0, -1), glm::vec2(0, 1)},
    };

    //Indices for 2 triangles per skybox side
    std::vector<uint> indices = {0, 1, 2, 0, 2, 3};

    //Create 6 skybox meshes for each side of the skybox
    for (std::string& filename : filenames) {
        //Create a material by loading the corresponding texture
        gui::Model::Mesh::Material material;
        material.texture_diffuse = loadTexture(filename);

        //Append a new mesh to the skybox
        skybox.emplace_back(vertices, indices, material);
    }

    //Create cubemap texture
    texture_cubemap = loadCubemapTexture(filenames);
}

void TestApp::drawSkybox() const {
    //Skybox size
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(50));

    gui::Shaders::activeShader->activate();
    gui::Shaders::activeShader->setFloat("objectAlpha", 1.0f);

    //Draw right side
    glm::mat4 transform = glm::rotate(scale, glm::radians(90.0f), glm::vec3(0, 1, 0));
    gui::Shaders::activeShader->setMat4("modelPose", transform);
    skybox[0].draw(std::nullopt);

    //Draw left side
    transform = glm::rotate(scale, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    gui::Shaders::activeShader->setMat4("modelPose", transform);
    skybox[1].draw(std::nullopt);

    //Draw top side
    transform = glm::rotate(scale, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    gui::Shaders::activeShader->setMat4("modelPose", transform);
    skybox[2].draw(std::nullopt);

    //Draw bottom side
    transform = glm::rotate(scale, glm::radians(90.0f), glm::vec3(1, 0, 0));
    gui::Shaders::activeShader->setMat4("modelPose", transform);
    skybox[3].draw(std::nullopt);

    //Draw front side
    transform = scale;
    gui::Shaders::activeShader->setMat4("modelPose", transform);
    skybox[4].draw(std::nullopt);

    //Draw back side
    transform = glm::rotate(scale, glm::radians(180.0f), glm::vec3(0, 1, 0));
    gui::Shaders::activeShader->setMat4("modelPose", transform);
    skybox[5].draw(std::nullopt);
}

void TestApp::drawScene() const {
    //Draw skybox without environment mapping
    gui::Shaders::activeShader->activate();
    gui::Shaders::activeShader->setBool("enableEnvironmentMapping", enableEnvironmentMapping);
    gui::Shaders::activeShader->setInt("environmentMappingType", environmentMappingType);
    gui::Shaders::activeShader->setBool("isSkybox", true);
    drawSkybox();
    gui::Shaders::activeShader->setBool("isSkybox", false);

    //Activate cubemap texture unit
    if (texture_cubemap.has_value()) {
        glActiveTexture(GL_TEXTURE1);
        glUniform1i(glGetUniformLocation(gui::Shaders::activeShader->getID(), "texture_cubemap"), 1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture_cubemap.value());
    }

    //Activate cubemap texture unit
    if (texture_cubemap.has_value()) {
        gui::Shaders::activeShader->setBool("useCubemap", true);
        glActiveTexture(GL_TEXTURE1);
        glUniform1i(glGetUniformLocation(gui::Shaders::activeShader->getID(), "texture_cubemap"), 1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture_cubemap.value());
    }

    //Draw models
    std::optional<Eigen::Vector3d> modelColor = std::nullopt;
    if (!showMaterials)
        modelColor = rendererColor.segment(0, 3);
    for (const AppModel& model : models)
        model.mesh.draw(model.position, model.orientation, model.scale, modelColor, rendererColor[3]);

    //Draw a reference sphere to better see the reflections
    if (showReferenceSphere)
        gui::Renderer::I->drawSphere(Eigen::Vector3d(-2, 0, 1), 1.0, Eigen::Vector4d(0.6, 0.6, 0.6, 1));

    //Disable environment mapping for ground
    gui::Shaders::activeShader->setBool("enableEnvironmentMapping", false);
}

void TestApp::drawGui() {
    ImGui::Begin("Menu");

    ImGui::Checkbox("Show materials", &showMaterials);
    if (!showMaterials)
        ImGui::ColorPicker4("Renderer color", rendererColor);

    ImGui::Checkbox("Show reference sphere", &showReferenceSphere);

    ImGui::Separator();

    //Checkbox for environment mapping
    ImGui::Checkbox("Enable environment mapping", &enableEnvironmentMapping);

    if (enableEnvironmentMapping) {
        //Radio buttons for environment mapping type
        ImGui::Text("Mapping type");
        ImGui::Indent();
        ImGui::RadioButton("Add", &environmentMappingType, 0);
        ImGui::RadioButton("Multiply", &environmentMappingType, 1);
        ImGui::RadioButton("Average", &environmentMappingType, 2);
        ImGui::RadioButton("Mix", &environmentMappingType, 3);
        ImGui::Unindent();
    }

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
