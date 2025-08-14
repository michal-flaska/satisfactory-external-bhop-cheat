#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <thread>
#include <chrono>

// Configuration structure
struct Config {
        WORD WalkForward = 0x57;    // W
        WORD Sprint = 0x10;         // SHIFT
        WORD Crouch = 0x43;         // C
        WORD Jump = 0x20;           // SPACE
        int DelayBeforeCrouch = 1000; // ms
        int DelayBeforeJump = 500;   // ms
        int JumpInterval = 50;       // ms
};

// Global config
Config cfg;

// Read key from hex string
WORD HexToVK(const std::string& hexStr) {
        return static_cast<WORD>(std::stoul(hexStr, nullptr, 16));
}

// Load config.ini
bool LoadConfig(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return false;

        std::string line;
        std::map<std::string, std::string> kv;
        while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;
                auto pos = line.find('=');
                if (pos == std::string::npos) continue;
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                kv[key] = value;
        }

        if (kv.count("WalkForward")) cfg.WalkForward = HexToVK(kv["WalkForward"]);
        if (kv.count("Sprint")) cfg.Sprint = HexToVK(kv["Sprint"]);
        if (kv.count("Crouch")) cfg.Crouch = HexToVK(kv["Crouch"]);
        if (kv.count("Jump")) cfg.Jump = HexToVK(kv["Jump"]);
        if (kv.count("DelayBeforeCrouch")) cfg.DelayBeforeCrouch = std::stoi(kv["DelayBeforeCrouch"]);
        if (kv.count("DelayBeforeJump")) cfg.DelayBeforeJump = std::stoi(kv["DelayBeforeJump"]);
        if (kv.count("JumpInterval")) cfg.JumpInterval = std::stoi(kv["JumpInterval"]);

        return true;
}

// Press key using scan code
void PressKey(WORD vk) {
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wScan = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
        input.ki.dwFlags = KEYEVENTF_SCANCODE;
        SendInput(1, &input, sizeof(INPUT));
}

// Release key using scan code
void ReleaseKey(WORD vk) {
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wScan = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
        input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
}

// Check if key is down
bool IsKeyDown(WORD vk) {
        return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

int main() {
        if (!LoadConfig("config.ini")) {
                std::cerr << "[ERROR] Could not load config.ini" << std::endl;
                return 1;
        }

        std::cout << "[CONFIG LOADED]" << std::endl;

        while (true) {
                // Wait for user to hold WalkForward + Sprint
                while (!(IsKeyDown(cfg.WalkForward) && IsKeyDown(cfg.Sprint))) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }

                // Initial delay before crouch
                std::this_thread::sleep_for(std::chrono::milliseconds(cfg.DelayBeforeCrouch));
                PressKey(cfg.Crouch);

                // Delay before starting jump spam
                std::this_thread::sleep_for(std::chrono::milliseconds(cfg.DelayBeforeJump));

                // Spam jump while keys are held
                while (IsKeyDown(cfg.WalkForward) && IsKeyDown(cfg.Sprint)) {
                        PressKey(cfg.Jump);
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        ReleaseKey(cfg.Jump);
                        std::this_thread::sleep_for(std::chrono::milliseconds(cfg.JumpInterval));
                }

                ReleaseKey(cfg.Crouch);
        }

        return 0;
}
