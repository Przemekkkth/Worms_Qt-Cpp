#ifndef PHYSICS_OBJECT_H
#define PHYSICS_OBJECT_H


class GameScene;
class PhysicsObject
{
public:
    PhysicsObject(float x = 0.0f, float y = 0.0f);
    virtual ~PhysicsObject();

    //position
    float px = 0.0f;
    float py = 0.0f;
    // velocity
    float vx = 0.0f;
    float vy = 0.0f;
    // acelaration
    float ax = 0.0f;
    float ay = 0.0f;

    // bounding circle for collision
    float radius = 4.0f;
    // has object stopped moving
    bool bStable = false;
    // acctualy a dampening factor is a more accurate name
    float fFriction = 0.8f;
    // how many time object can bounce before death
    // - 1 = infinite
    int nBounceBeforeDeath = -1;
    // Flag to indicate object should be removed
    bool bDead = false;

    virtual void Draw(GameScene* scene, float fOffsetX, float fOffsetY, bool bPixel = false) = 0;
    virtual int BounceDeathAction() = 0;
    virtual bool Damage(float d) = 0;
};

#endif // PHYSICS_OBJECT_H
