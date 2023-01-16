//
// Created by jacob on 21/10/22.
//

#include <LeicesterEngine.h>
#include <Rendering/Vulkan/VulkanRenderer.h>
#include "Utils/FileUtils.h"
#include "Collision/SphereCollider.h"
#include "ControlledActor.h"
#include "Collision/GJKCollisionEngine.h"
#include "Collision/AABBCollider.h"
#include "Collision/MeshCollider.h"

void addActorsToScene(Scene* scene) {
    Mesh* mesh = new Mesh();
    mesh->loadMeshFromFile(FileUtils::getAssetsPath() + "/Monkey.lmesh");

    Material* triangleMaterial = new Material("/meshtriangle.vert.spv", "/colourtriangle.frag.spv");


    Actor* collisionObject = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    collisionObject->setPosition(glm::vec3(3, 3, 3));
    scene->addActorToScene(collisionObject);

    Actor* collisionObject2 = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    collisionObject2->setPosition(glm::vec3(-3, 3, 3));
    scene->addActorToScene(collisionObject2);

    Actor* collisionObject3 = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    collisionObject3->setPosition(glm::vec3(3, -3, 3));
    scene->addActorToScene(collisionObject3);

    Actor* collisionObject4 = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    collisionObject4->setPosition(glm::vec3(-3, -3, 3));
    scene->addActorToScene(collisionObject4);

    Actor* collisionObject5 = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    collisionObject5->setPosition(glm::vec3(3, 3, -3));
    scene->addActorToScene(collisionObject5);

    Actor* collisionObject6 = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    collisionObject6->setPosition(glm::vec3(-3, 3, -3));
    scene->addActorToScene(collisionObject6);

    Actor* collisionObject7 = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}));
    collisionObject7->setPosition(glm::vec3(3, -3, -3));
    scene->addActorToScene(collisionObject7);

    Actor* collisionObject8 = new Actor(nullptr, new MeshCollider(CollisionMode::BLOCK, Mesh::createNewMeshFromFile(FileUtils::getAssetsPath() + "/Shapes/Diamond.lmesh")));

    collisionObject8->setPosition(glm::vec3(-3, -3, -3));
    scene->addActorToScene(collisionObject8);

    Actor* monkey = new ControlledActor(new StaticMesh(mesh, triangleMaterial),
                                        new AABBCollider(CollisionMode::BLOCK, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    scene->addActorToScene(monkey);
    scene->setControlledActor(monkey);
}

int main() {
    LeicesterEngine engine;
    engine.setRenderer(new VulkanRenderer());
    engine.setCollisionEngine(new GJKCollisionEngine());
    engine.initialise();

    engine.setScene(new Scene());
    addActorsToScene(engine.currentScene);
    engine.startLoop();
}