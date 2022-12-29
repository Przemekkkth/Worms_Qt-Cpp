#include "gamescene.h"
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QGraphicsLineItem>
#include <QDebug>
#include "utils.h"
#include "pixmapmanager.h"
#include "debris.h"
#include "dummy.h"
#include "missile.h"
#include "worm.h"

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent), map(new char[nMapWidth * nMapHeight])
{
    //map = new unsigned char[nMapWidth * nMapHeight];
    memset(map, 0, nMapWidth*nMapHeight * sizeof(unsigned char));

    nGameState = GS_RESET;
    nNextState = GS_RESET;
    nAIState = AI_ASSESS_ENVIRONMENT;
    nAINextState = AI_ASSESS_ENVIRONMENT;

    bGameIsStable = false;

    m_image = QImage(SCREEN::LOGICAL_SIZE, QImage::Format_RGB32);
    m_image.fill(QColor(Qt::black));


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
    if(m_keys[KEYBOARD::KEY_T]->m_released)
    {
        bZoomOut = !bZoomOut;
    }

    // Mouse Edge Map Scroll
    float fMapScrollSpeed = 400.0f;
    if (m_mousePosition.x() < 10) fCameraPosX -= fMapScrollSpeed * fElapsedTime;
    if (m_mousePosition.x() > SCREEN::PHYSICAL_SIZE.width() - 10) fCameraPosX += fMapScrollSpeed * fElapsedTime;
    if (m_mousePosition.y() < 10) fCameraPosY -= fMapScrollSpeed * fElapsedTime;
    if (m_mousePosition.y() > SCREEN::PHYSICAL_SIZE.height() - 10) fCameraPosY += fMapScrollSpeed * fElapsedTime;

    // Control Supervisor
    switch (nGameState)
    {
    case GS_RESET: // Set game variables to know state
    {
        bEnablePlayerControl = false;
        bGameIsStable = false;
        bPlayerHasFired = false;
        bShowCountDown = false;
        nNextState = GS_GENERATE_TERRAIN;
    }
        break;

    case GS_GENERATE_TERRAIN: // Create a new terrain
    {
        bZoomOut = true;
        CreateMap();
        bGameIsStable = false;
        bShowCountDown = false;
        nNextState = GS_GENERATING_TERRAIN;
    }
        break;

    case GS_GENERATING_TERRAIN: // Does nothing, for now ;)
    {
        bShowCountDown = false;
        if (bGameIsStable)
        {
            nNextState = GS_ALLOCATE_UNITS;
        }
    }
        break;

    case GS_ALLOCATE_UNITS: // Add a unit to the top of the screen
    {
        // Deploy teams
        int nTeams = 4;
        int nWormsPerTeam = 4;

        // Calculate spacing of worms and teams
        float fSpacePerTeam = (float)nMapWidth / (float)nTeams;
        float fSpacePerWorm = fSpacePerTeam / (nWormsPerTeam * 2.0f);

        // Create teams
        for (int t = 0; t < nTeams; t++)
        {
            vecTeams.emplace_back(Team());
            float fTeamMiddle = (fSpacePerTeam / 2.0f) + (t * fSpacePerTeam);
            for (int w = 0; w < nWormsPerTeam; w++)
            {
                float fWormX = fTeamMiddle - ((fSpacePerWorm * (float)nWormsPerTeam) / 2.0f) + w * fSpacePerWorm;
                float fWormY = 0.0f;

                // Add worms to teams
                Worm *worm = new Worm(fWormX,fWormY);
                worm->setTeam(t);
                listObjects.push_back(std::unique_ptr<Worm>(worm));
                vecTeams[t].vecMembers.push_back(worm);
                vecTeams[t].nTeamSize = nWormsPerTeam;
            }

            vecTeams[t].nCurrentMember = 0;
        }

        // Select players first worm for control and camera tracking
        pObjectUnderControl = vecTeams[0].vecMembers[vecTeams[0].nCurrentMember];
        pCameraTrackingObject = pObjectUnderControl;
        bShowCountDown = false;
        nNextState = GS_ALLOCATING_UNITS;
    }
        break;

    case GS_ALLOCATING_UNITS: // Stay in this state whilst units are deploying
    {
        if (bGameIsStable)
        {
            bEnablePlayerControl = true;
            bEnableComputerControl = false;
            fTurnTime = 15.0f;
            bZoomOut = false;
            nNextState = GS_START_PLAY;
        }
    }
        break;

    case GS_START_PLAY: // Player is in control of unit
    {
        bShowCountDown = true;

        // If player has discharged weapon, or turn time is up, move on to next state
        if (bPlayerHasFired || fTurnTime <= 0.0f)
        {
            nNextState = GS_CAMERA_MODE;
        }
    }
        break;

    case GS_CAMERA_MODE: // Camera is tracking on-screen action
    {
        bEnableComputerControl = false;
        bEnablePlayerControl = false;
        bPlayerHasFired = false;
        bShowCountDown = false;
        fEnergyLevel = 0.0f;

        if (bGameIsStable) // Once settled, choose next worm
        {
            // Get Next Team, if there is no next team, game is over
            int nOldTeam = nCurrentTeam;
            do {
                nCurrentTeam++;
                nCurrentTeam %= vecTeams.size();
            } while (!vecTeams[nCurrentTeam].IsTeamAlive());

            // Lock controls if AI team is currently playing
            if (nCurrentTeam == 0) // Player Team
            {
                bEnablePlayerControl = true;	// Swap these around for complete AI battle
                bEnableComputerControl = false;
            }
            else // AI Team
            {
                bEnablePlayerControl = false;
                bEnableComputerControl = true;
            }

            // Set control and camera
            pObjectUnderControl = vecTeams[nCurrentTeam].GetNextMember();
            pCameraTrackingObject = pObjectUnderControl;
            fTurnTime = 15.0f;
            bZoomOut = false;
            nNextState = GS_START_PLAY;

            // If no different team could be found...
            if (nCurrentTeam == nOldTeam)
            {
                // ...Game is over, Current Team have won!
                nNextState = GS_GAME_OVER1;
            }
        }

    }
        break;
    case GS_GAME_OVER1: // Zoom out and launch loads of missiles!
    {
        bEnableComputerControl = false;
        bEnablePlayerControl = false;
        bZoomOut = true;
        bShowCountDown = false;

        for (int i = 0; i < 100; i ++)
        {
            int nBombX = rand() % nMapWidth;
            int nBombY = rand() % (nMapHeight / 2);
            listObjects.push_back(std::unique_ptr<Missile>(new Missile(nBombX, nBombY, 0.0f, 0.5f)));
        }

        nNextState = GS_GAME_OVER2;
    }
    break;

    case GS_GAME_OVER2: // Stay here and wait for chaos to settle
    {
        bEnableComputerControl = false;
        bEnablePlayerControl = false;
        // No exit from this state!
    }
    break;
    }

    // AI State Machine
    if (bEnableComputerControl)
    {
        switch (nAIState)
        {
        case AI_ASSESS_ENVIRONMENT:
        {

            int nAction = rand() % 3;
            if (nAction == 0) // Play Defensive - move away from team
            {
                // Find nearest ally, walk away from them
                float fNearestAllyDistance = INFINITY; float fDirection = 0;
                Worm *origin = (Worm*)pObjectUnderControl;

                for (auto w : vecTeams[nCurrentTeam].vecMembers)
                {
                    if (w != pObjectUnderControl)
                    {
                        if (fabs(w->px - origin->px) < fNearestAllyDistance)
                        {
                            fNearestAllyDistance = fabs(w->px - origin->px);
                            fDirection = (w->px - origin->px) < 0.0f ? 1.0f : -1.0f;
                        }
                    }
                }

                if (fNearestAllyDistance < 50.0f)
                    fAISafePosition = origin->px + fDirection * 80.0f;
                else
                    fAISafePosition = origin->px;
            }

            if (nAction == 1) // Play Ballsy - move towards middle
            {
                Worm *origin = (Worm*)pObjectUnderControl;
                float fDirection = ((float)(nMapWidth / 2.0f) - origin->px) < 0.0f ? -1.0f : 1.0f;
                fAISafePosition = origin->px + fDirection * 200.0f;
            }

            if (nAction == 2) // Play Dumb - don't move
            {
                Worm *origin = (Worm*)pObjectUnderControl;
                fAISafePosition = origin->px;
            }

            // Clamp so dont walk off map
            if (fAISafePosition <= 20.0f) fAISafePosition = 20.0f;
            if (fAISafePosition >= nMapWidth - 20.0f) fAISafePosition = nMapWidth - 20.0f;
            nAINextState = AI_MOVE;
        }
        break;

        case AI_MOVE:
        {
            Worm *origin = (Worm*)pObjectUnderControl;
            if (fTurnTime >= 8.0f && origin->px != fAISafePosition)
            {
                // Walk towards target until it is in range
                if (fAISafePosition < origin->px && bGameIsStable)
                {
                    origin->fShootAngle = -3.14159f * 0.6f;
                    bAI_Jump = true;
                    nAINextState = AI_MOVE;
                }

                if (fAISafePosition > origin->px && bGameIsStable)
                {
                    origin->fShootAngle = -3.14159f * 0.4f;
                    bAI_Jump = true;
                    nAINextState = AI_MOVE;
                }
            }
            else
                nAINextState = AI_CHOOSE_TARGET;
        }
        break;

        case AI_CHOOSE_TARGET: // Worm finished moving, choose target
        {
            bAI_Jump = false;

            // Select Team that is not itself
            Worm *origin = (Worm*)pObjectUnderControl;
            int nCurrentTeam = origin->team();
            int nTargetTeam = 0;
            do {
                nTargetTeam = rand() % vecTeams.size();
            } while (nTargetTeam == nCurrentTeam || !vecTeams[nTargetTeam].IsTeamAlive());

            // Aggressive strategy is to aim for opponent unit with most health
            Worm *mostHealthyWorm = vecTeams[nTargetTeam].vecMembers[0];
            for (auto w : vecTeams[nTargetTeam].vecMembers)
                if (w->fHealth > mostHealthyWorm->fHealth)
                    mostHealthyWorm = w;

            pAITargetWorm = mostHealthyWorm;
            fAITargetX = mostHealthyWorm->px;
            fAITargetY = mostHealthyWorm->py;
            nAINextState = AI_POSITION_FOR_TARGET;
        }
        break;

        case AI_POSITION_FOR_TARGET: // Calculate trajectory for target, if the worm needs to move, do so
        {
            Worm *origin = (Worm*)pObjectUnderControl;
            float dy = -(fAITargetY - origin->py);
            float dx = -(fAITargetX - origin->px);
            float fSpeed = 30.0f;
            float fGravity = 2.0f;

            bAI_Jump = false;

            float a = fSpeed * fSpeed*fSpeed*fSpeed - fGravity * (fGravity * dx * dx + 2.0f * dy * fSpeed * fSpeed);

            if (a < 0) // Target is out of range
            {
                if (fTurnTime >= 5.0f)
                {
                    // Walk towards target until it is in range
                    if (pAITargetWorm->px < origin->px && bGameIsStable)
                    {
                        origin->fShootAngle = -3.14159f * 0.6f;
                        bAI_Jump = true;
                        nAINextState = AI_POSITION_FOR_TARGET;
                    }

                    if (pAITargetWorm->px > origin->px && bGameIsStable)
                    {
                        origin->fShootAngle = -3.14159f * 0.4f;
                        bAI_Jump = true;
                        nAINextState = AI_POSITION_FOR_TARGET;
                    }
                }
                else
                {
                    // Worm is stuck, so just fire in direction of enemy!
                    // Its dangerous to self, but may clear a blockage
                    fAITargetAngle = origin->fShootAngle;
                    fAITargetEnergy = 0.75f;
                    nAINextState = AI_AIM;
                }
            }
            else
            {
                // Worm is close enough, calculate trajectory
                float b1 = fSpeed * fSpeed + sqrtf(a);
                float b2 = fSpeed * fSpeed - sqrtf(a);

                float fTheta1 = atanf(b1 / (fGravity * dx)); // Max Height
                float fTheta2 = atanf(b2 / (fGravity * dx)); // Min Height

                // We'll use max as its a greater chance of avoiding obstacles
                fAITargetAngle = fTheta1 - (dx > 0 ? 3.14159f : 0.0f);
                float fFireX = cosf(fAITargetAngle);
                float fFireY = sinf(fAITargetAngle);

                // AI is clamped to 3/4 power
                fAITargetEnergy = 0.75f;
                nAINextState = AI_AIM;
            }
        }
        break;

        case AI_AIM: // Line up aim cursor
        {
            Worm *worm = (Worm*)pObjectUnderControl;

            bAI_AimLeft = false;
            bAI_AimRight = false;
            bAI_Jump = false;

            if (worm->fShootAngle < fAITargetAngle)
                bAI_AimRight = true;
            else
                bAI_AimLeft = true;

            // Once cursors are aligned, fire - some noise could be
            // included here to give the AI a varying accuracy, and the
            // magnitude of the noise could be linked to game difficulty
            if (fabs(worm->fShootAngle - fAITargetAngle) <= 0.1f)
            {
                bAI_AimLeft = false;
                bAI_AimRight = false;
                fEnergyLevel = 0.0f;
                nAINextState = AI_FIRE;
            }
            else
                nAINextState = AI_AIM;
        }
        break;

        case AI_FIRE:
        {
            bAI_Energise = true;
            bFireWeapon = false;
            bEnergising = true;

            if (fEnergyLevel >= fAITargetEnergy)
            {
                bFireWeapon = true;
                bAI_Energise = false;
                bEnergising = false;
                bEnableComputerControl = false;
                nAINextState = AI_ASSESS_ENVIRONMENT;
            }
        }
        break;

        }
    }

    // Decrease Turn Time
    fTurnTime -= fElapsedTime;

    if (pObjectUnderControl != nullptr)
    {
        pObjectUnderControl->ax = 0.0f;

        if (pObjectUnderControl->bStable)
        {
            if ((bEnablePlayerControl && m_keys[KEYBOARD::KEY_Z]->m_pressed) || (bEnableComputerControl && bAI_Jump))
            {
                float a = ((Worm*)pObjectUnderControl)->fShootAngle;

                pObjectUnderControl->vx = 4.0f * cosf(a);
                pObjectUnderControl->vy = 8.0f * sinf(a);
                pObjectUnderControl->bStable = false;

                bAI_Jump = false;
            }

            if ((bEnablePlayerControl && m_keys[KEYBOARD::KEY_S]->m_held) || (bEnableComputerControl && bAI_AimRight))
            {
                Worm* worm = (Worm*)pObjectUnderControl;
                worm->fShootAngle += 1.0f * fElapsedTime;
                if (worm->fShootAngle > 3.14159f) worm->fShootAngle -= 3.14159f * 2.0f;
            }

            if ((bEnablePlayerControl && m_keys[KEYBOARD::KEY_A]->m_held) || (bEnableComputerControl && bAI_AimLeft))
            {
                Worm* worm = (Worm*)pObjectUnderControl;
                worm->fShootAngle -= 1.0f * fElapsedTime;
                if (worm->fShootAngle < -3.14159f) worm->fShootAngle += 3.14159f * 2.0f;
            }

            if ((bEnablePlayerControl && m_keys[KEYBOARD::KEY_SPACE]->m_pressed))
            {
                bFireWeapon = false;
                bEnergising = true;
                fEnergyLevel = 0.0f;
            }

            if ((bEnablePlayerControl && m_keys[KEYBOARD::KEY_SPACE]->m_held) || (bEnableComputerControl && bAI_Energise))
            {
                if (bEnergising)
                {
                    fEnergyLevel += 0.75f * fElapsedTime;
                    if (fEnergyLevel >= 1.0f)
                    {
                        fEnergyLevel = 1.0f;
                        bFireWeapon = true;
                    }
                }
            }

            if ((bEnablePlayerControl && m_keys[KEYBOARD::KEY_SPACE]->m_released))
            {
                if (bEnergising)
                {
                    bFireWeapon = true;
                }

                bEnergising = false;
            }
        }

        if (pCameraTrackingObject != nullptr)
        {
            fCameraPosXTarget = pCameraTrackingObject->px - SCREEN::LOGICAL_SIZE.width()/ 2;
            fCameraPosYTarget = pCameraTrackingObject->py - SCREEN::LOGICAL_SIZE.height() / 2;
            fCameraPosX += (fCameraPosXTarget - fCameraPosX) * 15.0f * fElapsedTime;
            fCameraPosY += (fCameraPosYTarget - fCameraPosY) * 15.0f * fElapsedTime;
        }

        if (bFireWeapon)
        {
            Worm* worm = (Worm*)pObjectUnderControl;

            // Get Weapon Origin
            float ox = worm->px;
            float oy = worm->py;

            // Get Weapon Direction
            float dx = cosf(worm->fShootAngle);
            float dy = sinf(worm->fShootAngle);

            // Create Weapon Object
            Missile *m = new Missile(ox, oy, dx * 40.0f * fEnergyLevel, dy * 40.0f * fEnergyLevel);
            pCameraTrackingObject = m;
            listObjects.push_back(std::unique_ptr<Missile>(m));

            // Reset flags involved with firing weapon
            bFireWeapon = false;
            fEnergyLevel = 0.0f;
            bEnergising = false;
            bPlayerHasFired = true;

            if (rand() % 100 >= 50)
                bZoomOut = true;
        }
    }


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

        // Check For game state stability
        bGameIsStable = true;
        for (auto &p : listObjects)
            if (!p->bStable)
            {
                bGameIsStable = false;
                break;
            }

        // DEBUG Feature: Indicate Game Stability
        // Draw Team Health Bars
        for (size_t t = 0; t < vecTeams.size(); t++)
        {
            float fTotalHealth = 0.0f;
            float fMaxHealth = (float)vecTeams[t].nTeamSize;
            for (auto w : vecTeams[t].vecMembers) // Accumulate team health
                fTotalHealth += w->fHealth;

            QColor cols[] = { Qt::green, QColor(230,230,250), Qt::blue, Qt::red };
            QGraphicsRectItem* rItem = new QGraphicsRectItem();
            rItem->setPos(4*SCREEN::CELL_SIZE.width(), (4 + t * 4)*SCREEN::CELL_SIZE.height());
            rItem->setRect(0,0,
             ((fTotalHealth / fMaxHealth) * (float)(SCREEN::LOGICAL_SIZE.width() - 8) + 1)*SCREEN::CELL_SIZE.width(),
                           10);
            rItem->setPen(cols[t]);
            rItem->setBrush(cols[t]);
            addItem(rItem);
        }
        if (bShowCountDown)
        {
            QGraphicsSimpleTextItem *tItem = new QGraphicsSimpleTextItem();
            tItem->setText(QString::number(fTurnTime, 'g', 2));
            QFont font = tItem->font();
            font.setPixelSize(25);
            tItem->setFont(font);
            tItem->setPos(15, 75);
            tItem->setPen(QColor(Qt::white));
            tItem->setBrush(QColor(Qt::white));
            addItem(tItem);
        }
        // Update State Machine
        nGameState = nNextState;
        nAIState = nAINextState;

        resetStatus();
    }
}

void GameScene::boom(float fWorldX, float fWorldY, float fRadius)
{
    auto CircleBresenham = [&](int xc, int yc, int r)
    {
        int x = 0;
        int y = r;
        int p = 3 - 2 * r;
        if (!r) return;

        auto drawline = [&](int sx, int ex, int ny)
        {
            for (int i = sx; i < ex; i++)
                if(ny >=0 && ny < nMapHeight && i >=0 && i < nMapWidth)
                    map[ny*nMapWidth + i] = 0;
        };

        while (y >= x) // only formulate 1/8 of circle
        {
            //Filled Circle
            drawline(xc - x, xc + x, yc - y);
            drawline(xc - y, xc + y, yc - x);
            drawline(xc - x, xc + x, yc + y);
            drawline(xc - y, xc + y, yc + x);
            if (p < 0) p += 4 * x++ + 6;
            else p += 4 * (x++ - y--) + 10;
        }
    };

    int bx = (int)fWorldX;
    int by = (int)fWorldY;

    // Erase Terrain to form crater
    CircleBresenham(fWorldX, fWorldY, fRadius);

    // Shockwave other entities in range
    for (auto &p : listObjects)
    {
        float dx = p->px - fWorldX;
        float dy = p->py - fWorldY;
        float fDist = sqrt(dx*dx + dy*dy);
        if (fDist < 0.0001f) fDist = 0.0001f;
        if (fDist < fRadius)
        {
            p->vx = (dx / fDist) * fRadius;
            p->vy = (dy / fDist) * fRadius;
            p->Damege(((fRadius - fDist) / fRadius) * 0.8f); // Corrected ;)
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
    float fElapsedTime = (1.0f/m_loopSpeed);
    for (int z = 0; z < 5; z++)
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
                if (map[(int)fTestPosY * nMapWidth + (int)fTestPosX] > 0)
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
                        {
                            boom(p->px, p->py, nResponse);
                            // Dead objects can no lobger be tracked by the camera
                            pCameraTrackingObject = nullptr;
                        }
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
    PerlinNoise1D(nMapWidth, fNoiseSeed, 8, 2.0f, fSurface);

    // Fill 2D map based on adjacent 1D map
    for (int x = 0; x < nMapWidth; x++)
        for (int y = 0; y < nMapHeight; y++)
        {
            if (y >= fSurface[x] * nMapHeight)
            {
                map[y * nMapWidth + x] = 1;
            }
            else
            {
                // Shade the sky according to altitude - we only do top 1/3 of map
                // as the Boom() function will just paint in 0 (cyan)
                if ((float)y < (float)nMapHeight / 3.0f)
                    map[y * nMapWidth + x] = (-8.0f * ((float)y / (nMapHeight / 3.0f))) -1.0f;
                else
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
    if(!bZoomOut)
    {
        //50 60 118
        //62 78 137
        //74 97 146
        //92 130 164
        //117 164 191
        //134 183 200
        //160 210 228
        //197 240 255

        for (int x = 0; x < SCREEN::LOGICAL_SIZE.width(); x++)
        {
            for (int y = 0; y < SCREEN::LOGICAL_SIZE.height(); y++)
            {
                // Offset screen coordinates into world coordinates
                switch (map[(y + (int)fCameraPosY)*nMapWidth + (x + (int)fCameraPosX)])
                {
                case -1:m_image.setPixelColor(x, y, QColor(50, 60, 118)); break;
                case -2:m_image.setPixelColor(x, y, QColor(62, 78, 137)); break;
                case -3:m_image.setPixelColor(x, y, QColor(74, 97, 146)); break;
                case -4:m_image.setPixelColor(x, y, QColor(92, 130, 164)); break;
                case -5:m_image.setPixelColor(x, y, QColor(117, 164, 191)); break;
                case -6:m_image.setPixelColor(x, y, QColor(134, 183, 200)); break;
                case -7:m_image.setPixelColor(x, y, QColor(160, 210, 228)); break;
                case -8:m_image.setPixelColor(x, y, QColor(197, 240, 255)); break;
                case 0: m_image.setPixelColor(x, y, QColor(Qt::cyan));  break;
                case 1:	m_image.setPixelColor(x, y, QColor(Qt::darkGreen)); break;

                }
            }
        }

        QGraphicsPixmapItem* pItem = new QGraphicsPixmapItem();
        pItem->setPixmap(QPixmap::fromImage(m_image.scaled(SCREEN::PHYSICAL_SIZE)));
        pItem->setZValue(LAYER::BG);
        addItem(pItem);
        for (auto &p : listObjects)
        {
            p->Draw(this, fCameraPosX, fCameraPosY);
            Worm* worm = (Worm*)pObjectUnderControl;

            if (p.get() == worm)
            {
                float cx = worm->px + 8.0f * cosf(worm->fShootAngle) - fCameraPosX;
                float cy = worm->py + 8.0f * sinf(worm->fShootAngle) - fCameraPosY;

                // Draw "+" symbol
                QGraphicsRectItem* rItem0 = new QGraphicsRectItem();
                rItem0->setRect(0,0, SCREEN::CELL_SIZE.width(),SCREEN::CELL_SIZE.height());
                rItem0->setPen(QPen(QColor(Qt::black)));
                rItem0->setBrush(QBrush(QColor(Qt::black)));
                rItem0->setPos(cx*SCREEN::CELL_SIZE.width(), cy*SCREEN::CELL_SIZE.height());
                addItem(rItem0);

                QGraphicsRectItem* rItem1 = new QGraphicsRectItem();
                rItem1->setRect(0,0, SCREEN::CELL_SIZE.width(),SCREEN::CELL_SIZE.height());
                rItem1->setPen(QPen(QColor(Qt::black)));
                rItem1->setBrush(QBrush(QColor(Qt::black)));
                rItem1->setPos((cx+1)*SCREEN::CELL_SIZE.width(), cy*SCREEN::CELL_SIZE.height());
                addItem(rItem1);

                QGraphicsRectItem* rItem2 = new QGraphicsRectItem();
                rItem2->setRect(0,0, SCREEN::CELL_SIZE.width(),SCREEN::CELL_SIZE.height());
                rItem2->setPen(QPen(QColor(Qt::black)));
                rItem2->setBrush(QBrush(QColor(Qt::black)));
                rItem2->setPos((cx-1)*SCREEN::CELL_SIZE.width(), cy*SCREEN::CELL_SIZE.height());
                addItem(rItem2);

                QGraphicsRectItem* rItem3 = new QGraphicsRectItem();
                rItem3->setRect(0,0, SCREEN::CELL_SIZE.width(),SCREEN::CELL_SIZE.height());
                rItem3->setPen(QPen(QColor(Qt::black)));
                rItem3->setBrush(QBrush(QColor(Qt::black)));
                rItem3->setPos(cx*SCREEN::CELL_SIZE.width(), (cy+1)*SCREEN::CELL_SIZE.height());
                addItem(rItem3);

                QGraphicsRectItem* rItem4 = new QGraphicsRectItem();
                rItem4->setRect(0,0, SCREEN::CELL_SIZE.width(),SCREEN::CELL_SIZE.height());
                rItem4->setPen(QPen(QColor(Qt::black)));
                rItem4->setBrush(QBrush(QColor(Qt::black)));
                rItem4->setPos(cx*SCREEN::CELL_SIZE.width(), (cy-1)*SCREEN::CELL_SIZE.height());
                addItem(rItem4);
                // Draws an Energy Bar, indicating how much energy should the weapon be
                // fired with
                for (int i = 0; i < 11 * fEnergyLevel; i++)
                {
                    QGraphicsRectItem* rItem0 = new QGraphicsRectItem();
                    rItem0->setRect(0,0, SCREEN::CELL_SIZE.width(),SCREEN::CELL_SIZE.height());
                    rItem0->setPen(QPen(QColor(Qt::green)));
                    rItem0->setBrush(QBrush(QColor(Qt::green)));
                    rItem0->setPos((worm->px - 5 + i - fCameraPosX)*SCREEN::CELL_SIZE.width(),
                                   (worm->py - 12 - fCameraPosY)*SCREEN::CELL_SIZE.height());
                    addItem(rItem0);

                    QGraphicsRectItem* rItem1 = new QGraphicsRectItem();
                    rItem1->setRect(0,0, SCREEN::CELL_SIZE.width(),SCREEN::CELL_SIZE.height());
                    rItem1->setPen(QPen(QColor(Qt::red)));
                    rItem1->setBrush(QBrush(QColor(Qt::red)));
                    rItem1->setPos((worm->px - 5 + i - fCameraPosX)*SCREEN::CELL_SIZE.width(),
                                   (worm->py - 11 - fCameraPosY)*SCREEN::CELL_SIZE.height());
                    addItem(rItem1);

//                    Draw(worm->px - 5 + i - fCameraPosX, worm->py - 12 - fCameraPosY, PIXEL_SOLID, FG_GREEN);
//                    Draw(worm->px - 5 + i - fCameraPosX, worm->py - 11 - fCameraPosY, PIXEL_SOLID, FG_RED);
                }
            }
        }
    }
    else{
        for (int x = 0; x < SCREEN::LOGICAL_SIZE.width(); x++)
            for (int y = 0; y < SCREEN::LOGICAL_SIZE.height(); y++)
            {
                float fx = (float)x/(float)SCREEN::LOGICAL_SIZE.width() * (float)nMapWidth;
                float fy = (float)y/(float)SCREEN::LOGICAL_SIZE.height() * (float)nMapHeight;

                switch (map[((int)fy)*nMapWidth + ((int)fx)])
                {
                case -1:m_image.setPixelColor(x, y, QColor(50, 60, 118)); break;
                case -2:m_image.setPixelColor(x, y, QColor(62, 78, 137)); break;
                case -3:m_image.setPixelColor(x, y, QColor(74, 97, 146)); break;
                case -4:m_image.setPixelColor(x, y, QColor(92, 130, 164)); break;
                case -5:m_image.setPixelColor(x, y, QColor(117, 164, 191)); break;
                case -6:m_image.setPixelColor(x, y, QColor(134, 183, 200)); break;
                case -7:m_image.setPixelColor(x, y, QColor(160, 210, 228)); break;
                case -8:m_image.setPixelColor(x, y, QColor(197, 240, 255)); break;
                case 0: m_image.setPixelColor(x, y, QColor(Qt::cyan));  break;
                case 1:	m_image.setPixelColor(x, y, QColor(Qt::darkGreen)); break;
                }
            }
        QGraphicsPixmapItem* pItem = new QGraphicsPixmapItem();
        pItem->setPixmap(QPixmap::fromImage(m_image.scaled(SCREEN::PHYSICAL_SIZE)));
        pItem->setZValue(LAYER::BG);
        addItem(pItem);
        for (auto &p : listObjects)
                        p->Draw(this, p->px-(p->px / (float)nMapWidth) * (float)SCREEN::LOGICAL_SIZE.width(),
                            p->py-(p->py / (float)nMapHeight) * (float)SCREEN::LOGICAL_SIZE.height(), true);
    }
}

QPoint GameScene::mousePosition() const
{
    return m_mousePosition;
}

void GameScene::handlePlayerInput()
{
    if(m_mouse->m_released)
    {
        boom((m_mousePosition.x()/SCREEN::CELL_SIZE.width()) + fCameraPosX, (m_mousePosition.y()/SCREEN::CELL_SIZE.height()) + fCameraPosY, 10.0f);
    }
    if(m_keys[KEYBOARD::KEY_1]->m_released)
    {
        listObjects.push_back(std::unique_ptr<Dummy>(new Dummy((m_mousePosition.x()/SCREEN::CELL_SIZE.width()) + fCameraPosX, (m_mousePosition.y()/SCREEN::CELL_SIZE.height()) + fCameraPosY)));
    }
    if(m_keys[KEYBOARD::KEY_2]->m_released)
    {
        listObjects.push_back(std::unique_ptr<Missile>(new Missile((m_mousePosition.x()/SCREEN::CELL_SIZE.width()) + fCameraPosX, (m_mousePosition.y()/SCREEN::CELL_SIZE.height()) + fCameraPosY)));
    }
    if(m_keys[KEYBOARD::KEY_3]->m_released)
    {
        Worm* worm = new Worm((m_mousePosition.x()/SCREEN::CELL_SIZE.width()) + fCameraPosX, (m_mousePosition.y()/SCREEN::CELL_SIZE.height()) + fCameraPosY);
        pObjectUnderControl = worm;
        pCameraTrackingObject = worm;
        listObjects.push_back(std::unique_ptr<Worm>(worm));
    }

    if(m_keys[KEYBOARD::KEY_9]->m_released)
    {
        CreateMap();
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
        if(event->isAutoRepeat())
        {
            m_keys[KEYBOARD::KeysMapper[event->key()]]->m_held = true;
            m_keys[KEYBOARD::KeysMapper[event->key()]]->m_pressed = false;
        }
        else
        {
            m_keys[KEYBOARD::KeysMapper[event->key()]]->m_pressed = true;
            m_keys[KEYBOARD::KeysMapper[event->key()]]->m_held    = false;
        }
    }
    QGraphicsScene::keyPressEvent(event);
}

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    if(KEYBOARD::KeysMapper.contains(event->key()))
    {
        if(!event->isAutoRepeat())
        {
            m_keys[KEYBOARD::KeysMapper[event->key()]]->m_held = false;
            m_keys[KEYBOARD::KeysMapper[event->key()]]->m_pressed = false;
            m_keys[KEYBOARD::KeysMapper[event->key()]]->m_released = true;
        }

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
