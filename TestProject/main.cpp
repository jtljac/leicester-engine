//
// Created by jacob on 21/10/22.
//

#include <LeicesterEngine.h>
#include <Rendering/Vulkan/VulkanRenderer.h>
#include "Utils/FileUtils.h"
#include "Collision/SphereCollider.h"

void addActorsToScene(Scene* scene) {
    Mesh* mesh = new Mesh();
    mesh->loadMeshFromFile(FileUtils::getAssetsPath() + "/Monkey.lmesh");


    Mesh* triangleMesh = new Mesh;
    triangleMesh->vertices.resize(3);
    triangleMesh->vertices[0].position = {0.5f, 0.5f, 0.f};
    triangleMesh->vertices[1].position = {-0.5f, 0.5f, 0.f};
    triangleMesh->vertices[2].position = {0.f, -0.5f, 0.f};

    triangleMesh->vertices[0].colour = {1.f, 0.f, 0.f};
    triangleMesh->vertices[1].colour = {0.f, 1.f, 0.f};
    triangleMesh->vertices[2].colour = {0.f, 0.f, 1.f};

    triangleMesh->indices = {0, 1, 2};
    Material* triangleMaterial = new Material("/meshtriangle.vert.spv", "/colourtriangle.frag.spv");

    Actor* triangle = new Actor(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle->position = glm::vec3(1, 1, 1);
    Actor* triangle1 = new Actor(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle1->position = glm::vec3(-1, 1, 1);
    Actor* triangle2 = new Actor(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle2->position = glm::vec3(1, -1, 1);
    Actor* triangle3 = new Actor(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle3->position = glm::vec3(-1, -1, 1);
    Actor* triangle4 = new Actor(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle4->position = glm::vec3(1, 1, -1);
    Actor* triangle5 = new Actor(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle5->position = glm::vec3(-1, 1, -1);
    Actor* triangle6 = new Actor(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle6->position = glm::vec3(1, -1, -1);
    Actor* triangle7 = new Actor(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle7->position = glm::vec3(-1, -1, -1);
    Actor* triangle8 = new Actor(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle8->position = glm::vec3(1, 2, 1);
    Actor* triangle9 = new Actor(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle9->position = glm::vec3(1, -2, 1);

    scene->addActorToScene(triangle);
    scene->addActorToScene(triangle1);
    scene->addActorToScene(triangle2);
    scene->addActorToScene(triangle3);
    scene->addActorToScene(triangle4);
    scene->addActorToScene(triangle5);
    scene->addActorToScene(triangle6);
    scene->addActorToScene(triangle7);
    scene->addActorToScene(triangle8);
    scene->addActorToScene(triangle9);

    Actor* monkey = new Actor(new StaticMesh(mesh, triangleMaterial), new SphereCollider(1.f));
    scene->addActorToScene(monkey);
}

int main() {
    LeicesterEngine engine;
    engine.setRenderer(new VulkanRenderer());
    engine.initialise();

    engine.currentScene = new Scene();
    addActorsToScene(engine.currentScene);
    engine.startLoop();
}