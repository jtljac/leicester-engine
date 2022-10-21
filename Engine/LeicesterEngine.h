#pragma once

#include <GLFW/glfw3.h>

#include <Rendering/Renderer.h>

class LeicesterEngine {
protected:
    Renderer* renderer;
    double lastFrameTime, frameDelta;
public:
    virtual int initialise();
};
