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


    Actor* collisionObject = new Actor(nullptr, new SphereCollider(CollisionMode::QUERY, 1.f));
    collisionObject->position = glm::vec3(3, 3, 3);
    scene->addActorToScene(collisionObject);

    Actor* collisionObject2 = new Actor(nullptr, new SphereCollider(CollisionMode::QUERY, 1.f));
    collisionObject2->position = glm::vec3(-3, 3, 3);
    scene->addActorToScene(collisionObject2);

    Actor* collisionObject3 = new Actor(nullptr, new SphereCollider(CollisionMode::QUERY, 1.f));
    collisionObject3->position = glm::vec3(3, -3, 3);
    scene->addActorToScene(collisionObject3);

    Actor* collisionObject4 = new Actor(nullptr, new SphereCollider(CollisionMode::QUERY, 1.f));
    collisionObject4->position = glm::vec3(-3, -3, 3);
    scene->addActorToScene(collisionObject4);

    Actor* collisionObject5 = new Actor(nullptr, new SphereCollider(CollisionMode::QUERY, 1.f));
    collisionObject5->position = glm::vec3(3, 3, -3);
    scene->addActorToScene(collisionObject5);

    Actor* collisionObject6 = new Actor(nullptr, new SphereCollider(CollisionMode::QUERY, 1.f));
    collisionObject6->position = glm::vec3(-3, 3, -3);
    scene->addActorToScene(collisionObject6);

    Actor* collisionObject7 = new Actor(nullptr, new AABBCollider(CollisionMode::QUERY, {-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}));
    collisionObject7->position = glm::vec3(3, -3, -3);
    scene->addActorToScene(collisionObject7);

    Actor* collisionObject8 = new Actor(nullptr, new MeshCollider(CollisionMode::QUERY, Mesh::createNewMeshFromFile(FileUtils::getAssetsPath() + "/Shapes/Diamond.lmesh")));

    collisionObject8->position = glm::vec3(-3, -3, -3);
    scene->addActorToScene(collisionObject8);

    Actor* monkey = new ControlledActor(new StaticMesh(mesh, triangleMaterial),
                                        new SphereCollider(CollisionMode::QUERY, 1));
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