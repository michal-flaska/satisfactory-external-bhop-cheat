#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
#include <map>

// Configuration structure
struct Config {
        UINT WalkForward;
        UINT Sprint;
        UINT Jump;
        UINT Crouch;
        int DelayBeforeCrouch;
        int DelayBeforeJump;
        int JumpInterval;
};

// Global config
Config cfg;

// Simple function to convert hex string like "0x57" to UINT
UINT HexStringToUINT(const std::string& str) {
        UINT val;
        std::stringstream ss;
        ss << std::hex << str;
        ss >> val;
        return val;
}

// Load config from config.ini
bool LoadConfig(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;

        std::map<std::string, std::string> values;
        std::string line;
        while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;
                size_t eq = line.find('=');
                if (eq == std::string::npos) continue;
                std::string key = line.substr(0, eq);
                std::string value = line.substr(eq + 1);
                values[key] = value;
        }

        try {
                cfg.WalkForward = HexStringToUINT(values.at("WalkForward"));
                cfg.Sprint = HexStringToUINT(values.at("Sprint"));
                cfg.Jump = HexStringToUINT(values.at("Jump"));
                cfg.Crouch = HexStringToUINT(values.at("Crouch"));
                cfg.DelayBeforeCrouch = std::stoi(values.at("DelayBeforeCrouch"));
                cfg.DelayBeforeJump = std::stoi(values.at("DelayBeforeJump"));
                cfg.JumpInterval = std::stoi(values.at("JumpInterval"));
        }
        catch (...) {
                return false;
        }
        return true;
}

// Press and hold key using SendInput
void HoldKey(UINT vk) {
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = vk;
        input.ki.dwFlags = 0; // key down
        SendInput(1, &input, sizeof(INPUT));
}

// Release key using SendInput
void ReleaseKey(UINT vk) {
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = vk;
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
}

// Check if key is currently pressed
bool IsKeyPressed(UINT vk) {
        return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

// Press key briefly
void PressKey(UINT vk, int durationMs) {
        HoldKey(vk);
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
        ReleaseKey(vk);
}

// Main loop
int main() {
        if (!LoadConfig("config.ini")) {
                std::cerr << "[ERROR] Could not load config.ini\n";
                return 1;
        }

        std::cout << "[CONFIG LOADED]\n";

        while (true) {
                // Wait until WalkForward + Sprint are pressed
                if (IsKeyPressed(cfg.WalkForward) && IsKeyPressed(cfg.Sprint)) {

                        std::this_thread::sleep_for(std::chrono::milliseconds(cfg.DelayBeforeCrouch));

                        // Hold Crouch
                        HoldKey(cfg.Crouch);

                        std::this_thread::sleep_for(std::chrono::milliseconds(cfg.DelayBeforeJump));

                        // Spam Jump while WalkForward + Sprint are held
                        while (IsKeyPressed(cfg.WalkForward) && IsKeyPressed(cfg.Sprint)) {
                                PressKey(cfg.Jump, cfg.JumpInterval);
                                std::this_thread::sleep_for(std::chrono::milliseconds(cfg.JumpInterval));
                        }

                        // Release Crouch when exiting
                        ReleaseKey(cfg.Crouch);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        return 0;
}
