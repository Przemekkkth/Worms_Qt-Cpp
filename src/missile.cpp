#include "missile.h"
#include <QPoint>
#include <QList>
#include <QPolygon>
#include <QGraphicsPolygonItem>
#include "utils.h"
#include "gamescene.h"

Missile::Missile(float x, float y, float _vx, float _vy)
    : PhysicsObject(x, y)
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
    pItem->setPen(QPen(QColor(Qt::yellow)));
    pItem->setBrush(QBrush(QColor(Qt::yellow)));
    scene->addItem(pItem);
}

int Missile::BounceDeathAction()
{
    return 20; // Explode Big
}

bool Missile::Damege(float d)
{
    return true;
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
