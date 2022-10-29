//
// Created by jacob on 29/10/22.
//

#include "Rendering/Vulkan/DeletionQueue.h"

void DeletionQueue::pushDeletor(std::function<void(VkDevice&)>&& deletor) {
    this->deletors.push_back(deletor);
}

void DeletionQueue::flush(VkDevice& device) {
    while (!this->deletors.empty()) {
        deletors.front()(device);
        deletors.pop_front();
    }
}
