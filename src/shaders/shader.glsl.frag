R"(
#version 460 core

const uint GRID_SIZE = 32;

layout(std430, binding = 0) readonly buffer voxels_buffer {
    uint data[];    
} voxels;

uniform vec4 viewport;
uniform vec3 camera_position;
uniform mat4 camera_basis;

out vec4 out_color;

vec2 boxIntersection(vec3 ro, vec3 rd, vec3 boxSize) {
    vec3 m = 1.0/rd;
    vec3 n = m*ro;
    vec3 k = abs(m)*boxSize;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    float tN = max( max( t1.x, t1.y ), t1.z );
    float tF = min( min( t2.x, t2.y ), t2.z );
    if( tN>tF || tF<0.0) return vec2(-1.0);
    return vec2( tN, tF );
}

vec3 raycast(vec3 origin, vec3 direction) {
    vec2 intersection = boxIntersection(origin, direction, vec3(GRID_SIZE / 2.0));
    if (intersection.x < 0.0) {
        if (intersection.y >= 0.0) {
            intersection.x = 0.0;
        } else {
            return vec3(0.1);
        }
    }

    origin += direction * (intersection.x + 1e-4);
    ivec3 voxel = ivec3(floor(origin));
    ivec3 step = ivec3(sign(direction));
    vec3 distance = vec3(1e30);
    vec3 delta = vec3(1e30);
    for (uint i = 0; i < 3; i++) {
        if (direction[i] != 0.0) {
            float next = float(voxel[i]);
            if (direction[i] > 0.0) {
                next += 1.0;
            }

            distance[i] = (next - origin[i]) / direction[i];
            delta[i] = abs(1.0 / direction[i]);
        }
    }
    
    for (uint i = 0; i < 128; i++) {
        ivec3 mask = ivec3(0);
        if (distance.x <= distance.y && distance.x <= distance.z) {
            mask.x = 1;
        } else if (distance.y < distance.x && distance.y <= distance.z) {
            mask.y = 1;
        } else {
            mask.z = 1;
        }

        ivec3 index = voxel + ivec3(GRID_SIZE / 2);
        if (index.x < 0 || index.y < 0 || index.z < 0 || index.x >= GRID_SIZE || index.y >= GRID_SIZE || index.z >= GRID_SIZE) break;
        
        if (voxels.data[index.z * GRID_SIZE * GRID_SIZE + index.y * GRID_SIZE + index.x] > 0) {
            return vec3(index) / (GRID_SIZE - 1.0);
        }
        
        distance += delta * vec3(mask);
        voxel += step * mask;
    }
    
    return vec3(0.1);
}

void main() {
    vec2 ndc = (gl_FragCoord.xy / viewport.zw) * 2.0 - 1.0;
    ndc.x *= viewport.z / viewport.w;
    vec3 direction = normalize(vec3(ndc, -1.0));
    direction = normalize(vec3(camera_basis * vec4(direction, 1.0)));

    out_color = vec4(raycast(camera_position, direction), 1.0);
}
)"
