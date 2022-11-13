//
// Created by jacob on 09/11/22.
//

#pragma once

#include <type_traits>
#include <unordered_map>

template<typename KeyType, typename ResourceClass, typename std::enable_if_t<std::is_integral<KeyType>::value>* = nullptr>
class IDTrackedResource {
    KeyType nextKey;
    std::unordered_map<KeyType, ResourceClass> resourceMap;

public:
    IDTrackedResource() = default;

    KeyType insert(ResourceClass resource) {
        resourceMap.emplace(nextKey, resource);
        return nextKey++;
    }
    ResourceClass get(KeyType key) {
        return resourceMap.at(key);
    }
    void remove(KeyType key) {
        resourceMap.erase(key);
    }
    void clear() {
        nextKey = 0;
        resourceMap.clear();
    }

    std::unordered_map<KeyType, ResourceClass> getMap() {
        return resourceMap;
    }
};
