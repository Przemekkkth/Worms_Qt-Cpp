#include "debris.h"
#include "utils.h"
#include "gamescene.h"
#include <cmath>
#include <QPolygon>
#include <QGraphicsPolygonItem>
#include <QPen>

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
    //engine->DrawWireFrameModel(vecModel, px - fOffsetX, py - fOffsetY, atan2f(vy, vx), radius, FG_DARK_GREEN);
    //QPolygon polygon;
    QList<QPoint> points;
    for(auto val : vecModel)
    {
        QPoint p = QPoint(val.first*SCREEN::CELL_SIZE.width(),
                          val.second*SCREEN::CELL_SIZE.height());
        points.append(p);
    }
    QPolygon polygon = QPolygon(points);
    QGraphicsPolygonItem *pItem = new QGraphicsPolygonItem;
    pItem->setPolygon(polygon);
    QPoint p = QPoint(px-fOffsetX, py-fOffsetY);
    pItem->setPos(p.x()*SCREEN::CELL_SIZE.width(), p.y()*SCREEN::CELL_SIZE.height());
    //pItem->setPos(px, py);
    pItem->setRotation(std::atan2(vy, vx)* (180.0f / 3.14159f));

    pItem->setScale(radius*SCREEN::CELL_SIZE.width()/2.0f);
    pItem->setPen(QPen(QColor(Qt::darkGreen)));
    pItem->setBrush(QBrush(QColor(Qt::darkGreen)));
    scene->addItem(pItem);
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
