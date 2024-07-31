#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <vector>
#include "implot.h"
#include "implot_internal.h"
#include <queue>
#include <nvml.h>
#include <chrono>


class IMGui
{
private:
	static inline bool isGatheringData = false;


public:
	// Initialize ImGui and set styles
	static void InitImGui(GLFWwindow* window); 
	// Used to process input, mouse/keyboard
	static void processInput(GLFWwindow* window);
	// Main Function used to render all ImGui and ImPlot UI
	static void RenderUI(int& GRID_WIDTH, int& GRID_HEIGHT, std::vector<double>& GpuData, std::vector<double>& TimeData);
	// Functions used to create widgets and render Controls
	static void RenderControlsWindow(int& GRID_WIDTH, int& GRID_HEIGHT);
	static void SetWindowSizeComboBox(int& GRID_WIDTH, int& GRID_HEIGHT);
	// Functions used to gather data, create widgets and render data 
	static void RenderPerformanceWindow(std::vector<double>& GpuData, std::vector<double>& TimeData);
	static bool GatherData();
	static double GetGPUUsage();
	static void CreateGPUGraph(std::vector<double>& GpuData, std::vector<double>& TimeData);
	// Cleanup all ImGui 
	static void CleanupImGui();
	
};
