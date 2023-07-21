#pragma once

namespace lenny {

class StaticCubemap {
public:
    void load(std::vector<std::string>& filenames);

    GLuint texture = 0;
};

}  // namespace lenny