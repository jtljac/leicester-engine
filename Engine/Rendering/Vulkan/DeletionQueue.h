/**
 * This code (and it's source) has been created using the following tutorial:
 * https://vkguide.dev/docs/chapter-2/cleanup/     MIT Licence: https://github.com/vblanco20-1/vulkan-guide/blob/master/LICENSE.txt
 */

#pragma once
#include <deque>
#include <functional>

#include <vulkan/vulkan.h>
/**
 *
 */
struct DeletionQueue {
    /**
     * The function definition for the deletors
     * @param device A reference to the VK device used to delete the object
     */
    typedef std::function<void(VkDevice& device)> DeletorFunction;
public:
    /**
     * Push a deletor to the back of the queue
     * @param deletor The deletor used to cleanup the object
     */
    void pushDeletor(DeletorFunction&& deletor);

    /**
     * Execute all the deletors in the queue
     * @param device A reference to the VK device
     */
    void flush(VkDevice& device);
private:
    /**
     * The queue of deletors
     */
    std::deque<DeletorFunction> deletors;
};
