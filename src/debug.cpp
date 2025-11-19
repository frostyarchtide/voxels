#if defined(_WIN32) || defined(_WIN64)
    #include <intrin.h>
    #define panic() __debugbreak()
#else
    #include <csignal>
    #define panic() raise(SIGBREAK)
#endif

#include <epoxy/gl.h>

#include <iostream>

#include "debug.hpp"

void debug::print_info(const std::string& message, const std::string& header) {
    std::cout << "\033[90m" << "[" << header << "] " << message << "\033[0m" << std::endl; 
}

void debug::print_warning(const std::string& message, const std::string& header) {
    std::cout << "\033[93m" << "[" << header << "] " << message << "\033[0m" << std::endl; 
}

void debug::print_error(const std::string& message, bool panic, const std::string& header) {
    std::cout << "\033[91m" << "[" << header << "] " << message << "\033[0m" << std::endl; 
    if (panic) panic();
}

void debug::debug_message_callback(
    unsigned int source,
    unsigned int type,
    unsigned intid,
    unsigned int severity,
    int length,
    const char* message,
    const void* user_param
) {
    debug::print_error(message, true, "OpenGL Error");
}

void debug::check_shader(unsigned int shader) {
    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        int max_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

        std::string error_log(max_length, '\0');
        glGetShaderInfoLog(shader, max_length, &max_length, &error_log[0]);
        error_log.resize(max_length);
        error_log.erase(error_log.find_last_not_of(" \r\n"));

        debug::print_warning(error_log, "Shader Error");
    }
}