#pragma once

#include <GLFW/glfw3.h>

#include <Rendering/Renderer.h>

class LeicesterEngine {
protected:
    Renderer* renderer = nullptr;
    double lastFrameTime = 0, frameDelta = 0;
public:
    virtual int initialise();

    void setRenderer(Renderer* pRenderer);
};
