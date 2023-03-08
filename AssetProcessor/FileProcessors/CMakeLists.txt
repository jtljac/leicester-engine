cmake_minimum_required(VERSION 3.22)

target_sources(asset-processor PRIVATE
        Source/ObjProcessor.cpp
        Source/ImgProcessor.cpp
        Source/ShaderProcessor.cpp
        Source/CopyProcessor.cpp
)