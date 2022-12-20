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
}
