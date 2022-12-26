#ifndef PIXMAPMANAGER_H
#define PIXMAPMANAGER_H
#include "resource_holder.h"
#include <QPixmap>

class PixmapManager
{
public:
    enum class TextureID{
        Worms, All, Worm_Green, Worm_Red, Worm_Purple, Worm_Blue,
        Timberstone_Green, Timberstone_Red, Timberstone_Purple, Timberstone_Blue
    };
    static PixmapManager* Instance();
    QPixmap& getPixmap(TextureID id);
private:
    PixmapManager();
    PixmapManager(PixmapManager& other) = delete;
    void operator=(const PixmapManager &) = delete;

    ResourceHolder<QPixmap, TextureID> m_textures;
    static PixmapManager* ptr;
};

#endif // PIXMAPMANAGER_H
