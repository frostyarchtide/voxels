#define GLM_ENABLE_EXPERIMENTAL

#include <epoxy/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <iostream>

void debug_message_callback(
    unsigned int source,
    unsigned int type,
    unsigned intid,
    unsigned int severity,
    int length,
    const char* message,
    const void* user_param
) {
    std::cout << "[OpenGL Error] " << message << std::endl;
}

void check_shader(unsigned int shader) {
    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        int max_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

        char* error_log = (char*) malloc(max_length);
        glGetShaderInfoLog(shader, max_length, &max_length, error_log);

        std::cout << "[Shader Error] " << error_log << std::endl;

        free((void*) error_log);
    }
}

void window_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    glfwInitHint(GLFW_WAYLAND_LIBDECOR, GLFW_WAYLAND_DISABLE_LIBDECOR);

    if (glfwInit() != GL_TRUE) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Voxels", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSwapInterval(1);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debug_message_callback, nullptr);
    
    unsigned int vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    float vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };

    unsigned int vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    
    unsigned int voxels[32 * 32 * 32];
    for (size_t z = 0; z < 32; z++) {
        for (size_t y = 0; y < 32; y++) {
            for (size_t x = 0; x < 32; x++) {
                voxels[z * 32 * 32 + y * 32 + x] = (
                    std::pow(x - 16.0f, 2)
                    + std::pow(y - 16.0f, 2)
                    + std::pow(z - 16.0f, 2)
                ) < std::pow(16.0f, 2);
            }
        }
    }
    voxels[0] = 1;
    voxels[32 * 32 * 32 - 1] = 1;
    
    unsigned int voxels_buffer;
    glGenBuffers(1, &voxels_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, voxels_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(voxels), voxels, GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, voxels_buffer);

    const char* vertex_shader_source =
    #include "shaders/shader.glsl.vert"
    ;

    const char* fragment_shader_source =
    #include "shaders/shader.glsl.frag"
    ;

    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);
    check_shader(vertex_shader);

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);
    check_shader(fragment_shader);

    unsigned int shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    
    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);

    glUseProgram(shader_program);
    
    glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, 30.0f);
    glm::vec3 camera_rotation = glm::vec3(0.0f);

    while (!glfwWindowShouldClose(window)) {
        glm::mat4 camera_basis = glm::rotate(glm::rotate(glm::identity<glm::mat4>(), camera_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)), camera_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));

        if (glfwGetKey(window, GLFW_KEY_W)) {
            camera_position += glm::rotateY(glm::vec3(0.0f, 0.0f, -0.1f), camera_rotation.y);
        }
        if (glfwGetKey(window, GLFW_KEY_S)) {
            camera_position += glm::rotateY(glm::vec3(0.0f, 0.0f, 0.1f), camera_rotation.y);
        }
        if (glfwGetKey(window, GLFW_KEY_A)) {
            camera_position += glm::rotateY(glm::vec3(-0.1f, 0.0f, 0.0f), camera_rotation.y);
        }
        if (glfwGetKey(window, GLFW_KEY_D)) {
            camera_position += glm::rotateY(glm::vec3(0.1f, 0.0f, 0.0f), camera_rotation.y);
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE)) {
            camera_position.y += 0.1f;
        }
        if (glfwGetKey(window, GLFW_KEY_C)) {
            camera_position.y -= 0.1f;
        }
        if (glfwGetKey(window, GLFW_KEY_UP)) {
            camera_rotation.x += 0.01f;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN)) {
            camera_rotation.x -= 0.01f;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT)) {
            camera_rotation.y += 0.01f;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
            camera_rotation.y -= 0.01f;
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int width;
        int height;
        glfwGetWindowSize(window, &width, &height);
        glUniform4f(glGetUniformLocation(shader_program, "viewport"), 0.0f, 0.0f, (float) width, (float) height);

        glUniform3fv(glGetUniformLocation(shader_program, "camera_position"), 1, &camera_position.x);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "camera_basis"), 1, GL_FALSE, &camera_basis[0][0]);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shader_program);
    glDeleteBuffers(1, &vertex_buffer);
    glfwDestroyWindow(window);
    glfwTerminate();
}
