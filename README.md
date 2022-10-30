# leicester-engine
My 3rd year project at The University of Leicester. A 3d game engine, specifically implementing collision and PBR Rendering.

# Libraries Used
The following libraries have been used to speed up development and fill in boilerplate
 - [GLM](https://github.com/g-truc/glm) - A maths library that closely follows GLSL in syntax
 - [GLFW](https://www.glfw.org/) - A multiplatform library for handling windows, inputs, and related events
 - [Vulkan SDK](https://www.vulkan.org/) - The graphics API used for rendering in this project
 - [VK-Bootstrap](https://github.com/charles-lunarg/vk-bootstrap) - A utility library to simplify a lot of the 
boilerplate involved in setting up vulkan 
 - [Vulkan Memory Manager](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) - A library to help with 
all the boilerplate involved in managing GPU memory

With the exception of VK-Bookstrap and Vulkan Memory Manager, these libraries are loaded using CMake's `find_package` directive, thus must be
installed on the system building the project.
VK-Boostrap and Vulkan Memory Manager are loaded as git submodules.