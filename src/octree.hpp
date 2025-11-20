#pragma once

#include <glm/vec3.hpp>

#include <vector>

struct Node {
    glm::vec3 position;
    glm::vec3 size;
    size_t voxel_index;
};

struct Octree {
public:
    Octree();

    const std::vector<bool>& get_voxels() const;

private:
    std::vector<Node> nodes;
    std::vector<bool> voxels;
};
