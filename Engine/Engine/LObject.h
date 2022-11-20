//
// Created by jacob on 20/11/22.
//

#pragma once

/**
 *
 */
class LObject {
public:
    virtual void onCreate() = 0;
    virtual void tick(double deltaTime) = 0;
    virtual void onDestroy() = 0;
};
