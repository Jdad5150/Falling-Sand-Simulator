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
	int MAX_FRAMES = 100;
	float frameTimes[100];
	int frameIndex = 0;
	float frameTimeSum = 0.0f;


public:
	static void InitImGui(GLFWwindow* window);
	static void RenderPlot(std::vector<float>& x_data);
	static void CleanupImGui();
	static void processInput(GLFWwindow* window);
	static float GetGPUUsage();
	static void CreateGPUGraph(std::vector<float>& GPUUsageData);
	static double CalculateFPS();
	static void AddFPSValue(std::vector<double> FPS, double newFPS);
	static void CreateFPSGraph();
	void UpdateFrameTimes(float deltaTime);
	static float GetFPS();
};
