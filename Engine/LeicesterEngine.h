#pragma once

#include <GLFW/glfw3.h>

#include <Rendering/Renderer.h>

class LeicesterEngine {
protected:
    Renderer* renderer = nullptr;
    double lastFrameTime = 0, frameDelta = 0;
public:
    EngineSettings settings;

    Scene* currentScene;

    virtual int initialise();

    virtual int startLoop();

    void setRenderer(Renderer* pRenderer);

    Renderer* getRenderer() const;
};
