#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QElapsedTimer>
#include <QTimer>
#include <list>
#include <memory>
#include "physics_object.h"
#include "team.h"
#include <QImage>

struct KeyStatus
{
    bool m_pressed = false;
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
    void createMap();
    void perlinNoise1D(int nCount, float *fSeed, int nOctaves, float fBias, float *fOutput);
    void drawLandscapeAndObjects();
    QPoint mousePosition() const;
    void boom(float fWorldX, float fWorldY, float fRadius);
    void updatePhysics();
    void mouseEdgeMapScroll();
    void controlSupervisor();
    void handleAIStateMachine();
    void handleObjectUnderControl();
    void clampMapBoundaries();
    void decreaseTurnTime();
    void checkGameStateStability();
    void drawTeamHealthBars();
    void drawCounter();
    void updateStateMachine();
    void renderGameScene();
    //Terrain size
    int nMapWidth  = 1024;
    int nMapHeight = 512;
    char *map = nullptr;
    // Camera coordinates
    float fCameraPosX = 0.0f;
    float fCameraPosY = 0.0f;
    float fCameraPosXTarget = 0.0f;
    float fCameraPosYTarget = 0.0f;

    enum GAME_STATE
    {
        GS_RESET = 0,
        GS_GENERATE_TERRAIN = 1,
        GS_GENERATING_TERRAIN,
        GS_ALLOCATE_UNITS,
        GS_ALLOCATING_UNITS,
        GS_START_PLAY,
        GS_CAMERA_MODE,
        GS_GAME_OVER1,
        GS_GAME_OVER2
    } nGameState, nNextState;

    enum AI_STATE
    {
        AI_ASSESS_ENVIRONMENT = 0,
        AI_MOVE,
        AI_CHOOSE_TARGET,
        AI_POSITION_FOR_TARGET,
        AI_AIM,
        AI_FIRE,
    } nAIState, nAINextState;

    bool bZoomOut = false;
    bool bGameIsStable = false;
    bool bEnablePlayerControl = true;		// The player is in control, keyboard input enabled
    bool bEnableComputerControl = false;	// The AI is in control
    bool bEnergising = false;				// Weapon is charging
    bool bFireWeapon = false;				// Weapon should be discharged
    bool bShowCountDown = false;			// Display turn time counter on screen
    bool bPlayerHasFired = false;			// Weapon has been discharged
    //List of things that exist in game world
    std::list<std::unique_ptr<PhysicsObject>> listObjects;
    PhysicsObject* pObjectUnderControl = nullptr;
    PhysicsObject* pCameraTrackingObject = nullptr;

    float fEnergyLevel = 0.0f;
    float fTurnTime    = 0.0f;
    std::vector<Team> vecTeams;
    int nCurrentTeam = 0;

    // AI control flags
    bool bAI_Jump = false;				// AI has pressed "JUMP" key
    bool bAI_AimLeft = false;			// AI has pressed "AIM_LEFT" key
    bool bAI_AimRight = false;			// AI has pressed "AIM_RIGHT" key
    bool bAI_Energise = false;			// AI has pressed "FIRE" key


    float fAITargetAngle = 0.0f;		// Angle AI should aim for
    float fAITargetEnergy = 0.0f;		// Energy level AI should aim for
    float fAISafePosition = 0.0f;		// X-Coordinate considered safe for AI to move to
    Worm* pAITargetWorm = nullptr;		// Pointer to worm AI has selected as target
    float fAITargetX = 0.0f;			// Coordinates of target missile location
    float fAITargetY = 0.0f;

    QImage m_image;
    QPoint m_mousePosition;

    void handlePlayerInput();
    void resetStatus();
    KeyStatus* m_keys[256];
    MouseStatus* m_mouse;

    const int FPS = 45;
    QTimer m_timer;
    QElapsedTimer m_elapsedTimer;
    float m_deltaTime = 0.0f, m_loopTime = 0.0f;
    const float m_loopSpeed = int(1000.0f/FPS);
protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // GAMESCENE_H
