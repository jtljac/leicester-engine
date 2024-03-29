#pragma once

#include <GLFW/glfw3.h>

#include <Rendering/Renderer.h>
#include <Collision/CollisionEngine.h>

class LeicesterEngine {
protected:
    Renderer* renderer = nullptr;
    CollisionEngine* collisionEngine = nullptr;
    double lastFrameTime = 0, frameDelta = 0;
public:
    EngineSettings settings;

    Scene* currentScene = nullptr;

    virtual int initialise();

    virtual int startLoop();

    void setRenderer(Renderer* pRenderer);

    Renderer* getRenderer() const;

    void setCollisionEngine(CollisionEngine* pCollisionEngine);

    CollisionEngine* getCollisionEngine() const;

    void setScene(Scene* scene);
};
