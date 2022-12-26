#include "pixmapmanager.h"

PixmapManager* PixmapManager::ptr = nullptr;

PixmapManager *PixmapManager::Instance()
{
    if(!ptr)
    {
        ptr = new PixmapManager();
    }
    return ptr;
}


QPixmap& PixmapManager::getPixmap(TextureID id)
{
    return m_textures.get(id);
}

PixmapManager::PixmapManager()
{
    m_textures.load(TextureID::Worms, ":/res/worms.png");
    m_textures.load(TextureID::All, ":/res/all.png");

    std::unique_ptr<QPixmap> wormGreen(new QPixmap(getPixmap(TextureID::All).copy(0,0,62,65)));
    m_textures.insertResource(TextureID::Worm_Green, std::move(wormGreen));

    std::unique_ptr<QPixmap> wormPurple(new QPixmap(getPixmap(TextureID::All).copy(0,65,62,65)));
    m_textures.insertResource(TextureID::Worm_Purple, std::move(wormPurple));

    std::unique_ptr<QPixmap> wormBlue(new QPixmap(getPixmap(TextureID::All).copy(0,2*65,62,65)));
    m_textures.insertResource(TextureID::Worm_Blue, std::move(wormBlue));

    std::unique_ptr<QPixmap> wormRed(new QPixmap(getPixmap(TextureID::All).copy(0,3*65,62,65)));
    m_textures.insertResource(TextureID::Worm_Red, std::move(wormRed));

////////////////////////////TIMBERSTONES
    std::unique_ptr<QPixmap> timberstoneGreen(new QPixmap(getPixmap(TextureID::All).copy(62,0,62,65)));
    m_textures.insertResource(TextureID::Timberstone_Green, std::move(timberstoneGreen));

    std::unique_ptr<QPixmap> timberstonePurple(new QPixmap(getPixmap(TextureID::All).copy(62,65,62,65)));
    m_textures.insertResource(TextureID::Timberstone_Purple, std::move(timberstonePurple));

    std::unique_ptr<QPixmap> timberstoneBlue(new QPixmap(getPixmap(TextureID::All).copy(62,2*65,62,65)));
    m_textures.insertResource(TextureID::Timberstone_Blue, std::move(timberstoneBlue));

    std::unique_ptr<QPixmap> timberstoneRed(new QPixmap(getPixmap(TextureID::All).copy(62,3*65,62,65)));
    m_textures.insertResource(TextureID::Timberstone_Red, std::move(timberstoneRed));


}
