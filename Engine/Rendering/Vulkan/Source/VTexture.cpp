//
// Created by jacob on 27/02/23.
//

#include "Rendering/Vulkan/VTexture.h"

VTexture::VTexture(const AllocatedImage& image, VkImageView imageView) : image(image), imageView(imageView) {}
