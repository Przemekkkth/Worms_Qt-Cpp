#include "worm.h"
#include "pixmapmanager.h"
#include <QGraphicsPixmapItem>
#include "gamescene.h"
#include "utils.h"

Worm::Worm(float x, float y)
    : PhysicsObject(x, y)
{
    radius = 3.3f;
    fFriction = 0.2f;
    bDead = false;
    nBounceBeforeDeath = -1;

    m_pixmap = PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Worms);
}

void Worm::Draw(GameScene *scene, float fOffsetX, float fOffsetY)
{
    //engine->DrawPartialSprite(px - fOffsetX - radius, py - fOffsetY - radius, sprWorm, 0, 0, 8, 8);
    QGraphicsPixmapItem* pItem = new QGraphicsPixmapItem();
    pItem->setPixmap(m_pixmap.scaled(8*SCREEN::CELL_SIZE.width(),
                                     8*SCREEN::CELL_SIZE.height()));
    QPoint p = QPoint(px - fOffsetX - radius, py - fOffsetY - radius);
    pItem->setPos(p.x()*SCREEN::CELL_SIZE.width(), p.y()*SCREEN::CELL_SIZE.height());
    scene->addItem(pItem);
}

int Worm::BounceDeathAction()
{
    return 0;
}
