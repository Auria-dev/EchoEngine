#include "Material.h"

Material::Material() { }
Material::~Material() { }

void Material::SetDiffuse(Texture t)
{
    DiffuseTexture = std::make_unique<Texture>(std::move(t));
}

void Material::SetNormal(Texture t)
{
    NormalTexture = std::make_unique<Texture>(std::move(t));
}

void Material::SetARM(Texture t)
{
    ARMTexture = std::make_unique<Texture>(std::move(t));
}

void Material::PackARM(Texture ao, Texture rough, Texture metal)
{
    int width = std::max({ ao.GetWidth(), rough.GetWidth(), metal.GetWidth() });
    int height = std::max({ ao.GetHeight(), rough.GetHeight(), metal.GetHeight() });

    if (ARMTexture)
    {
        width = std::max(width, ARMTexture->GetWidth());
        height = std::max(height, ARMTexture->GetHeight());
    }

    if (width == 0 || height == 0) return;

    bool isNew = false;
    if (!ARMTexture || ARMTexture->GetWidth() != width || ARMTexture->GetHeight() != height)
    {
        ARMTexture = std::make_unique<Texture>(width, height, 3);
        isNew = true;
    }

    uchar* dest = ARMTexture->GetData();
    int pixelCount = width * height;

    if (isNew)
    {
        for (int i = 0; i < pixelCount; ++i)
        { // set defaults
            dest[i * 3 + 0] = 255; // AO
            dest[i * 3 + 1] = 255; // Roughness
            dest[i * 3 + 2] = 0;   // Metal
        }
    }

    const uchar* aoData = ao.GetData();
    const uchar* roughData = rough.GetData();
    const uchar* metalData = metal.GetData();

    int aoStride = ao.GetChannels();
    int roughStride = rough.GetChannels();
    int metalStride = metal.GetChannels();

    int aoLimit = ao.GetWidth() * ao.GetHeight();
    int roughLimit = rough.GetWidth() * rough.GetHeight();
    int metalLimit = metal.GetWidth() * metal.GetHeight();

    for (int i = 0; i < pixelCount; ++i)
    {
        if (aoData && i < aoLimit) 
        {
            dest[i * 3 + 0] = aoData[i * aoStride];
        }

        if (roughData && i < roughLimit) 
        {
            dest[i * 3 + 1] = roughData[i * roughStride];
        }

        if (metalData && i < metalLimit) 
        {
            dest[i * 3 + 2] = metalData[i * metalStride];
        }
    }
}

void Material::SetAO(Texture t)
{
    BlitChannel(t, 0, 255); 
}

void Material::SetRough(Texture t)
{
    BlitChannel(t, 1, 255); 
}

void Material::SetMetal(Texture t)
{
    BlitChannel(t, 2, 0); 
}

void Material::SetAlphaMask(Texture mask)
{
    EnsureDiffuseRGBA();
    
    int w = std::min(DiffuseTexture->GetWidth(), mask.GetWidth());
    int h = std::min(DiffuseTexture->GetHeight(), mask.GetHeight());
    
    unsigned char* diffData = DiffuseTexture->GetData();
    const unsigned char* maskData = mask.GetData();
    int maskStride = mask.GetChannels();

    for (int i = 0; i < w * h; ++i)
    {
        unsigned char alphaVal = maskData[i * maskStride];
        
        diffData[i * 4 + 3] = alphaVal;
    }

    Translucent = true; 
}

void Material::EnsureDiffuseRGBA()
{
    if (!DiffuseTexture)
    {
        DiffuseTexture = std::make_unique<Texture>(1, 1, 4);
        unsigned char* d = DiffuseTexture->GetData();
        d[0] = 255; d[1] = 255; d[2] = 255; d[3] = 255;
        return;
    }

    if (DiffuseTexture->GetChannels() == 3)
    {
        int w = DiffuseTexture->GetWidth();
        int h = DiffuseTexture->GetHeight();
        uchar* newBuffer = (uchar*)malloc(w * h * 4);
        unsigned char* src = DiffuseTexture->GetData();
        
        for (int i = 0; i < w * h; ++i)
        {
            newBuffer[i * 4 + 0] = src[i * 3 + 0];
            newBuffer[i * 4 + 1] = src[i * 3 + 1];
            newBuffer[i * 4 + 2] = src[i * 3 + 2];
            newBuffer[i * 4 + 3] = 255;
        }
        
        *DiffuseTexture = Texture(w, h, 4, newBuffer);
    }
}

void Material::BlitChannel(const Texture& src, int destChannelIdx, uchar defaultValue)
{
    if (!ARMTexture || ARMTexture->GetWidth() != src.GetWidth() || ARMTexture->GetHeight() != src.GetHeight())
    {
        int w = src.GetWidth();
        int h = src.GetHeight();
        
        ARMTexture = std::make_unique<Texture>(w, h, 3);
        
        unsigned char* d = ARMTexture->GetData();
        for(int i=0; i<w*h; i++) {
            d[i*3 + 0] = 255; // AO
            d[i*3 + 1] = 255; // Rough
            d[i*3 + 2] = 0;   // Metal
        }
    }

    unsigned char* destData = ARMTexture->GetData();
    const unsigned char* srcData = src.GetData();
    int count = ARMTexture->GetWidth() * ARMTexture->GetHeight();
    int srcStride = src.GetChannels();

    for (int i = 0; i < count; ++i)
    {
        unsigned char val = srcData ? srcData[i * srcStride] : defaultValue;
        destData[i * 3 + destChannelIdx] = val;
    }
}