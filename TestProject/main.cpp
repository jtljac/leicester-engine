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

    scene->actors.push_back(Actor(new StaticMesh(triangleMesh, triangleMaterial), nullptr));
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