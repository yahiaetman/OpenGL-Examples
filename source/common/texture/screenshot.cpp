#include "screenshot.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <glad/gl.h>

#include <vector>

bool our::screenshot_png(const std::string& filename, bool include_alpha) {

    // Read the current viewport parameters
    struct {
        int x = 0, y = 0, w = 0, h = 0;
    } viewport;
    glGetIntegerv(GL_VIEWPORT, (GLint*)&viewport);

    // If alpha is included, we have 4 components (RGBA). Otherwise, we only have 3 (RGB).
    uint8_t components = include_alpha ? 4 : 3;

    // Allocate memory to store image
    std::vector<uint8_t> data(components * viewport.w * viewport.h);

    // If alpha is included, each pixel will use 4 bytes so the row would always be divisible by 4.
    // Otherwise, we can only be sure it is divisible by 1 (because everything is divisible by 1).
    glPixelStorei(GL_PACK_ALIGNMENT, include_alpha ? 4 : 1);
    // Pick a format for reading pixels from framebuffer
    GLenum format = include_alpha ? GL_RGBA : GL_RGB;
    // Read Pixels from framebuffer
    glReadPixels(viewport.x, viewport.y, viewport.w, viewport.h, format, GL_UNSIGNED_BYTE, data.data());

    // Since texture row in OpenGL start from bottom and goes up, we need to flip since image formats start from top to bottom.
    stbi_flip_vertically_on_write(true);

    // Save image and return whether it succeeded or not
    return stbi_write_png(filename.c_str(), viewport.w, viewport.h, components, data.data(), 0);
}
