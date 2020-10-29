#include "texture-utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>

glm::ivec2 our::texture_utils::loadImage(GLuint texture, const char *filename, bool generate_mipmap) {
    glm::ivec2 size;
    int channels;
    //Since OpenGL puts the texture origin at the bottom left while images typically has the origin at the top left,
    //We need to till stb to flip images vertically after loading them
    stbi_set_flip_vertically_on_load(true);
    //Load image data and retrieve width, height and number of channels in the image
    //The last argument is the number of channels we want and it can have the following values:
    //- 0: Keep number of channels the same as in the image file
    //- 1: Grayscale only
    //- 2: Grayscale and Alpha
    //- 3: RGB
    //- 4: RGB and Alpha
    //Note: channels (the 4th argument) always returns the original number of channels in the file
    unsigned char* data = stbi_load(filename, &size.x, &size.y, &channels, 4);
    if(data == nullptr){
        std::cerr << "Failed to load image: " << filename << std::endl;
        return {0, 0};
    }
    //Bind the texture such that we upload the image data to its storage
    glBindTexture(GL_TEXTURE_2D, texture);
    //Set Unpack Alignment to 4-byte (it means that each row takes multiple of 4 bytes in memory)
    //Note: this is not necessary since:
    //- Alignment is 4 by default
    //- Alignment of 1 or 2 will still work correctly but 8 may cause problems
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    //Send data to texture
    //NOTE: the internal format is set to GL_RGBA8 so every pixel contains 4 bytes, one for each channel
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //Generate versions of the texture at smaller level of details (useful for filtering)
    if(generate_mipmap) glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data); //Free image data after uploading to GPU
    return size;
}

glm::ivec2 our::texture_utils::loadImageGrayscale(GLuint texture, const char *filename, bool generate_mipmap) {
    glm::ivec2 size;
    int channels;
    //Since OpenGL puts the texture origin at the bottom left while images typically has the origin at the top left,
    //We need to till stb to flip images vertically after loading them
    stbi_set_flip_vertically_on_load(true);
    //Load image data and retrieve width, height and number of channels in the image
    //The last argument is the number of channels we want and it can have the following values:
    //- 0: Keep number of channels the same as in the image file
    //- 1: Grayscale only
    //- 2: Grayscale and Alpha
    //- 3: RGB
    //- 4: RGB and Alpha
    //Note: channels (the 4th argument) always returns the original number of channels in the file
    unsigned char* data = stbi_load(filename, &size.x, &size.y, &channels, 1);
    if(data == nullptr){
        std::cerr << "Failed to load image: " << filename << std::endl;
        return {0, 0};
    }
    //Bind the texture such that we upload the image data to its storage
    glBindTexture(GL_TEXTURE_2D, texture);
    //Set Unpack Alignment to 1-byte (it means that each row takes multiple of 1 bytes in memory)
    //Note: Alignment is 4 by default which may not work for grayscale images if the row size is not divisible by 4.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //Send data to texture
    //NOTE: the internal format is set to GL_R8 so every pixel contains 1 byte only
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, size.x, size.y, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    //Generate versions of the texture at smaller level of details (useful for filtering)
    if(generate_mipmap) glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data); //Free image data after uploading to GPU
    return size;
}

void our::texture_utils::singleColor(GLuint texture, our::Color color, glm::ivec2 size){
    //Allocate array for texture data
    auto* data = new Color[size.x * size.y];
    //Fill array with the same color
    std::fill_n(data, size.x * size.y, color);
    //Bind the texture such that we upload the image data to its storage
    glBindTexture(GL_TEXTURE_2D, texture);
    //Set Unpack Alignment to 4-byte (it means that each row takes multiple of 4 bytes in memory)
    //Note: this is not necessary since:
    //- Alignment is 4 by default
    //- Alignment of 1 or 2 will still work correctly but 8 will cause problems
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    //Send data to texture
    //NOTE: the internal format is set to GL_RGBA8 so every pixel contains 4 bytes, one for each channel
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //Generate Mipmaps after loading the texture
    glGenerateMipmap(GL_TEXTURE_2D);
    delete[] data;
}

void our::texture_utils::checkerBoard(GLuint texture, glm::ivec2 size, glm::ivec2 patternSize, our::Color color1, our::Color color2){
    auto* data = new our::Color[size.x * size.y];
    int ptr = 0;
    for(int y = 0; y < size.y; y++){
        for(int x = 0; x < size.x; x++){
            data[ptr++] = ((x/patternSize.x)&1)^((y/patternSize.y)&1)?color1:color2;
        }
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    delete[] data;
}
