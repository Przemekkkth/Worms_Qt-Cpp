#include "gamescene.h"
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QGraphicsLineItem>
#include <QDebug>
#include "utils.h"
#include "pixmapmanager.h"
#include "debris.h"

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent), map(new unsigned char[nMapWidth * nMapHeight])
{
    //map = new unsigned char[nMapWidth * nMapHeight];
    memset(map, 0, nMapWidth*nMapHeight * sizeof(unsigned char));
    m_image = QImage(SCREEN::LOGICAL_SIZE, QImage::Format_RGB32);
    m_image.fill(QColor(Qt::yellow));
    CreateMap();

    boom(0,0,0);
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

void GameScene::setCamera()
{
    float fElapsedTime = 1.0f/m_loopSpeed;
    // Mouse Edge Map Scroll
    float fMapScrollSpeed = 400.0f;
    if (m_mousePosition.x() < 10) fCameraPosX -= fMapScrollSpeed * fElapsedTime;
    if (m_mousePosition.x() > SCREEN::PHYSICAL_SIZE.width() - 10) fCameraPosX += fMapScrollSpeed * fElapsedTime;
    if (m_mousePosition.y() < 10) fCameraPosY -= fMapScrollSpeed * fElapsedTime;
    if (m_mousePosition.y() > SCREEN::PHYSICAL_SIZE.height() - 10) fCameraPosY += fMapScrollSpeed * fElapsedTime;
    // Clamp map boundaries
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
    if (fCameraPosY >= nMapHeight - SCREEN::LOGICAL_SIZE.height())
    {
        fCameraPosY = nMapHeight - SCREEN::LOGICAL_SIZE.height();
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


        clear();
        m_image.fill(Qt::yellow);
        setCamera();
        updatePhysics();
        drawLandscape();
        // Draw Objects
        for (auto &p : listObjects)
        {
            p->Draw(this, fCameraPosX, fCameraPosY);
            //(m_mousePosition.x()/SCREEN::CELL_SIZE.width()) + fCameraPosX, (m_mousePosition.y()/SCREEN::CELL_SIZE.height()) + fCameraPosY
            //p->Draw()
        }

        resetStatus();
    }
}

void GameScene::boom(float fWorldX, float fWorldY, float fRadius)
{
    auto CircleBresenham = [&](int xc, int yc, int r)
    {
        // Taken from wikipedia
        int x = 0;
        int y = r;
        int p = 3 - 2 * r;
        if (!r) return;

        auto drawline = [&](int sx, int ex, int ny)
        {
            for (int i = sx; i < ex; i++)
                if (ny >= 0 && ny < nMapHeight && i >= 0 && i < nMapWidth)
                {
                    map[ny*nMapWidth + i] = 0;
                }
        };

        while (y >= x)
        {
            // Modified to draw scan-lines instead of edges
            drawline(xc - x, xc + x, yc - y);
            drawline(xc - y, xc + y, yc - x);
            drawline(xc - x, xc + x, yc + y);
            drawline(xc - y, xc + y, yc + x);
            if (p < 0) p += 4 * x++ + 6;
            else p += 4 * (x++ - y--) + 10;
        }
    };

    // Erase Terrain to form crater
    CircleBresenham(fWorldX, fWorldY, fRadius);

    // Shockwave other entities in range
    for (auto &p : listObjects)
    {
        // Work out distance between explosion origin and object
        float dx = p->px - fWorldX;
        float dy = p->py - fWorldY;
        float fDist = sqrt(dx*dx + dy*dy);
        if (fDist < 0.0001f) fDist = 0.0001f;

        // If within blast radius
        if (fDist < fRadius)
        {
            // Set velocity proportional and away from boom origin
            p->vx = (dx / fDist) * fRadius;
            p->vy = (dy / fDist) * fRadius;
            p->bStable = false;
        }
    }

    // Launch debris proportional to blast size
    for (int i = 0; i < (int)fRadius; i++)
        listObjects.push_back(std::unique_ptr<Debris>(new Debris(fWorldX, fWorldY)));
}

void GameScene::updatePhysics()
{
    // Do 10 physics iterations per frame - this allows smaller physics steps
    // giving rise to more accurate and controllable calculations
    float fElapsedTime = 1.0f/m_loopSpeed;
    for (int z = 0; z < 10; z++)
    {
        // Update physics of all physical objects
        for (auto &p : listObjects)
        {
            // Apply Gravity
            p->ay += 2.0f;

            // Update Velocity
            p->vx += p->ax * fElapsedTime;
            p->vy += p->ay * fElapsedTime;

            // Update Position
            float fPotentialX = p->px + p->vx * fElapsedTime;
            float fPotentialY = p->py + p->vy * fElapsedTime;

            // Reset Acceleration
            p->ax = 0.0f;
            p->ay = 0.0f;
            p->bStable = false;

            // Collision Check With Map
            float fAngle = atan2f(p->vy, p->vx);
            float fResponseX = 0;
            float fResponseY = 0;
            bool bCollision = false;

            // Iterate through semicircle of objects radius rotated to direction of travel
            for (float r = fAngle - 3.14159f / 2.0f; r < fAngle + 3.14159f / 2.0f; r += 3.14159f / 8.0f)
            {
                // Calculate test point on circumference of circle
                float fTestPosX = (p->radius) * cosf(r) + fPotentialX;
                float fTestPosY = (p->radius) * sinf(r) + fPotentialY;

                // Constrain to test within map boundary
                if (fTestPosX >= nMapWidth) fTestPosX = nMapWidth - 1;
                if (fTestPosY >= nMapHeight) fTestPosY = nMapHeight - 1;
                if (fTestPosX < 0) fTestPosX = 0;
                if (fTestPosY < 0) fTestPosY = 0;

                // Test if any points on semicircle intersect with terrain
                if (map[(int)fTestPosY * nMapWidth + (int)fTestPosX] != 0)
                {
                    // Accumulate collision points to give an escape response vector
                    // Effectively, normal to the areas of contact
                    fResponseX += fPotentialX - fTestPosX;
                    fResponseY += fPotentialY - fTestPosY;
                    bCollision = true;
                }
            }

            // Calculate magnitudes of response and velocity vectors
            float fMagVelocity = sqrtf(p->vx*p->vx + p->vy*p->vy);
            float fMagResponse = sqrtf(fResponseX*fResponseX + fResponseY*fResponseY);

            // Collision occurred
            if (bCollision)
            {
                // Force object to be stable, this stops the object penetrating the terrain
                p->bStable = true;

                // Calculate reflection vector of objects velocity vector, using response vector as normal
                float dot = p->vx * (fResponseX / fMagResponse) + p->vy * (fResponseY / fMagResponse);

                // Use friction coefficient to dampen response (approximating energy loss)
                p->vx = p->fFriction * (-2.0f * dot * (fResponseX / fMagResponse) + p->vx);
                p->vy = p->fFriction * (-2.0f * dot * (fResponseY / fMagResponse) + p->vy);

                //Some objects will "die" after several bounces
                if (p->nBounceBeforeDeath > 0)
                {
                    p->nBounceBeforeDeath--;
                    p->bDead = p->nBounceBeforeDeath == 0;

                    // If object died, work out what to do next
                    if (p->bDead)
                    {
                        // Action upon object death
                        // = 0 Nothing
                        // > 0 Explosion
                        int nResponse = p->BounceDeathAction();
                        if (nResponse > 0)
                            boom(p->px, p->py, nResponse);
                    }
                }

            }
            else
            {
                // No collision so update objects position
                p->px = fPotentialX;
                p->py = fPotentialY;
            }

            // Turn off movement when tiny
            if (fMagVelocity < 0.1f) p->bStable = true;
        }

        // Remove dead objects from the list, so they are not processed further. As the object
        // is a unique pointer, it will go out of scope too, deleting the object automatically. Nice :-)
        listObjects.remove_if([](std::unique_ptr<PhysicsObject> &o) {return o->bDead; });
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
                break;
            }
        }
    }

    QGraphicsPixmapItem* pItem = new QGraphicsPixmapItem();
    pItem->setPixmap(QPixmap::fromImage(m_image.scaled(SCREEN::PHYSICAL_SIZE)));
    pItem->setZValue(LAYER::BG);
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
        //qDebug() << "m_mouse->m_released " << m_mouse->m_released;
        boom((m_mousePosition.x()/SCREEN::CELL_SIZE.width()) + fCameraPosX, (m_mousePosition.y()/SCREEN::CELL_SIZE.height()) + fCameraPosY, 10.0f);
        //boom(100.0f, 800.0f, 100.0f);
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
