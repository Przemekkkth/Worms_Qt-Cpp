#include "dummy.h"
#include "utils.h"
#include "gamescene.h"
#include <QColor>
#include <QPolygon>
#include <QGraphicsPolygonItem>
#include <cmath>
#include <QList>
#include <QPoint>

std::vector<std::pair<float, float>> DefineDummy();

Dummy::Dummy(float x, float y)
    : PhysicsObject(x, y)
{

}

Dummy::~Dummy()
{

}

void Dummy::Draw(GameScene *scene, float fOffsetX, float fOffsetY, bool bPixel)
{
    //engine->DrawWireFrameModel(vecModel, px - fOffsetX, py - fOffsetY, atan2f(vy, vx), radius, FG_WHITE);
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

    float factorScale = radius*SCREEN::CELL_SIZE.width()/2.0f;
    float pixelScale = 0.5f*SCREEN::CELL_SIZE.width()/2.0f;
    pItem->setScale(bPixel ? pixelScale : factorScale);
    pItem->setPen(QPen(QColor(Qt::white)));
    scene->addItem(pItem);
}

int Dummy::BounceDeathAction()
{
    return 0;
}

bool Dummy::Damage(float d)
{
    return true;
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
