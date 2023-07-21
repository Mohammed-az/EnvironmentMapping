
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <lenny/gui/Renderer.h>
#include <lenny/gui/Shaders.h>
#include "DynamicCubemap.h"

namespace lenny {

void DynamicCubemap::create() {
    //Create a new cubemap texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    //Reserve textures
    for (int side = 0; side < 6; side++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    //Set texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //Create a new framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    //Create a new depth buffer
    GLuint depthbuffer = 0;
    glGenRenderbuffers(1, &depthbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    //Attach the color and depth buffers
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer);

    //Check for framebuffer errors
    checkStatus();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void DynamicCubemap::checkStatus() {
    //Report any framebuffer errors
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            break;
        case GL_FRAMEBUFFER_UNDEFINED:
            LENNY_LOG_ERROR("Framebuffer: undefined");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            LENNY_LOG_ERROR("Framebuffer: incomplete attachment point");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            LENNY_LOG_ERROR("Framebuffer: no attached images");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            LENNY_LOG_ERROR("Framebuffer: no color attachment points for drawing");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            LENNY_LOG_ERROR("Framebuffer: no color attachment points for reading");
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            LENNY_LOG_ERROR("Framebuffer: unsupported format");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            LENNY_LOG_ERROR("Framebuffer: inconsistent samples");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            LENNY_LOG_ERROR("Framebuffer: inconsistent layers");
            break;
        default:
            LENNY_LOG_ERROR("Framebuffer: other error");
            break;
    };
}

void DynamicCubemap::startUpdating() {
    //Store the current framebuffer binding and bind the cubemap framebuffer
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    //Store the current viewport and set the cubemap viewport
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, width, height);
}

void DynamicCubemap::stopUpdating() {

    //restore the previous framebuffer binding
    glBindFramebuffer(GL_FRAMEBUFFER, previousFramebuffer);

    //Restore the previous viewport
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

glm::mat4 DynamicCubemap::selectSide(int side, glm::vec3 position) {
    //Attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, side, texture, 0);
    checkStatus();

    //Clear the cubemap viewport
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Create the view matrix for each side
    glm::mat4 view(1.0f);
    switch (side)
    {
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            view = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(+1, 0, 0), glm::vec3(0, -1, 0));
            break;
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            view = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
            break;
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            view = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, +1, 0), glm::vec3(0, 0, +1));
            break;
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            view = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
            break;
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            view = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, +1), glm::vec3(0, -1, 0));
            break;
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            view = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
            break;
    };

    //Set the point of view
    view = glm::translate(view, -position);
    return view;
}

}  // namespace lenny