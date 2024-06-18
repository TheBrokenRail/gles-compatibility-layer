#include <cstdio>
#include <cstdlib>

#include <string>
#include <vector>
#include <fstream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <GLES/gl.h>

#define INFO(format, ...) \
    { \
        fprintf(stderr, "[INFO]: " format "\n", ##__VA_ARGS__); \
    }
#define WARN(format, ...) \
    { \
        fprintf(stderr, "[WARN]: " format "\n", ##__VA_ARGS__); \
    }
#define ERR(format, ...) \
    { \
        fprintf(stderr, "[ERR]: (%s:%i): " format "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        exit(EXIT_FAILURE); \
    }

// Tests
#define MAX_TESTS 255
static const char *tests[MAX_TESTS] = { 0 };
extern "C" void add_test(const char *function_name) {
    static int i = 0;
    if (i >= MAX_TESTS) {
        ERR("Too Many Tests!");
    }
    tests[i++] = function_name;
}

// Header Files
static std::vector<std::string> header_lines;
static void load_header(std::string filename) {
    std::ifstream file(filename);
    if (!file.good()) {
        ERR("Unable To Load Header: %s", filename.c_str());
    }
    std::string line;
    while (std::getline(file, line)) {
        header_lines.push_back(line);
    }
    file.close();
}
static void load_headers() {
    header_lines.clear();
    load_header("/usr/include/GLES2/gl2.h");
}

// Run Test
static void run_test(std::string function_name) {
    // Log
    INFO("Checking %s...", function_name.c_str());

    // Search Headers
    std::string check_str = " " + function_name + " ";
    bool found = false;
    for (std::string line : header_lines) {
        if (line.find(check_str) != std::string::npos) {
            // Found
            found = true;
            break;
        }
    }
    if (!found) {
        ERR("Unable To Find Function");
    }
}

// Handle GLFW Error
static void glfw_error(__attribute__((unused)) int error, const char *description) {
    WARN("GLFW Error: %s", description);
}

int main() {
    INFO("Starting Test");

    // Init GLFW
    glfwSetErrorCallback(glfw_error);
    if (!glfwInit()) {
        ERR("Unable To Initialize GLFW");
    }

    // Create OpenGL ES Context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#ifdef GLES_COMPATIBILITY_LAYER_USE_ES3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
#endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create Window
    GLFWwindow *glfw_window = glfwCreateWindow(640, 480, "Test Window", NULL, NULL);
    if (!glfw_window) {
        ERR("Unable To Create GLFW Window");
    }

    // Make Window Context Current
    glfwMakeContextCurrent(glfw_window);

    // Setup Compatibility Layer
    init_gles_compatibility_layer((getProcAddress_t) glfwGetProcAddress);

    // Run Tests
    load_headers();
    for (int i = 0; tests[i] != NULL; i++) {
        run_test(tests[i]);
    }

    // Exit
    glfwDestroyWindow(glfw_window);
    glfwTerminate();
    INFO("Test Complete");
}
