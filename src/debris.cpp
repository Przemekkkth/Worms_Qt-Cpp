#include "debris.h"
#include <cmath>

std::vector<std::pair<float, float>> DefineDebris();

Debris::Debris(float x, float y)
    : PhysicsObject(x, y)
{
    // Set velocity to random direction and size for "boom" effect
    vx = 10.0f * cosf(((float)rand() / (float)RAND_MAX) * 2.0f * 3.14159f);
    vy = 10.0f * sinf(((float)rand() / (float)RAND_MAX) * 2.0f * 3.14159f);
    radius = 1.0f;
    fFriction = 0.8f;
    nBounceBeforeDeath = 5; // After 5 bounces, dispose
}

Debris::~Debris()
{

}

void Debris::Draw(GameScene *scene, float fOffsetX, float fOffsetY)
{

}

int Debris::BounceDeathAction()
{
    return 0;
}

std::vector<std::pair<float, float>> DefineDebris()
{
    // A small unit rectangle
    std::vector<std::pair<float, float>> vecModel;
    vecModel.push_back({ 0.0f, 0.0f });
    vecModel.push_back({ 1.0f, 0.0f });
    vecModel.push_back({ 1.0f, 1.0f });
    vecModel.push_back({ 0.0f, 1.0f });
    return vecModel;
}
std::vector<std::pair<float, float>> Debris::vecModel = DefineDebris();
