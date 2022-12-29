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

    //m_pixmap = PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Worm_Red);
}

void Worm::Draw(GameScene *scene, float fOffsetX, float fOffsetY, bool bPixel)
{
    if(bIsPlayable)
    {
        QGraphicsPixmapItem* pItem = new QGraphicsPixmapItem();
        if(fShootAngle >= -3.14/2 && fShootAngle <= 3.14/2)
            pItem->setPixmap(m_pixmap.scaled(8*SCREEN::CELL_SIZE.width(),
                                         8*SCREEN::CELL_SIZE.height()).transformed(QTransform().scale(-1,1)));
        else
        {
            pItem->setPixmap(m_pixmap.scaled(8*SCREEN::CELL_SIZE.width(),
                                         8*SCREEN::CELL_SIZE.height()));
        }
        QPoint p = QPoint(px - fOffsetX - radius, py - fOffsetY - radius);
        pItem->setPos(int(p.x()*SCREEN::CELL_SIZE.width()), int(p.y()*SCREEN::CELL_SIZE.height()));
        scene->addItem(pItem);

    }
    else
    {
        //Timbersone
    }
}

int Worm::BounceDeathAction()
{
    return 0;
}

bool Worm::Damege(float d)
{
    fHealth -= d;
    if (fHealth <= 0)
    { // Worm has died, no longer playable
        fHealth = 0.0f;
        bIsPlayable = false;
    }
    return fHealth > 0;
}

void Worm::setTeam(int nT)
{
    if(nT == 0)
    {
        m_pixmap = PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Worm_Green);
    }
    else if(nT == 1)
    {
        m_pixmap = PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Worm_Purple);
    }
    else if(nT == 2)
    {
        m_pixmap = PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Worm_Blue);
    }
    else if(nT == 3)
    {
        m_pixmap = PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Worm_Red);
    }
    nTeam = nT;
}

int Worm::team() const
{
    return nTeam;
}
