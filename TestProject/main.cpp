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

Scene *pbrScene();

Scene* collisionScene() {
    Scene* scene = new Scene();
    Mesh* mesh = Mesh::createNewMeshFromFile(FileUtils::getAssetsPath() + "/Monkey.lmesh");

    Texture* texture = Texture::createNewTextureFromFile(FileUtils::getAssetsPath() + "/wall.ltex");
    std::vector<Texture*> texArray;
    texArray.push_back(texture);
    Material* triangleMaterial = new Material("/meshtriangle.vert.spv", "/colourtriangle.frag.spv", ShaderType::OPAQUE, texArray);


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

    return scene;
}

Scene *pbrScene() {
    Scene* scene = new Scene();

    Mesh* mesh = Mesh::createNewMeshFromFile(FileUtils::getAssetsPath() + "/Shapes/Sphere.lmesh");
    std::vector<Texture*> texArray;
    texArray.push_back(Texture::createNewTextureFromFile(FileUtils::getAssetsPath() + "/PBR Example/rustediron2_basecolor.ltex"));
    texArray.push_back(Texture::createNewTextureFromFile(FileUtils::getAssetsPath() + "/PBR Example/rustediron2_metallic.ltex"));
    texArray.push_back(Texture::createNewTextureFromFile(FileUtils::getAssetsPath() + "/PBR Example/rustediron2_normal.ltex"));
    texArray.push_back(Texture::createNewTextureFromFile(FileUtils::getAssetsPath() + "/PBR Example/rustediron2_roughness.ltex"));
    Material* mat = new Material("/meshtriangle.vert.spv", "/PBR Example/pbr_sphere.frag.spv", ShaderType::OPAQUE, texArray);
    StaticMesh* staticMesh = new StaticMesh(mesh, mat);

    Actor* sphere1 = new Actor(staticMesh, nullptr);
    sphere1->setPosition(glm::vec3(-2, 2, 0));
    scene->addActorToScene(sphere1);

    Actor* sphere2 = new Actor(staticMesh, nullptr);
    sphere2->setPosition(glm::vec3(2, 2, 0));
    scene->addActorToScene(sphere2);

    Actor* sphere3 = new Actor(staticMesh, nullptr);
    sphere3->setPosition(glm::vec3(-2, -2, 0));
    scene->addActorToScene(sphere3);

    Actor* sphere4 = new Actor(staticMesh, nullptr);
    sphere4->setPosition(glm::vec3(2, -2, 0));
    scene->addActorToScene(sphere4);

    Actor* controlled = new ControlledActor(nullptr, nullptr);
    scene->addActorToScene(controlled);
    scene->setControlledActor(controlled);

    return scene;
}

int main() {
    LeicesterEngine engine;
    engine.setRenderer(new VulkanRenderer());
    engine.setCollisionEngine(new GJKCollisionEngine());
    engine.initialise();

//    engine.setScene(collisionScene());
    engine.setScene(pbrScene());
    engine.startLoop();
}
