#ifndef WORM_H
#define WORM_H
#include "physics_object.h"
#include <QPixmap>

class Worm : public PhysicsObject
{
public:
    Worm(float x, float y);
    virtual void Draw(GameScene *scene, float fOffsetX, float fOffsetY, bool bPixel = false) override;
    virtual int BounceDeathAction() override;
    virtual bool Damege(float d) override;

    float fShootAngle = 0.0f;
    float fHealth = 1.0f;
    bool bIsPlayable = true;
    void setTeam(int nT);
    int team() const;
private:
    int nTeam = 0;	// ID of which team this worm belongs to
    QPixmap m_pixmap;
};

#endif // WORM_H
