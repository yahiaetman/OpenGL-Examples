#include "screenshot.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <glad/gl.h>

bool our::screenshot_png(const std::string& filename, bool include_alpha) {

    struct {
        int x = 0, y = 0, w = 0, h = 0;
    } viewport;
    glGetIntegerv(GL_VIEWPORT, (GLint*)&viewport);

    uint8_t components = include_alpha ? 4 : 3;

    auto data = new uint8_t[components * viewport.w * viewport.h];

    glPixelStorei(GL_PACK_ALIGNMENT, include_alpha ? 4 : 1);
    GLenum format = include_alpha ? GL_RGBA : GL_RGB;
    glReadPixels(viewport.x, viewport.y, viewport.w, viewport.h, format, GL_UNSIGNED_BYTE, data);

    stbi_flip_vertically_on_write(true);

    bool saved = stbi_write_png(filename.c_str(), viewport.w, viewport.h, components, data, 0);

    delete [] data;

    return saved;
}
