#include "Renderer.h"

#include "Walnut/Random.h"

void Renderer::OnResize(uint32_t width, uint32_t height)
{

    if(m_FinalImage)
    {
        //Exit function when no resize is needed so delete doesnt get called
        if(m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
            return;
        //Personal function inside Walnut::Image | Will check if image needs resizing, if so -> release and reallocate memory
        //Better design than to delete image and create new one + the object pointer doesnt change, just the contents of the memory block
        //So everything that holds a reference to this object still has same image because we dont delete object and create new one that will
        //different spot in memory
        m_FinalImage->Resize(width, height);
    }
    else
    {
        //Create image if it doesnt exist only no need to recreate it when width/height dont match current image size
        //this gets handles in resize function
        m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
    }

    //Delete current image data if exists - its ok to call delete on nullptr(first render) - delete will check for u
    delete[] m_ImageData;
    //Reallocate for new data
    //uint32 is 32 bits -> RGBA 1 byte for each color = 32bits
    m_ImageData = new uint32_t[width * height];
}

void Renderer::Render()
{
    //Render every pixel of viewport
    //Fill image data
    for (uint32_t i = 0; i < m_FinalImage->GetWidth() * m_FinalImage->GetHeight(); i++) {
        m_ImageData[i] = Walnut::Random::UInt(); // 0xffff00ff : From l->r: ABGR, 1 byte each == 2 letters
        m_ImageData[i] |= 0xff000000; //Or equals to set bits of alpha channel to 1 (most significant) because
        //random uint will give us random alpha values aswell and we want to see image
    }

    //Upload to gpu for rendering - Inefficient to load data on cpu and transfer to GPU but will change later in series
    m_FinalImage->SetData(m_ImageData);
}