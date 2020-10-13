#ifndef OUR_TEXTURE_UTILS_H
#define OUR_TEXTURE_UTILS_H

#include <glad/gl.h>
#include <glm/vec2.hpp>

namespace our::texture_utils {

    glm::ivec2 loadImage(GLuint texture, const char* filename, bool generate_mipmap = true);

}

#endif //OUR_TEXTURE_UTILS_H
