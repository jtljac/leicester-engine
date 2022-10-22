//
// Created by jacob on 21/10/22.
//

#include <LeicesterEngine.h>
#include <Rendering/Vulkan/VulkanRenderer.h>

int main() {
    LeicesterEngine engine;
    engine.setRenderer(new VulkanRenderer());
    engine.initialise();
}