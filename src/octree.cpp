#include "octree.hpp"

Octree::Octree() {
    nodes.emplace_back(Node {
        glm::vec3(0.0f),
        glm::vec3(2.0f),
        0
    });

    for (size_t i = 0; i < 8; i++) {
        voxels.emplace_back(true);
    }
}

const std::vector<bool>& Octree::get_voxels() const {
    return voxels;
}
