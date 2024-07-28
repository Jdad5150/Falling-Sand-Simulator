#include "IMGui.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include <GLFW/glfw3.h>

void IMGui::InitImGui(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(15, 15);
    style.WindowRounding = 5.0f;
    style.FramePadding = ImVec2(5, 5);
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 15.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 5.0f;
    style.GrabRounding = 3.0f;

    ImGuiIO& io = ImGui::GetIO();

    // Enable keyboard controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Set display size (optional, if you want to specify manually)
    io.DisplaySize = ImVec2(800.0f, 600.0f); // Match with your window size



    // Set mouse sensitivity and speed (optional)
    io.MouseDoubleClickTime = 0.30f;
    io.MouseDoubleClickMaxDist = 6.0f;

}

void IMGui::RenderPlot(std::vector<float>& x_data, int& GRID_WIDTH, int& GRID_HEIGHT)
{
    //create a new ImGui Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //Set the size
    ImGui::SetNextWindowSize(ImVec2(550, 150), ImGuiCond_FirstUseEver);
    

    //Begin the Frame
    ImGui::Begin("Tools");

    //Add Text
    ImGui::Text("Change the size of the grid.");
    
    //Create Grid Size combo box
    SetWindowSizeComboBox(GRID_WIDTH, GRID_HEIGHT);

    // Debugging: Show IO values
    ImGuiIO& io = ImGui::GetIO();

    // Scale the font bigger
    io.FontGlobalScale = 1.5f;

    ImGui::Text("Framerate: %.1f FPS", io.Framerate);

    //End the frame and render
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void IMGui::CleanupImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

void IMGui::processInput(GLFWwindow* window)
{
    ImGuiIO& io = ImGui::GetIO();

    // Capture keyboard inputs
    io.KeysDown[GLFW_KEY_ESCAPE] = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    io.KeysDown[GLFW_KEY_SPACE] = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    // Capture mouse position
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    io.MousePos = ImVec2(static_cast<float>(mouseX), static_cast<float>(mouseY));

    // Capture mouse buttons
    io.MouseDown[0] = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    io.MouseDown[1] = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    // Close the window if Escape is pressed
    if (io.KeysDown[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);
}

float IMGui::GetGPUUsage()
{
    nvmlReturn_t result;
    unsigned int deviceCount;
    nvmlDevice_t device;
    nvmlUtilization_t utilization;

    // Initialize NVML
    result = nvmlInit();
    if (result != NVML_SUCCESS) {
        std::cerr << "Failed to initialize NVML: " << nvmlErrorString(result) << std::endl;
        return 0.0f;
    }

    // Get the number of NVIDIA GPUs
    result = nvmlDeviceGetCount(&deviceCount);
    if (result != NVML_SUCCESS) {
        std::cerr << "Failed to get device count: " << nvmlErrorString(result) << std::endl;
        nvmlShutdown();
        return 0.0f;
    }

    // For simplicity, we assume only one GPU is present
    result = nvmlDeviceGetHandleByIndex(0, &device);
    if (result != NVML_SUCCESS) {
        std::cerr << "Failed to get device handle: " << nvmlErrorString(result) << std::endl;
        nvmlShutdown();
        return 0.0f;
    }

    // Get GPU utilization
    result = nvmlDeviceGetUtilizationRates(device, &utilization);
    if (result != NVML_SUCCESS) {
        std::cerr << "Failed to get utilization rates: " << nvmlErrorString(result) << std::endl;
        nvmlShutdown();
        return 0.0f;
    }

    // Shutdown NVML
    nvmlShutdown();

    // Return GPU utilization percentage
    return static_cast<float>(utilization.gpu);
    
}

void IMGui::CreateGPUGraph(std::vector<float>& GPUUsageData)
{
    if (ImPlot::BeginPlot("GPU Usage (%)"))
    {
        ImPlot::PlotLine("Usage", GPUUsageData.data(), GPUUsageData.size());

        ImPlot::EndPlot();
    }
}



void IMGui::SetWindowSizeComboBox(int& GRID_WIDTH, int& GRID_HEIGHT)
{
    const char* windowSizes[] =
    {
        "30 x 30",
        "200 x 150",
        "300 x 200"
    };

    static const char* currentSize = windowSizes[2];
    static int selectedIndex = -1;

    if (ImGui::BeginCombo("Grid Size", currentSize))
    {
        for (int i = 0; i < IM_ARRAYSIZE(windowSizes); ++i)
        {
            bool isSelected = (currentSize == windowSizes[i]);

            if (ImGui::Selectable(windowSizes[i], isSelected))
            {
                currentSize = windowSizes[i];
                selectedIndex = i;                
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    switch (selectedIndex)
    {
    case 0:
        GRID_WIDTH = 30;
        GRID_HEIGHT = 30;
        break;
        
    case 1:
        GRID_WIDTH = 200;
        GRID_HEIGHT = 150;
        break;
    case 2:
        GRID_WIDTH = 300;
        GRID_HEIGHT = 200;
        break;

    default:
        break;
    }

}



void IMGui::RenderPerformanceWindow(std::vector<float> framerate_values, int history_size)
{
    

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();    

    ImGui::SetNextWindowSize(ImVec2(550, 150), ImGuiCond_FirstUseEver);
    ImPlot::SetNextAxesLimits(0.0, 100.0, 0, 200);

    ImGui::Begin("Performance");
    ImGui::Text("Framerate: %.1f FPS", io.Framerate);

    if (ImPlot::BeginPlot("Framerate Plot"))
    {
        ImPlot::PlotLine("Framerate", framerate_values.data(), history_size);
        ImPlot::EndPlot();
    }
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
