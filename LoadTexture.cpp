#include "LoadTexture.h"
#include <il.h>
#include <string>

bool LoadTexture::LoadTextureFromFile(std::wstring path) {
    bool textureLoaded = false;

    //Generate and set current image ID
    ILuint imgID = 0;
    ilGenImages(1, &imgID);
    ilBindImage(imgID);

    ILboolean success = ilLoadImage(path.c_str());

    //Image loaded successfully
    if (success == IL_TRUE)
    {
        //Convert image to RGBA
        success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
        if (success == IL_TRUE)
        {
            //Create texture from file pixels
            //textureLoaded = loadTextureFromPixels32((GLuint*)ilGetData(), (GLuint)ilGetInteger(IL_IMAGE_WIDTH), (GLuint)ilGetInteger(IL_IMAGE_HEIGHT));
        }

        //Delete file from memory
        ilDeleteImages(1, &imgID);
    }

    return false;
}
