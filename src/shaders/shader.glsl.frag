R"(
#version 460 core

const float EPSILON = 1.0e-3;
const float INFINITY = 1.0e+30;
const uint GRID_SIZE = 32;
const vec3 LIGHT_DIRECTION = normalize(vec3(1.0, 0.5, 1.0));
const float AMBIENT_LIGHT = 0.1;
const ivec3 DIRECTIONS[] = ivec3[](
    ivec3(0, 1, 0),
    ivec3(0, -1, 0),
    ivec3(-1, 0, 0),
    ivec3(1, 0, 0),
    ivec3(0, 0, -1),
    ivec3(0, 0, 1)
);
ivec3 NEIGHBORS[][] = ivec3[][](
    ivec3[]( ivec3(-1, 0, 0), ivec3(1, 0, 0), ivec3(0, 0, -1), ivec3(0, 0, 1) ), // ivec3(0, 1, 0)
    ivec3[]( ivec3(-1, 0, 0), ivec3(1, 0, 0), ivec3(0, 0, -1), ivec3(0, 0, 1) ), // ivec3(0, -1, 0)
    ivec3[]( ivec3(0, 1, 0), ivec3(0, -1, 0), ivec3(0, 0, -1), ivec3(0, 0, 1) ), // ivec3(-1, 0, 0)
    ivec3[]( ivec3(0, 1, 0), ivec3(0, -1, 0), ivec3(0, 0, -1), ivec3(0, 0, 1) ), // ivec3(1, 0, 0)
    ivec3[]( ivec3(-1, 0, 0), ivec3(1, 0, 0), ivec3(0, 1, 0), ivec3(0, -1, 0) ), // ivec3(0, 0, -1)
    ivec3[]( ivec3(-1, 0, 0), ivec3(1, 0, 0), ivec3(0, 1, 0), ivec3(0, -1, 0) )  // ivec3(0, 0, 1)
);

struct IntersectionInfo {
    float start;
    float end;
    vec3 normal;
};

struct HitInfo {
    ivec3 voxel;
    vec3 normal;
    float distance;
};

layout(std430, binding = 0) readonly buffer Voxels {
    uint data[];    
} voxels;

uniform vec4 viewport;
uniform vec3 camera_position;
uniform mat4 camera_basis;

out vec4 out_color;

// https://iquilezles.org/articles/intersectors/
// axis aligned box centered at the origin, with size boxSize
IntersectionInfo boxIntersection( in vec3 ro, in vec3 rd, vec3 boxSize ) 
{
    vec3 m = 1.0/rd; // can precompute if traversing a set of aligned boxes
    vec3 n = m*ro;   // can precompute if traversing a set of aligned boxes
    vec3 k = abs(m)*boxSize;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    float tN = max( max( t1.x, t1.y ), t1.z );
    float tF = min( min( t2.x, t2.y ), t2.z );
    if( tN>tF || tF<0.0) return IntersectionInfo(-1.0, -1.0, vec3(-1.0)); // no intersection
    vec3 outNormal = (tN>0.0) ? step(vec3(tN),t1) : // ro ouside the box
                           step(t2,vec3(tF));  // ro inside the box
    outNormal *= -sign(rd);
    return IntersectionInfo(
        tN,
        tF,
        outNormal
    );
}

bool is_voxel(ivec3 index) {
    if (index.x < 0 || index.y < 0 || index.z < 0 || index.x >= GRID_SIZE || index.y >= GRID_SIZE || index.z >= GRID_SIZE) return false;
    return voxels.data[index.z * GRID_SIZE * GRID_SIZE + index.y * GRID_SIZE + index.x] > 0;
}

HitInfo raycast(vec3 origin, vec3 direction) {
    IntersectionInfo intersection = boxIntersection(origin, direction, vec3(GRID_SIZE / 2.0));
    if (intersection.start < 0.0) {
        if (intersection.end >= 0.0) {
            intersection.start = 0.0;
        } else {
            return HitInfo(
                ivec3(-1),
                vec3(0.0),
                -1.0
            );
        }
    }

    ivec3 voxel = ivec3(floor(origin + direction * (intersection.start + EPSILON)));
    ivec3 step_direction = ivec3(sign(direction));
    vec3 distance = vec3(INFINITY);
    vec3 delta = vec3(INFINITY);
    for (uint i = 0; i < 3; i++) {
        if (direction[i] != 0.0) {
            float next = float(voxel[i]) + step(0.0, direction[i]);

            distance[i] = (next - origin[i]) / direction[i];
            delta[i] = abs(1.0 / direction[i]);
        }
    }
    
    float total_distance = intersection.start;
    vec3 normal = intersection.normal;
    
    for (uint i = 0; i < GRID_SIZE; i++) {
        ivec3 index = voxel + ivec3(GRID_SIZE / 2);
        if (is_voxel(index)) {
            return HitInfo(
                index,
                normal,
                total_distance
            );
        }

        float minimum = min(min(distance.x, distance.y), distance.z);
        vec3 mask = vec3(
            distance.x == minimum,
            distance.y == minimum,
            distance.z == minimum
        );
        
        voxel += step_direction * ivec3(mask);
        distance += delta * mask;
        total_distance += minimum - total_distance;
        normal = -step_direction * mask;
    }
    
    return HitInfo(
        ivec3(-1),
        vec3(0.0),
        -1.0
    );
}

void main() {
    vec2 ndc = (gl_FragCoord.xy / viewport.zw) * 2.0 - 1.0;
    ndc.x *= viewport.z / viewport.w;
    vec3 direction = normalize(vec3(ndc, -1.0));
    direction = normalize(vec3(camera_basis * vec4(direction, 1.0)));
    
    vec3 color = vec3(0.0);

    HitInfo hit_info = raycast(camera_position, direction);
    // vec3 hit_position = camera_position + direction * hit_info.distance;

    if (hit_info.voxel != ivec3(-1)) {
        color = vec3(hit_info.voxel) / GRID_SIZE;
        // float light = dot(hit_info.normal, LIGHT_DIRECTION);

        // uint neighbor = 0;
        // while (neighbor < 6) {
        //     if (DIRECTIONS[neighbor] == ivec3(hit_info.normal)) {
        //         break;
        //     } else {
        //         neighbor++;
        //     }
        // }
        // 
        // ivec3 normal_voxel = hit_info.voxel + ivec3(hit_info.normal);
        // float ambient_occlusion = 0.0;
        // for (uint i = 0; i < 4; i++) {
        //     ivec3 offset = NEIGHBORS[neighbor][i];
        //     if (is_voxel(normal_voxel + offset)) {
        //         vec3 hit_masked = hit_position * offset;
        //         float distance = 1.0 - length(fract(hit_masked));
        //         
        //         for (uint j = 0; j < 4; j++) {
        //             ivec3 other_offset = NEIGHBORS[neighbor][j];
        //             if (j == i || abs(other_offset) == abs(offset)) continue;

        //             if (!is_voxel(hit_info.voxel + other_offset) || !is_voxel(normal_voxel + offset + other_offset)) {
        //                 hit_masked = hit_position * other_offset;
        //                 distance /= clamp(1.0 - length(fract(hit_masked)), EPSILON, 2.0);
        //             }
        //         }

        //         ambient_occlusion = clamp(ambient_occlusion + smoothstep(0.25, 0.0, distance) * 0.25, 0.0, 1.0);
        //     }
        // }

        // hit_info = raycast(camera_position + direction * hit_info.distance, LIGHT_DIRECTION);
        // if (hit_info.voxel != ivec3(-1)) {
        //     light = AMBIENT_LIGHT;
        // }
        // 
        // light *= 1.0 - ambient_occlusion;
        // color *= light;
    } else {
        // float value = dot(direction, LIGHT_DIRECTION);
        // color = vec3(smoothstep(0.95, 0.96, pow(value, 3.0)));
    }

    out_color = vec4(color, 1.0);
}
)"
