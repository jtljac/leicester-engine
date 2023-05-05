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
#include "RockingActor.h"

Scene *pbrTest();

Scene* collisionScene() {
    Scene* scene = new Scene();
    Mesh* mesh = Mesh::createNewMeshFromFile(FileUtils::getAssetsPath() + "/Monkey.lmesh");

    Texture* texture = Texture::createNewTextureFromFile(FileUtils::getAssetsPath() + "/wall.ltex");
    std::vector<Texture*> texArray;
    texArray.push_back(texture);
    Material* triangleMaterial = new Material("/meshtriangle.vert.spv", "/colourtriangle.frag.spv", ShaderType::OPAQUE, texArray);


    Actor* collisionObject = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    collisionObject->setLocalPosition(glm::vec3(3, 3, 3));
    scene->addActorToScene(collisionObject);

    Actor* collisionObject2 = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    collisionObject2->setLocalPosition(glm::vec3(-3, 3, 3));
    scene->addActorToScene(collisionObject2);

    Actor* collisionObject3 = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    collisionObject3->setLocalPosition(glm::vec3(3, -3, 3));
    scene->addActorToScene(collisionObject3);

    Actor* collisionObject4 = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    collisionObject4->setLocalPosition(glm::vec3(-3, -3, 3));
    scene->addActorToScene(collisionObject4);

    Actor* collisionObject5 = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    collisionObject5->setLocalPosition(glm::vec3(3, 3, -3));
    scene->addActorToScene(collisionObject5);

    Actor* collisionObject6 = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    collisionObject6->setLocalPosition(glm::vec3(-3, 3, -3));
    scene->addActorToScene(collisionObject6);

    Actor* collisionObject7 = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}));
    collisionObject7->setLocalPosition(glm::vec3(3, -3, -3));
    scene->addActorToScene(collisionObject7);

    Actor* collisionObject8 = new Actor(nullptr, new MeshCollider(CollisionMode::BLOCK, Mesh::createNewMeshFromFile(FileUtils::getAssetsPath() + "/Shapes/Diamond.lmesh")));

    collisionObject8->setLocalPosition(glm::vec3(-3, -3, -3));
    scene->addActorToScene(collisionObject8);

    Actor* monkey = new ControlledActor(new StaticMesh(mesh, triangleMaterial),
                                        new AABBCollider(CollisionMode::BLOCK, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    scene->addActorToScene(monkey);
    scene->setControlledActor(monkey);

    return scene;
}

Scene* collisionResponseTestScene() {
    Scene* scene = new Scene();
    Mesh* mesh = Mesh::createNewMeshFromFile(FileUtils::getAssetsPath() + "/Monkey.lmesh");

    Material* mat = new Material("/meshtriangle.vert.spv", "/Collision Test/collision_test.frag.spv", ShaderType::OPAQUE);
    Actor* controlled = new ControlledActor(new StaticMesh(mesh, mat),
                                            new AABBCollider(CollisionMode::BLOCK, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));

    Actor* collisionObject1 = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}));
    collisionObject1->setLocalPosition(glm::vec3(3, -3, -3));
    scene->addActorToScene(collisionObject1);

    Actor* collisionObject2 = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}));
    collisionObject2->setLocalPosition(glm::vec3(3, -3, -3));
    scene->addActorToScene(collisionObject2);

    Actor* collisionObject3 = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}));
    collisionObject3->setLocalPosition(glm::vec3(3, -3, -3));
    scene->addActorToScene(collisionObject3);

    Actor* collisionObject4 = new Actor(nullptr, new AABBCollider(CollisionMode::BLOCK, {-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}));
    collisionObject4->setLocalPosition(glm::vec3(3, -3, -3));
    scene->addActorToScene(collisionObject4);

    controlled->setLocalPosition(glm::vec3(0, 0, -3));
    scene->addActorToScene(controlled);
    scene->setControlledActor(controlled);

    return scene;

}

Scene* collisionTestScene() {
    Scene* scene = new Scene();

    Mesh* mesh = Mesh::createNewMeshFromFile(FileUtils::getAssetsPath() + "/Shapes/Diamond.lmesh");

    // Colliders 1
    Actor* collisionObject11 = new Actor(nullptr, new AABBCollider(CollisionMode::QUERY, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    collisionObject11->setLocalPosition(glm::vec3(0, 0, 0));
    scene->addActorToScene(collisionObject11);

    Actor* collisionObject12 = new Actor(nullptr, new SphereCollider(CollisionMode::QUERY, 1));
    collisionObject12->setLocalPosition(glm::vec3(-2, 0, 0));
    scene->addActorToScene(collisionObject12);

    Actor* collisionObject13 = new Actor(nullptr, new MeshCollider(CollisionMode::QUERY, mesh));
    collisionObject13->setLocalPosition(glm::vec3(2, 0, 0));
    scene->addActorToScene(collisionObject13);

    Actor* collisionObject14 = new RockingActor(nullptr, new AABBCollider(CollisionMode::QUERY, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}), {0,0,0}, 5);
    collisionObject14->setLocalPosition(glm::vec3(0, 0, 0));
    scene->addActorToScene(collisionObject14);
//
//    // Colliders 2
    Actor* collisionObject21 = new Actor(nullptr, new AABBCollider(CollisionMode::QUERY, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    collisionObject21->setLocalPosition(glm::vec3(0, 0, 4));
    scene->addActorToScene(collisionObject21);

    Actor* collisionObject22 = new Actor(nullptr, new SphereCollider(CollisionMode::QUERY, 1));
    collisionObject22->setLocalPosition(glm::vec3(-2, 0, 4));
    scene->addActorToScene(collisionObject22);

    Actor* collisionObject23 = new Actor(nullptr, new MeshCollider(CollisionMode::QUERY, mesh));
    collisionObject23->setLocalPosition(glm::vec3(2, 0, 4));
    scene->addActorToScene(collisionObject23);

    Actor* collisionObject24 = new RockingActor(nullptr, new SphereCollider(CollisionMode::QUERY, 1), {0,0,4}, 3);
    collisionObject24->setLocalPosition(glm::vec3(0, 0, 4));
    scene->addActorToScene(collisionObject24);

//     Colliders 3
    Actor* collisionObject31 = new Actor(nullptr, new AABBCollider(CollisionMode::QUERY, {-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));
    collisionObject31->setLocalPosition(glm::vec3(0, 0, -4));
    scene->addActorToScene(collisionObject31);

    Actor* collisionObject32 = new Actor(nullptr, new SphereCollider(CollisionMode::QUERY, 1));
    collisionObject32->setLocalPosition(glm::vec3(-2, 0, -4));
    scene->addActorToScene(collisionObject32);

    Actor* collisionObject33 = new Actor(nullptr, new MeshCollider(CollisionMode::QUERY, mesh));
    collisionObject33->setLocalPosition(glm::vec3(2, 0, -4));
    scene->addActorToScene(collisionObject33);

    Actor* collisionObject34 = new RockingActor(nullptr, new MeshCollider(CollisionMode::QUERY, mesh), {0,0,-4}, 5);
    collisionObject34->setLocalPosition(glm::vec3(0, 0, -4));
    scene->addActorToScene(collisionObject34);

    Actor* controlled = new ControlledActor(nullptr, nullptr);
    controlled->setLocalPosition(glm::vec3(0, -3, 0));
    scene->addActorToScene(controlled);
    scene->setControlledActor(controlled);

    return scene;
}

Scene* pbrTest() {
    Scene* scene = new Scene();

    Mesh* mesh = Mesh::createNewMeshFromFile(FileUtils::getAssetsPath() + "/Shapes/Sphere.lmesh");
    std::vector<Texture*> texArray;
    texArray.push_back(Texture::createNewTextureFromFile(FileUtils::getAssetsPath() + "/PBR Example/rustediron2_basecolor.ltex"));
    texArray.push_back(Texture::createNewTextureFromFile(FileUtils::getAssetsPath() + "/PBR Example/rustediron2_metallic.ltex"));
    texArray.push_back(Texture::createNewTextureFromFile(FileUtils::getAssetsPath() + "/PBR Example/rustediron2_normal.ltex"));
    texArray.push_back(Texture::createNewTextureFromFile(FileUtils::getAssetsPath() + "/PBR Example/rustediron2_roughness.ltex"));
    Material* mat = new Material("/meshtriangle.vert.spv", "/PBR Test/metallic-unrough.frag.spv", ShaderType::OPAQUE);
    Material* mat1 = new Material("/meshtriangle.vert.spv", "/PBR Test/metallicrough.frag.spv", ShaderType::OPAQUE);
    Material* mat2 = new Material("/meshtriangle.vert.spv", "/PBR Test/nonmetallic-unrough.frag.spv", ShaderType::OPAQUE);
    Material* mat3 = new Material("/meshtriangle.vert.spv", "/PBR Test/nonmetallic-rough.frag.spv", ShaderType::OPAQUE);
    Material* mat4 = new Material("/meshtriangle.vert.spv", "/PBR Example/pbr_sphere.frag.spv", ShaderType::OPAQUE, texArray);
    StaticMesh* staticMesh = new StaticMesh(mesh, mat);
    StaticMesh* staticMesh2 = new StaticMesh(mesh, mat1);
    StaticMesh* staticMesh3 = new StaticMesh(mesh, mat2);
    StaticMesh* staticMesh4 = new StaticMesh(mesh, mat3);
    StaticMesh* staticMesh5 = new StaticMesh(mesh, mat4);
    StaticMesh* staticMesh6 = new StaticMesh(mesh, mat4);

    Actor* sphere1 = new Actor(staticMesh, nullptr);
    sphere1->setLocalPosition(glm::vec3(2, 2, 0));
    scene->addActorToScene(sphere1);

    Actor* sphere2 = new Actor(staticMesh2, nullptr);
    sphere2->setLocalPosition(glm::vec3(-2, 2, 0));
    scene->addActorToScene(sphere2);

    Actor* sphere3 = new Actor(staticMesh3, nullptr);
    sphere3->setLocalPosition(glm::vec3(2, -2, 0));
    scene->addActorToScene(sphere3);

    Actor* sphere4 = new Actor(staticMesh4, nullptr);
    sphere4->setLocalPosition(glm::vec3(-2, -2, 0));
    scene->addActorToScene(sphere4);

    Actor* sphere5 = new Actor(staticMesh5, nullptr);
    sphere5->setLocalPosition(glm::vec3(3, 0, 0));
    scene->addActorToScene(sphere5);

    Actor* sphere6 = new Actor(staticMesh6, nullptr);
    sphere6->setLocalPosition(glm::vec3(-3, 0, 0));
    scene->addActorToScene(sphere6);

    Actor* controlled = new ControlledActor(new StaticMesh(mesh, new Material("/meshtriangle.vert.spv", "/Collision Test/collision_test.frag.spv", ShaderType::OPAQUE)), nullptr);
    controlled->setLocalPosition(glm::vec3(0, 0, -3));
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
//    engine.setScene(pbrTest());
//    engine.setScene(collisionTestScene());
    engine.setScene(collisionResponseTestScene());
    engine.startLoop();
}
