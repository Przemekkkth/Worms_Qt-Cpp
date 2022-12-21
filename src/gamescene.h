#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QElapsedTimer>
#include <QTimer>
#include <list>
#include <memory>
#include "physics_object.h"
#include <QImage>

struct KeyStatus
{
    bool m_held = false;
    bool m_released = false;
};

struct MouseStatus
{
    float m_x = 0.0f;
    float m_y = 0.0f;
    bool m_released = false;
    bool m_pressed = false;
};


class GameScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GameScene(QObject *parent = nullptr);
    void setMousePosition(QPoint newPos);
signals:

private slots:
    void loop();

private:
    void CreateMap();
    void PerlinNoise1D(int nCount, float *fSeed, int nOctaves, float fBias, float *fOutput);
    void drawLandscape();
    QPoint mousePosition() const;    
    void boom(float fWorldX, float fWorldY, float fRadius);
    //Terrain size
    int nMapWidth  = 1024;
    int nMapHeight = 512;
    unsigned char *map = nullptr;
    // Camera coordinates
    float fCameraPosX = 0.0f;
    float fCameraPosY = 0.0f;
    //List of things that exist in game world
    std::list<std::unique_ptr<PhysicsObject>> listObjects;
    QImage m_image;
    QPoint m_mousePosition;

    void handlePlayerInput();
    void resetStatus();
    KeyStatus* m_keys[256];
    MouseStatus* m_mouse;

    const int FPS = 60;
    QTimer m_timer;
    QElapsedTimer m_elapsedTimer;
    float m_deltaTime = 0.0f, m_loopTime = 0.0f;
    const float m_loopSpeed = int(1000.0f/FPS);
    // QGraphicsScene interface
    void setCamera();
    
protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // GAMESCENE_H
