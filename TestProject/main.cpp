//
// Created by jacob on 21/10/22.
//

#include <LeicesterEngine.h>
#include <Rendering/Vulkan/VulkanRenderer.h>
#include "Utils/FileUtils.h"

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

    Actor triangle(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle.position = glm::vec3(1, 1, 1);
    Actor triangle1(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle1.position = glm::vec3(-1, 1, 1);
    Actor triangle2(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle2.position = glm::vec3(1, -1, 1);
    Actor triangle3(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle3.position = glm::vec3(-1, -1, 1);
    Actor triangle4(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle4.position = glm::vec3(1, 1, -1);
    Actor triangle5(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle5.position = glm::vec3(-1, 1, -1);
    Actor triangle6(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle6.position = glm::vec3(1, -1, -1);
    Actor triangle7(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle7.position = glm::vec3(-1, -1, -1);
    Actor triangle8(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle8.position = glm::vec3(1, 2, 1);
    Actor triangle9(new StaticMesh(triangleMesh, triangleMaterial), nullptr);
    triangle9.position = glm::vec3(1, -2, 1);

    scene->actors.push_back(triangle);
    scene->actors.push_back(triangle1);
    scene->actors.push_back(triangle2);
    scene->actors.push_back(triangle3);
    scene->actors.push_back(triangle4);
    scene->actors.push_back(triangle5);
    scene->actors.push_back(triangle6);
    scene->actors.push_back(triangle7);
    scene->actors.push_back(triangle8);
    scene->actors.push_back(triangle9);
    scene->actors.push_back(Actor(new StaticMesh(mesh, triangleMaterial), nullptr));
}

int main() {
    LeicesterEngine engine;
    engine.setRenderer(new VulkanRenderer());
    engine.initialise();

    engine.currentScene = new Scene();
    addActorsToScene(engine.currentScene);
    engine.startLoop();
}