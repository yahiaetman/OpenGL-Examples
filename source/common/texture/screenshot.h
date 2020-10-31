#ifndef GFX_LAB_SCREENSHOT_H
#define GFX_LAB_SCREENSHOT_H

#include <string>

namespace our {

    bool screenshot_png(const std::string& filename, bool include_alpha = false);

}

#endif //GFX_LAB_SCREENSHOT_H
