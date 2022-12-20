#include "worm.h"
#include "pixmapmanager.h"
#include <QGraphicsPixmapItem>
#include "gamescene.h"

Worm::Worm(float x, float y)
    : PhysicsObject(x, y)
{
    radius = 3.5f;
    fFriction = 0.2f;
    bDead = false;
    nBounceBeforeDeath = -1;

    m_pixmap = PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Worms);
}

void Worm::Draw(GameScene *scene, float fOffsetX, float fOffsetY)
{
    QGraphicsPixmapItem* pItem = new QGraphicsPixmapItem();
    pItem->setPixmap(m_pixmap);
    pItem->setPos(px - fOffsetX, py - fOffsetY);
    scene->addItem(pItem);
}

int Worm::BounceDeathAction()
{
    return 0;
}
