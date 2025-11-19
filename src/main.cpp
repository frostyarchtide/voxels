#define GLM_ENABLE_EXPERIMENTAL

#include <epoxy/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <chrono>
#include <vector>

#include "debug.hpp"

const unsigned int GRID_SIZE = 64;

void window_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    glfwSetErrorCallback(debug::glfw_error_callback);
    glfwInitHint(GLFW_WAYLAND_LIBDECOR, GLFW_WAYLAND_DISABLE_LIBDECOR);

    if (glfwInit() != GL_TRUE) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

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
    glDebugMessageCallback(debug::debug_message_callback, nullptr);
    debug::print_info((const char*) glGetString(GL_VERSION), "OpenGL Version");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");
    
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
    
    std::vector<unsigned int> voxels(GRID_SIZE * GRID_SIZE * GRID_SIZE);
    unsigned int voxel_count = 0;
    for (size_t z = 0; z < GRID_SIZE; z++) {
        for (size_t y = 0; y < GRID_SIZE; y++) {
            for (size_t x = 0; x < GRID_SIZE; x++) {
                bool value = (
                    std::pow(x - (float) GRID_SIZE / 2.0f, 2)
                    + std::pow(y - (float) GRID_SIZE / 2.0f, 2)
                    + std::pow(z - (float) GRID_SIZE / 2.0f, 2)
                ) < std::pow((float) GRID_SIZE / 2.0f - 1.0f, 2);
                voxels[z * GRID_SIZE * GRID_SIZE + y * GRID_SIZE + x] = value;
                if (value) voxel_count++;
            }
        }
    }
    voxels[0] = 1;
    voxels[GRID_SIZE * GRID_SIZE * GRID_SIZE - 1] = 1;
    
    unsigned int voxels_buffer;
    glGenBuffers(1, &voxels_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, voxels_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * GRID_SIZE * GRID_SIZE * GRID_SIZE, &voxels[0], GL_STATIC_READ);
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
    debug::check_shader(vertex_shader);

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);
    debug::check_shader(fragment_shader);

    unsigned int shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    
    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);

    glUseProgram(shader_program);
    
    std::chrono::high_resolution_clock::time_point last_time = std::chrono::high_resolution_clock::now();
    glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, (float) GRID_SIZE);
    glm::vec3 camera_rotation = glm::vec3(0.0f);

    while (!glfwWindowShouldClose(window)) {
        std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
        float delta = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_time).count();
        last_time = now;

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
            camera_rotation.x = glm::clamp(camera_rotation.x + 0.01f, -glm::half_pi<float>(), glm::half_pi<float>());
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN)) {
            camera_rotation.x = glm::clamp(camera_rotation.x - 0.01f, -glm::half_pi<float>(), glm::half_pi<float>());
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT)) {
            camera_rotation.y = std::fmod(camera_rotation.y + 0.01f, glm::tau<float>());
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
            camera_rotation.y = std::fmod(camera_rotation.y - 0.01f, glm::tau<float>());
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
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoDecoration
            | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::Begin("Statistics", nullptr, flags);
        ImGui::Text("%.2f ms", delta * 1000.0f);
        ImGui::Text("Grid Size: %u", GRID_SIZE);
        ImGui::Text("Voxels: %u", voxel_count);
        ImGui::Text("Camera Position: (x: %.2f, y: %.2f, z: %.2f)", camera_position.x, camera_position.y, camera_position.z);
        ImGui::Text("Camera Rotation: (x: %.2f, y: %.2f, z: %.2f)", camera_rotation.x, camera_rotation.y, camera_rotation.z);
        ImGui::End();
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shader_program);
    glDeleteBuffers(1, &voxels_buffer);
    glDeleteBuffers(1, &vertex_buffer);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}
