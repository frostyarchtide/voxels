#pragma once

#include <string>

namespace debug {
    void print_info(const std::string& message, const std::string& header = "Info");
    void print_warning(const std::string& message, const std::string& header = "Warning");
    void print_error(const std::string& message, bool panic = true, const std::string& header = "Error");
    void glfw_error_callback(int error, const char* description);
    void debug_message_callback(
        unsigned int source,
        unsigned int type,
        unsigned intid,
        unsigned int severity,
        int length,
        const char* message,
        const void* user_param
    );
    void check_shader(unsigned int shader);
}