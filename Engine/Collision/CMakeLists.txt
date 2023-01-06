cmake_minimum_required(VERSION 3.22)

target_sources(leicester-engine PRIVATE
        Source/Collider.cpp
        Source/AABBCollider.cpp
        Source/SphereCollider.cpp
        Source/MeshCollider.cpp
        Source/CollisionEngine.cpp
        Source/GJKCollisionEngine.cpp
)