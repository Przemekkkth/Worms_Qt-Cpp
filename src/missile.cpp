#include "missile.h"

Missile::Missile(float x, float y, float _vx, float _vy)
{
    radius = 2.5f;
    fFriction = 0.5f;
    vx = _vx;
    vy = _vy;
    bDead = false;
    nBounceBeforeDeath = 1;
}

void Missile::Draw(GameScene *scene, float fOffsetX, float fOffsetY)
{

}

int Missile::BounceDeathAction()
{
    return 20; // Explode Big
}

std::vector<std::pair<float, float>> DefineMissile()
{
    // Defines a rocket like shape
    std::vector<std::pair<float, float>> vecModel;
    vecModel.push_back({ 0.0f, 0.0f });
    vecModel.push_back({ 1.0f, 1.0f });
    vecModel.push_back({ 2.0f, 1.0f });
    vecModel.push_back({ 2.5f, 0.0f });
    vecModel.push_back({ 2.0f, -1.0f });
    vecModel.push_back({ 1.0f, -1.0f });
    vecModel.push_back({ 0.0f, 0.0f });
    vecModel.push_back({ -1.0f, -1.0f });
    vecModel.push_back({ -2.5f, -1.0f });
    vecModel.push_back({ -2.0f, 0.0f });
    vecModel.push_back({ -2.5f, 1.0f });
    vecModel.push_back({ -1.0f, 1.0f });

    // Scale points to make shape unit sized
    for (auto &v : vecModel)
    {
        v.first /= 2.5f; v.second /= 2.5f;
    }
    return vecModel;
}

std::vector<std::pair<float, float>> Missile::vecModel = DefineMissile();
