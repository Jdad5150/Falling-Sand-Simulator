#include "main.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <vector>
#include "implot.h"
#include "implot_internal.h"
#include <iomanip>
#include <string>



// Constants
int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
int GRID_WIDTH = 300;
int GRID_HEIGHT = 200;
bool isDragging = false;
// Element types
enum class ElementType { Air, Sand };

// Element structure
struct Element {
    ElementType type;
    glm::vec3 color;    
};

// Initialize the grid with air
std::vector<std::vector<Element>> grid(GRID_HEIGHT, std::vector<Element>(GRID_WIDTH, { ElementType::Air, {0.0f, 0.0f, 0.0f} }));

// Store GPU usage data for plotting
std::vector<float> gpuUsageData;

// Used to store framerate
////float values[90] = { 0 };
////float x_values[90] = { 0 };
////int values_offset = 0;

const int history_size = 90;
std::vector<float> framerate_values(history_size, 0.0f);
int values_offset = 0;

// Function prototypes
GLuint CompileShader(GLenum type, const char* source);
GLuint CreateShaderProgram();
void UpdateSimulation(std::vector<std::vector<Element>>& grid);
void DrawGrid(const std::vector<std::vector<Element>>& grid, GLuint shaderProgram);
void HandleMouseClick(double xpos, double ypos);
void HandleMouseDrag(double xpos, double ypos);
void HandleMouseErase(double xpos, double ypos);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void MouseMotionCallback(GLFWwindow* window, double xpos, double ypos);
GLFWwindow* InitFullScreenWindow();
void AdjustViewport(int width, int height);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void ConvertNormalizedToGrid(float normalizedX, float normalizedY, int& gridX, int& gridY);
float GetDeltaTime();


// Vertex Shader source code
const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec2 aPos;
    layout(location = 1) in vec3 aColor;
    out vec3 ourColor;
    uniform mat4 transform;
    void main() {
        gl_Position = transform * vec4(aPos, 0.0, 1.0);
        ourColor = aColor;
    }
)";

// Fragment Shader source code
const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 ourColor;
    out vec4 FragColor;
    void main() {
        FragColor = vec4(ourColor, 1.0f);
    }
)";


int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set GLFW window hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create GLFW window
    GLFWwindow* window = InitFullScreenWindow();

    // Set the call back keys to the window
    glfwSetKeyCallback(window, KeyCallback);

    //Make sure the window is initialized
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    //Make the current context, the window
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set viewport
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Compile and link shaders
    GLuint shaderProgram = CreateShaderProgram();

    // Initialize ImGui    
    IMGui::InitImGui(window);

    // Set mouse button callback
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    // Set mouse motion callback
    glfwSetCursorPosCallback(window, MouseMotionCallback);

    // Vertex data setup
    float vertices[] = {
        // Positions   // Colors
         1.0f, -1.0f,    1.0f, 0.0f, 0.0f, // Bottom-right
        -1.0f, -1.0f,    1.0f, 1.0f, 0.0f, // Bottom-left
        -1.0f,  1.0f,    1.0f, 0.0f, 0.0f,  // Top-left
         1.0f,  1.0f,    0.0f, 0.0f, 0.0f // Top-right        
    };
    

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

   

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        
        IMGui::processInput(window);

        ImGuiIO& io = ImGui::GetIO();

        if (!io.WantCaptureMouse)
        {
            // Update simulation
            UpdateSimulation(grid);
        }
        

        std::vector<float> GPUUsage;

        

        float CurrentGPUUsage = IMGui::GetGPUUsage();
        GPUUsage.push_back(CurrentGPUUsage + 1);
       
        //std::cout << std::fixed << std::setprecision(5) << CurrentGPUUsage << std::endl;

        

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);        

        /* Draw grid*/
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        DrawGrid(grid, shaderProgram);        

        glBindVertexArray(0);
        glUseProgram(0);        

        // Render ImGui
        IMGui::RenderPlot(GPUUsage, GRID_WIDTH,GRID_HEIGHT);

        

       ///* values[values_offset] = io.Framerate;
       // x_values[values_offset] = static_cast<float>(values_offset);
       // values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);*/

        framerate_values[values_offset] = io.Framerate;
        values_offset = (values_offset + 1) % history_size;
        

        IMGui::RenderPerformanceWindow(framerate_values, history_size);

        // Swap buffers
        glfwSwapBuffers(window);

        // Poll events
       glfwPollEvents();
    }


    // Cleanup
    IMGui::CleanupImGui();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

GLuint CreateShaderProgram() {
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void UpdateSimulation(std::vector<std::vector<Element>>& grid)
{
    std::vector<std::vector<bool>> hasMoved(GRID_HEIGHT, std::vector<bool>(GRID_WIDTH, false));

    for (int y = GRID_HEIGHT - 2; y >= 0; --y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            if (hasMoved[y][x]) continue;

            Element& currentElement = grid[y][x];

            //If Sand
            if (currentElement.type == ElementType::Sand) {
                // Check if the cell below is within bounds and empty
                if (y + 1 < GRID_HEIGHT && grid[y + 1][x].type == ElementType::Air && !hasMoved[y + 1][x])
                {
                    std::swap(currentElement, grid[y + 1][x]);
                    hasMoved[y + 1][x] = true;
                }
                // Check if the cell below is sand and try to move diagonally
                else if (y + 1 < GRID_HEIGHT && grid[y + 1][x].type == ElementType::Sand) {
                    if (x > 0 && grid[y + 1][x - 1].type == ElementType::Air && !hasMoved[y + 1][x - 1]) {
                        std::swap(currentElement, grid[y + 1][x - 1]);
                        hasMoved[y + 1][x - 1] = true;
                    }
                    else if (x < GRID_WIDTH - 1 && grid[y + 1][x + 1].type == ElementType::Air && !hasMoved[y + 1][x + 1]) {
                        std::swap(currentElement, grid[y + 1][x + 1]);
                        hasMoved[y + 1][x + 1] = true;
                    }
                }
            }
        }
    }  
}

void DrawGrid(const std::vector<std::vector<Element>>& grid, GLuint shaderProgram)
{
    glUseProgram(shaderProgram);
    GLuint transformLoc = glGetUniformLocation(shaderProgram, "transform");

    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            if (grid[y][x].type == ElementType::Air) continue;

            glm::mat4 transform = glm::mat4(1.0f);

            // Map grid coordinates to normalized device coordinates
            transform = glm::translate(transform, glm::vec3((x + 0.5f) / float(GRID_WIDTH) * 2.0f - 1.0f,
                (GRID_HEIGHT - (y + 0.5f)) / float(GRID_HEIGHT) * 2.0f - 1.0f,
                0.0f));
            transform = glm::scale(transform, glm::vec3(1.0f / GRID_WIDTH, 1.0f / GRID_HEIGHT, 1.0f));

            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

            GLuint colorLoc = glGetUniformLocation(shaderProgram, "aColor");
            glUniform3fv(colorLoc, 1, glm::value_ptr(grid[y][x].color));

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Assuming you are using 4 vertices for a grid cell
        }
    }
}

void HandleMouseClick(double xpos, double ypos)
{
    // Convert screen coordinates to normalized device coordinates
    float normalizedX = (xpos / WINDOW_WIDTH) * 2.0f - 1.0f; // [-1, 1]
    float normalizedY = 1.0f - (ypos / WINDOW_HEIGHT) * 2.0f;

    // Map normalized coordinates to grid coordinates
    int gridX = static_cast<int>((normalizedX + 1.0f) * 0.5f * GRID_WIDTH);
    int gridY = static_cast<int>((1.0f - normalizedY) * 0.5f * GRID_HEIGHT);


    if (gridX >= 0 && gridX < GRID_WIDTH && gridY >= 0 && gridY < GRID_HEIGHT) {
        grid[gridY][gridX] = { ElementType::Sand, {1.0f, 0.85f, 0.55f} };       
    }
}

void HandleMouseDrag(double xpos, double ypos) {
    int gridX = static_cast<int>((xpos / WINDOW_WIDTH) * GRID_WIDTH);
    int gridY = static_cast<int>(((WINDOW_HEIGHT - ypos) / WINDOW_HEIGHT) * GRID_HEIGHT);

    if (gridX >= 0 && gridX < GRID_WIDTH && gridY >= 0 && gridY < GRID_HEIGHT) {
        grid[gridY][gridX] = { ElementType::Sand, {1.0f, 0.85f, 0.55f} };
    }
}

void HandleMouseErase(double xpos, double ypos) {
    int gridX = static_cast<int>((xpos / WINDOW_WIDTH) * GRID_WIDTH);
    int gridY = static_cast<int>(((WINDOW_HEIGHT - ypos) / WINDOW_HEIGHT) * GRID_HEIGHT);

    if (gridX >= 0 && gridX < GRID_WIDTH && gridY >= 0 && gridY < GRID_HEIGHT) {
        grid[gridY][gridX] = { ElementType::Air, {0.0f, 0.0f, 0.0f} };
    }
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            isDragging = true;
        }
        else if (action == GLFW_RELEASE) {
            isDragging = false;
        }
    }
}

void MouseMotionCallback(GLFWwindow* window, double xpos, double ypos) {
    if (isDragging) {
        // Convert window coordinates to normalized device coordinates
        float normalizedX = (xpos / WINDOW_WIDTH) * 2.0f - 1.0f;
        float normalizedY = 1.0f - (ypos / WINDOW_HEIGHT) * 2.0f;

        int gridX, gridY;
        ConvertNormalizedToGrid(normalizedX, normalizedY, gridX, gridY);

        // Check bounds and add sand to a 3x3 area
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int newGridX = gridX + dx;
                int newGridY = gridY + dy;

                if (newGridX >= 0 && newGridX < GRID_WIDTH && newGridY >= 0 && newGridY < GRID_HEIGHT) {
                    grid[newGridY][newGridX] = { ElementType::Sand };
                }
            }
        }
    }   
}


GLFWwindow* InitFullScreenWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Get the primary monitor
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (!monitor) {
        std::cerr << "Failed to get primary monitor" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Get the video mode of the monitor
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (!mode) {
        std::cerr << "Failed to get video mode" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Create a full-screen window
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Falling Sand Simulator", monitor, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    AdjustViewport(mode->width, mode->height);
    return window;
}

void AdjustViewport(int width, int height) {
    glViewport(0, 0, width, height);
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void ConvertNormalizedToGrid(float normalizedX, float normalizedY, int& gridX, int& gridY)
{
    gridX = static_cast<int>((normalizedX + 1.0f) * 0.5f * GRID_WIDTH);
    gridY = static_cast<int>((1.0f - normalizedY) * 0.5f * GRID_HEIGHT);
}

float GetDeltaTime()
{
    static float lastTime = 0.0f;
    float currentTime = static_cast<float>(glfwGetTime());
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    return deltaTime;    
}
