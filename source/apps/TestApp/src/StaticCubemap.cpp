
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <lenny/gui/Renderer.h>
#include <lenny/gui/Shaders.h>
#include <stb_image.h>
#include "StaticCubemap.h"

namespace lenny {

void StaticCubemap::load(std::vector<std::string>& filenames) {
    //Create a new cubemap texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    //Cubemap textures should not be upside-down
    stbi_set_flip_vertically_on_load(0);

    for (int side = 0; side < 6; side++) {
        //Load the image
        int width, height, nrComponents;
        unsigned char *data = stbi_load(filenames[side].c_str(), &width, &height, &nrComponents, 0);
        if (data) {
            GLenum format = 0;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            //Create one side of cubemap texture from image
            GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + side;
            glTexImage2D(target, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

            stbi_image_free(data);
        } else {
            LENNY_LOG_WARNING("Failed to load texture from path `%s`", filenames[side].c_str());
        }

        //Set texture parameters
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

}  // namespace lenny