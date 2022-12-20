#include "dummy.h"
#include <cmath>

std::vector<std::pair<float, float>> DefineDummy();

Dummy::Dummy(float x, float y)
    : PhysicsObject(x, y)
{

}

Dummy::~Dummy()
{

}

void Dummy::Draw(GameScene *scene, float fOffsetX, float fOffsetY)
{

}

int Dummy::BounceDeathAction()
{
    return 0;
}

std::vector<std::pair<float, float>> DefineDummy()
{
    // Defines a circle with a line fom center to edge
    std::vector<std::pair<float, float>> vecModel;
    vecModel.push_back({ 0.0f, 0.0f });
    for (int i = 0; i < 10; i++)
        vecModel.push_back({ cosf(i / 9.0f * 2.0f * 3.14159f) , sinf(i / 9.0f * 2.0f * 3.14159f) });
    return vecModel;
}


std::vector<std::pair<float, float>> Dummy::vecModel = DefineDummy();
