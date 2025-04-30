#pragma once

#include "../config.hpp"
#include "../system_monitor.hpp"
#include "widgets.hpp"
#include "fonts.hpp"
#include <imgui.h>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <lmcons.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#endif

class StatusBar {
public:
    StatusBar()
        : lastUpdateTime(std::chrono::steady_clock::now())
        , updateInterval(1000)
    {
        // Get the username once during initialization
        getCurrentUsername();
        updateCurrentTime();
    }

    void render() {
        ImGuiIO& io = ImGui::GetIO();

        // Get the instance of SystemMonitor
        SystemMonitor& sysMonitor = SystemMonitor::getInstance();
        sysMonitor.update();

        // Only update metrics occasionally to reduce CPU impact
        auto currentTime = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime).count() > updateInterval) {
            // Update SystemMonitor to get fresh stats
            sysMonitor.update();
            updateCurrentTime();
            lastUpdateTime = currentTime;
        }

        // Calculate status bar position and size
        ImVec2 windowPos(0, io.DisplaySize.y - Config::FOOTER_HEIGHT);
        ImVec2 windowSize(io.DisplaySize.x, Config::FOOTER_HEIGHT);

        // Begin a window with specific position and size
        ImGui::SetNextWindowPos(windowPos);
        ImGui::SetNextWindowSize(windowSize);

        // Status bar window flags
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        // Minimal styling
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.4f));

        // Helper function to format memory size
        auto formatMemory = [](size_t memorySizeMB) -> std::string {
            if (memorySizeMB >= 1024) {
                // Convert to GB with 2 decimal places
                double memorySizeGB = memorySizeMB / 1024.0;
                std::stringstream ss;
                ss << std::fixed << std::setprecision(2) << memorySizeGB;
                return ss.str() + " GB";
            }
            else {
                return std::to_string(memorySizeMB) + " MB";
            }
            };

        if (ImGui::Begin("##StatusBar", nullptr, window_flags)) {
            // Left side: Version
            LabelConfig versionLabel;
            versionLabel.id = "##versionLabel";
            versionLabel.label = "Version: " + std::string(APP_VERSION);
            versionLabel.size = ImVec2(200, 20);
            versionLabel.fontSize = FontsManager::SM;

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 10);

            Label::render(versionLabel);

            ImGui::SameLine();

            // Get metrics from SystemMonitor
            float cpuUsage = sysMonitor.getCpuUsagePercentage();
            size_t memoryUsageMB = sysMonitor.getUsedMemoryByProcess() / (1024 * 1024);
            size_t memoryTotalMB = sysMonitor.getTotalSystemMemory() / (1024 * 1024);

            // Format the CPU usage with one decimal place
            std::stringstream cpuSS;
            cpuSS << std::fixed << std::setprecision(1) << cpuUsage;

            // Right-align the time display
            float contentWidth = ImGui::GetContentRegionAvail().x;
            float timeWidth = 180;  // Approximate width needed for time display

            std::vector<ButtonConfig> buttonConfigs;

            // Get font scale factor from FontsManager
            float fontScale = FontsManager::GetInstance().GetTotalScaleFactor();

            // Format font scale with one decimal place
            std::stringstream fontSS;
            fontSS << std::fixed << std::setprecision(1) << fontScale << "x";

            // Create font scale button
            ButtonConfig fontScaleLabel;
            fontScaleLabel.id = "##fontScaleLabel";
            fontScaleLabel.label = "Zoom : " + fontSS.str();
            fontScaleLabel.size = ImVec2(110, 20);
            fontScaleLabel.fontSize = FontsManager::SM;

            buttonConfigs.push_back(fontScaleLabel);
            timeWidth += 120; // Add width for the font scale display

            if (sysMonitor.hasGpuSupport()) {
                ButtonConfig gpuLabel;
                gpuLabel.id = "##gpuLabel";
                gpuLabel.label = "Using " + sysMonitor.getGpuName();
                gpuLabel.size = ImVec2(300, 20);
                gpuLabel.fontSize = FontsManager::SM;

                buttonConfigs.push_back(gpuLabel);

                timeWidth += 300;
            }

            // Prepare buttons for system metrics
            ButtonConfig cpuUsageLabel;
            cpuUsageLabel.id = "##cpuUsageLabel";
            cpuUsageLabel.label = "CPU: " + cpuSS.str() + "%";
            cpuUsageLabel.size = ImVec2(100, 20);
            cpuUsageLabel.fontSize = FontsManager::SM;

            buttonConfigs.push_back(cpuUsageLabel);

            ButtonConfig memoryUsageLabel;
            memoryUsageLabel.id = "##memoryUsageLabel";
            memoryUsageLabel.label = "Memory: " + formatMemory(memoryUsageMB) + " / " + formatMemory(memoryTotalMB);
            memoryUsageLabel.size = ImVec2(170, 20);  // Adjusted size to accommodate GB format
            memoryUsageLabel.fontSize = FontsManager::SM;

            buttonConfigs.push_back(memoryUsageLabel);

            if (sysMonitor.hasGpuSupport()) {
                size_t gpuUsageMB = sysMonitor.getUsedGpuMemoryByProcess() / (1024 * 1024);
                size_t gpuTotalMB = sysMonitor.getTotalGpuMemory() / (1024 * 1024);
                ButtonConfig gpuUsageLabel;
                gpuUsageLabel.id = "##gpuUsageLabel";
                gpuUsageLabel.label = "GPU Memory: " + formatMemory(gpuUsageMB) + " / " + formatMemory(gpuTotalMB);
                gpuUsageLabel.size = ImVec2(245, 20);  // Adjusted size to accommodate GB format
                gpuUsageLabel.fontSize = FontsManager::SM;
                buttonConfigs.push_back(gpuUsageLabel);
                timeWidth += 255;
            }

            Button::renderGroup(buttonConfigs, contentWidth - timeWidth,
                ImGui::GetCursorPosY() - 2, 0);
        }
        ImGui::End();

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }

private:
    std::chrono::steady_clock::time_point lastUpdateTime;
    int updateInterval;
    std::string username;
    char timeBuffer[64];

    void getCurrentUsername() {
#ifdef _WIN32
        // Windows implementation
        char buffer[UNLEN + 1];
        DWORD size = UNLEN + 1;
        if (GetUserNameA(buffer, &size)) {
            username = buffer;
        }
        else {
            username = "unknown";
        }
#else
        // Unix/Linux/macOS implementation
        const char* user = getenv("USER");
        if (user != nullptr) {
            username = user;
        }
        else {
            // Fallback to getpwuid if USER environment variable is not available
            struct passwd* pwd = getpwuid(geteuid());
            if (pwd != nullptr) {
                username = pwd->pw_name;
            }
            else {
                username = "unknown";
            }
        }
#endif
    }

    void updateCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);

        // Format time as UTC with the specified format
        std::tm tm_utc;
#ifdef _WIN32
        gmtime_s(&tm_utc, &now_time_t);
#else
        gmtime_r(&now_time_t, &tm_utc);
#endif

        std::stringstream ss;
        ss << std::put_time(&tm_utc, "%Y-%m-%d %H:%M:%S");
        std::string str = ss.str();

        // Copy to our buffer
        strncpy(timeBuffer, str.c_str(), sizeof(timeBuffer) - 1);
        timeBuffer[sizeof(timeBuffer) - 1] = '\0';
    }
};