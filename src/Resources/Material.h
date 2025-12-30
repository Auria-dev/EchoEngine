#pragma once

#include "Texture.h"

#include <memory>
#include <algorithm>

class Material
{

public:
    std::unique_ptr<Texture> DiffuseTexture;
    std::unique_ptr<Texture> NormalTexture;
    std::unique_ptr<Texture> ARMTexture;

    Material();
    ~Material();

    void SetDiffuse(Texture t);
    void SetNormal(Texture t);
    void SetARM(Texture t);

    void SetAO(Texture t);
    void SetRough(Texture t);
    void SetMetal(Texture t);

    void PackARM(Texture AO, Texture rough, Texture metal);

private:
    void BlitChannel(const Texture& src, int destChannelIdx, uchar defaultValue);
    
};

/*

Textures:
    Diffuse
    Normal
    ARM (AO/Rough/Metal) // have multiple constructors and pack the textures into one ARM.
    Opacity..?
    Displacement..?

*/