#include "gamescene.h"
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include "utils.h"
#include "pixmapmanager.h"

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent), map(new unsigned char[nMapWidth * nMapHeight])
{
    //map = new unsigned char[nMapWidth * nMapHeight];
    memset(map, 0, nMapWidth*nMapHeight * sizeof(unsigned char));
    m_image = QImage(SCREEN::LOGICAL_SIZE, QImage::Format_RGB32);
    m_image.fill(QColor(Qt::yellow));
    CreateMap();

    drawLandscape();
    for(int i = 0; i < 256; ++i)
    {
        m_keys[i] = new KeyStatus();
    }
    m_mouse = new MouseStatus();
    setSceneRect(0,0, SCREEN::PHYSICAL_SIZE.width(), SCREEN::PHYSICAL_SIZE.height());
    connect(&m_timer, &QTimer::timeout, this, &GameScene::loop);
    m_timer.start(int(1000.0f/FPS));
    m_elapsedTimer.start();
}

void GameScene::setMousePosition(QPoint newPos)
{
    if(m_mousePosition != newPos)
    {
        m_mousePosition = newPos;
    }
}

void GameScene::loop()
{
    m_deltaTime = m_elapsedTimer.elapsed();
    m_elapsedTimer.restart();

    m_loopTime += m_deltaTime;
    if( m_loopTime > m_loopSpeed)
    {
        m_loopTime -= m_loopSpeed;

        handlePlayerInput();

        float fElapsedTime = 1.0f/m_loopSpeed;
        clear();
        m_image.fill(Qt::yellow);
        // Mouse Edge Map Scroll
        float fMapScrollSpeed = 400.0f;
        if (m_mousePosition.x() < 5) fCameraPosX -= fMapScrollSpeed * fElapsedTime;
        if (m_mousePosition.x() > SCREEN::PHYSICAL_SIZE.width() - 5) fCameraPosX += fMapScrollSpeed * fElapsedTime;
        if (m_mousePosition.y() < 5) fCameraPosY -= fMapScrollSpeed * fElapsedTime;
        if (m_mousePosition.y() > SCREEN::PHYSICAL_SIZE.height() - 5) fCameraPosY += fMapScrollSpeed * fElapsedTime;

//        qDebug() << m_mousePosition;
////		// Clamp map boundaries
        if (fCameraPosX < 0)
        {
            fCameraPosX = 0;
        }
        if (fCameraPosX >= nMapWidth - SCREEN::LOGICAL_SIZE.width())
        {
            fCameraPosX = nMapWidth - SCREEN::LOGICAL_SIZE.width();
        }
        if (fCameraPosY < 0)
        {
            fCameraPosY = 0;
        }
        if (fCameraPosY >= nMapHeight - 2*SCREEN::LOGICAL_SIZE.height())
        {
            fCameraPosY = nMapHeight - 2*SCREEN::LOGICAL_SIZE.height();
        }
//        fCameraPosY = 400;
//        qDebug() << "fCameraX " << fCameraPosX << " fCameraY " << fCameraPosY;
        fCameraPosY = 150;
        drawLandscape();


        resetStatus();
    }
}

void GameScene::CreateMap()
{
    // Used 1D Perlin Noise
    float *fSurface = new float[nMapWidth];
    float *fNoiseSeed = new float[nMapWidth];

    // Populate with noise
    for (int i = 0; i < nMapWidth; i++)
        fNoiseSeed[i] = (float)rand() / (float)RAND_MAX;

    // Clamp noise to half way up screen
    fNoiseSeed[0] = 0.5f;

    // Generate 1D map
    PerlinNoise1D(nMapWidth, fNoiseSeed, 10, 2.0f, fSurface);

    // Fill 2D map based on adjacent 1D map
    for (int x = 0; x < nMapWidth; x++)
        for (int y = 0; y < nMapHeight; y++)
        {
            if (y >= fSurface[x] * nMapHeight)
            {
                map[y * nMapWidth + x] = 1;
                //qDebug() << "Jest 1 Create map";
            }
            else
            {
                map[y * nMapWidth + x] = 0;
            }
        }

    // Clean up!
    delete[] fSurface;
    delete[] fNoiseSeed;
}

void GameScene::PerlinNoise1D(int nCount, float *fSeed, int nOctaves, float fBias, float *fOutput)
{
    // Used 1D Perlin Noise
    for (int x = 0; x < nCount; x++)
    {
        float fNoise = 0.0f;
        float fScaleAcc = 0.0f;
        float fScale = 1.0f;

        for (int o = 0; o < nOctaves; o++)
        {
            int nPitch = nCount >> o;
            int nSample1 = (x / nPitch) * nPitch;
            int nSample2 = (nSample1 + nPitch) % nCount;
            float fBlend = (float)(x - nSample1) / (float)nPitch;
            float fSample = (1.0f - fBlend) * fSeed[nSample1] + fBlend * fSeed[nSample2];
            fScaleAcc += fScale;
            fNoise += fSample * fScale;
            fScale = fScale / fBias;
        }

        // Scale to seed range
        fOutput[x] = fNoise / fScaleAcc;
    }
}

void GameScene::drawLandscape()
{
    // Draw Landscape
    for (int x = 0; x < SCREEN::LOGICAL_SIZE.width(); x++)
    {
        for (int y = 0; y < SCREEN::LOGICAL_SIZE.height(); y++)
        {
            // Offset screen coordinates into world coordinates
            switch (map[(y + (int)fCameraPosY)*nMapWidth + (x + (int)fCameraPosX)])
            {
            case 0:
                m_image.setPixelColor(x, y, QColor(Qt::cyan));
                break;
            case 1:
                m_image.setPixelColor(x, y, QColor(Qt::darkGreen));
                //qDebug() << "Jest 1";
                break;
            }
        }
    }

    QGraphicsPixmapItem* pItem = new QGraphicsPixmapItem();
    pItem->setPixmap(QPixmap::fromImage(m_image.scaled(SCREEN::PHYSICAL_SIZE)));
    //pItem->setPixmap(QPixmap::fromImage(m_image));
    addItem(pItem);
}

QPoint GameScene::mousePosition() const
{
    return m_mousePosition;
}

void GameScene::handlePlayerInput()
{
    if(m_mouse->m_released)
    {
        qDebug() << "m_mouse->m_released " << m_mouse->m_released;
    }
}

void GameScene::resetStatus()
{
    for(int i = 0; i < 256; ++i)
    {
        m_keys[i]->m_released = false;
    }
    m_mouse->m_released = false;
}

void GameScene::keyPressEvent(QKeyEvent *event)
{
    if(KEYBOARD::KeysMapper.contains(event->key()))
    {
        m_keys[KEYBOARD::KeysMapper[event->key()]]->m_held = true;
    }
    QGraphicsScene::keyPressEvent(event);
}

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    if(KEYBOARD::KeysMapper.contains(event->key()))
    {
        m_keys[KEYBOARD::KeysMapper[event->key()]]->m_held = false;
        m_keys[KEYBOARD::KeysMapper[event->key()]]->m_released = true;
    }
    QGraphicsScene::keyReleaseEvent(event);
}

void GameScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_mouse->m_x = event->scenePos().x();
    m_mouse->m_y = event->scenePos().y();
    m_mouse->m_pressed = true;
    QGraphicsScene::mousePressEvent(event);
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    m_mouse->m_x = event->scenePos().x();
    m_mouse->m_y = event->scenePos().y();
    QGraphicsScene::mouseMoveEvent(event);
}

void GameScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_mouse->m_x = event->scenePos().x();
    m_mouse->m_y = event->scenePos().y();
    m_mouse->m_pressed = false;
    m_mouse->m_released = true;
    QGraphicsScene::mouseReleaseEvent(event);
}
