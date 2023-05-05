//
// Created by jacob on 20/11/22.
//

#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>

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
    glm::vec3 rotation;
    /** The Object's scale in the world */
    glm::vec3 scale;

    /**
     * A pointer to an object of which this object's position, rotation, and scale is based off of
     */
    LObject* parent;
public:
    LObject()
            : position({0, 0, 0}), scale({1, 1, 1}), rotation({0, 0, 0}), parent(nullptr), enableTick(true) {}

    LObject(LObject* parent, bool enableTick)
            : position({0, 0, 0}), scale({1, 1, 1}), rotation({0, 0, 0}), parent(parent), enableTick(enableTick) {}

    LObject(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, LObject* parent, const bool enableTick)
            : position(position), scale(scale), rotation(rotation), parent(parent), enableTick(enableTick) {}

    LObject(LObject& lObject) = default;

    /** The function called when the object is added to the world */
    virtual void onCreate() {};
    /**
     * The function called every frame
     * @param deltaTime The time in seconds the last frame took
     */
    virtual void tick(double deltaTime) {};
    /** The function called when the object is removed from the world */
    virtual void onDestroy() {};

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
     * Get the world transform of the object
     * @return the global transform of the actor
     */
    [[nodiscard]] virtual const glm::mat4 getTransform() const {
        if (parent != nullptr) {
            return getLocalTransform() * parent->getTransform();
        } else {
            return getLocalTransform();
        }
    }

    /**
     * Get the local transform of the actor
     * @return the transform of the actor in local space
     */
    [[nodiscard]] virtual const glm::mat4 getLocalTransform() const {
        return glm::translate(glm::mat4(1), getLocalPosition()) * glm::orientate4(getLocalRotation()) * glm::scale(glm::mat4(1), getLocalScale());
    }

    /**
     * Get the current position of the object in local space
     * @return the local position of the object
     */
    [[nodiscard]] virtual const glm::vec3& getLocalPosition() const {
        return position;
    }

    [[nodiscard]] virtual glm::vec3 getPosition() const {
        if (parent != nullptr) {
            return (glm::vec3) (parent->getTransform() * glm::vec4(position, 1));
        } else {
            return getLocalPosition();
        }
    }

    /**
     * Set the position of the object in local space
     * @param position The new position of the object in local space
     */
    virtual void setLocalPosition(const glm::vec3& position) {
        LObject::position = position;
    }

    /**
     * Get the scale of the object in local space
     * @return the local scale of the object
     */
    [[nodiscard]] virtual const glm::vec3& getLocalScale() const {
        return scale;
    }

    [[nodiscard]] virtual glm::vec3 getScale() const {
        if (parent != nullptr) {
            return getLocalScale() + parent->getScale();
        } else {
            return getLocalScale();
        }
    }

    /**
     * Set the scale of the object in local space
     * @param scale The new local scale of the object
     */
    virtual void setLocalScale(const glm::vec3& scale) {
        LObject::scale = scale;
    }

    /**
     * Get the rotation of the object in local space
     * @return the local rotation of the object
     */
    [[nodiscard]] virtual const glm::vec3& getLocalRotation() const {
        return rotation;
    }

    /**
     * Set the rotation of the object in local space
     * @param rotation The new local rotation of the object
     */
    virtual void setLocalRotation(const glm::vec3& rotation) {
        LObject::rotation = rotation;
    }

    /**
     * Set the parent of the LObject
     * @param parent the new parent of the LObject
     */
    void setParent(LObject* parent) {
        this->parent = parent;
    }
};