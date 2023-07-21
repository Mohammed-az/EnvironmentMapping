#pragma once

namespace lenny {

class DynamicCubemap {
public:
    void create();
    void checkStatus();
    void startUpdating();
    void stopUpdating();
    glm::mat4 selectSide(int face, glm::vec3 position);

    const int width = 256, height = 256;
    GLuint texture = 0;
    GLint previousFramebuffer = 0;
    GLuint framebuffer = 0;
    GLint viewport[4] = {0, 0, 0, 0};
};

}  // namespace lenny