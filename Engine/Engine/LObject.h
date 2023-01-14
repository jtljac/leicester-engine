//
// Created by jacob on 20/11/22.
//

#pragma once

#include <glm/glm.hpp>

/**
 * An object in the world
 */
class LObject {
protected:
    /** The object should run its tick function every frame */
    bool enableTick;

    /** The Object's position in the world */
    glm::vec3 position;
    /** The Object's rotation in the world */
    glm::quat rotation;
    /** The Object's scale in the world */
    glm::vec3 scale;

    /**
     * A pointer to an object of which this object's position, rotation, and scale is based off of
     */
    LObject* parent;
public:
    LObject()
            : position({0, 0, 0}), scale({1, 1, 1}), rotation(glm::quat({0, 0, 0})), parent(nullptr), enableTick(true) {}

    LObject(LObject* parent, bool enableTick)
            : position({0, 0, 0}), scale({1, 1, 1}), rotation(glm::quat({0, 0, 0})), parent(parent), enableTick(enableTick) {}

    LObject(const glm::vec3& position, const glm::vec3& scale, const glm::quat& rotation, LObject* parent, const bool enableTick)
            : position(position), scale(scale), rotation(rotation), parent(parent), enableTick(enableTick) {}

    LObject(LObject& lObject) = default;

    /** The function called when the object is added to the world */
    virtual void onCreate() = 0;
    /**
     * The function called every frame
     * @param deltaTime The time in seconds the last frame took
     */
    virtual void tick(double deltaTime) = 0;
    /** The function called when the object is removed from the world */
    virtual void onDestroy() = 0;

    /**
     * Get if the object has ticking enabled
     * @return true if ticking is enabled
     */
    [[nodiscard]] virtual bool isTicking() const {
        return enableTick;
    }

    /**
     * Set if the object should tick
     * @param enableTick Whether the object should tick
     */
    virtual void setTicking(bool enableTick) {
        LObject::enableTick = enableTick;
    }

    /**
     * Get the current position of the object in the world
     * @return the position of the object
     */
    [[nodiscard]] virtual const glm::vec3& getPosition() const {
        return position;
    }

    /**
     * Set the position of the object in the world
     * @param position The new position of the object in the world
     */
    virtual void setPosition(const glm::vec3& position) {
        LObject::position = position;
    }

    /**
     * Get the scale of the object
     * @return the scale of the object
     */
    [[nodiscard]] virtual const glm::vec3& getScale() const {
        return scale;
    }

    /**
     * Set the scale of the object
     * @param scale The new scale of the object
     */
    virtual void setScale(const glm::vec3& scale) {
        LObject::scale = scale;
    }

    /**
     * Get the rotation of the object in the world
     * @return the rotation of the object
     */
    [[nodiscard]] virtual const glm::quat& getRotation() const {
        return rotation;
    }

    /**
     * Set the rotation of the object in the world
     * @param rotation The new rotation of the object
     */
    virtual void setRotation(const glm::quat& rotation) {
        LObject::rotation = rotation;
    }
};
