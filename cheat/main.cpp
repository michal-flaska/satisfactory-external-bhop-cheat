#include <windows.h>    // For Windows API functions like SendInput, GetAsyncKeyState
#include <iostream>
#include <fstream>      // For file reading
#include <sstream>      // For stringstream (used to convert hex strings)
#include <string>       // For std::string
#include <thread>       // For std::this_thread::sleep_for
#include <chrono>       // For std::chrono::milliseconds
#include <map>          // For storing config key-value pairs

// ------------------------------------------------ CONFIG ------------------------------------------------

// Store all configuration values in one place
struct Config {
        UINT WalkForward;       // Virtual-Key code for walking forward (e.g., W)
        UINT Sprint;            // Virtual-Key code for sprinting (e.g., Shift)
        UINT Jump;              // Virtual-Key code for jumping (e.g., Space)
        UINT Crouch;            // Virtual-Key code for crouching (e.g., Ctrl)
        int DelayBeforeCrouch;  // Delay in milliseconds before holding crouch
        int DelayBeforeJump;    // Delay in milliseconds before starting to spam jump
        int JumpInterval;       // Interval between jump presses in milliseconds
};

// Global config instance
Config cfg;

// ------------------------------------------------ HELPER FUNCTIONS ------------------------------------------------

// Convert hex string (like "0x57") to UINT (Virtual-Key code)
UINT HexStringToUINT(const std::string& str) {
        UINT val;
        std::stringstream ss;
        ss << std::hex << str;  // Convert string from hexadecimal
        ss >> val;              // Store result in val
        return val;
}

// Load config values from config.ini
bool LoadConfig(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;              // If file doesn't exist, return false

        std::map<std::string, std::string> values;      // Temporary storage for key=value pairs
        std::string line;

        // Read file line by line
        while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;   // Skip empty lines or comments
                size_t eq = line.find('=');                     // Find '='
                if (eq == std::string::npos) continue;          // Skip lines without '='
                std::string key = line.substr(0, eq);           // Extract key
                std::string value = line.substr(eq + 1);        // Extract value
                values[key] = value;                            // Store in map
        }

        // Assign values to config struct
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
                return false; // If any key is missing or invalid, fail
        }
        return true;
}

// Press and hold a key using Windows API
void HoldKey(UINT vk) {
        INPUT input{};
        input.type = INPUT_KEYBOARD;            // Specify that this is keyboard input
        input.ki.wVk = vk;                      // Set Virtual-Key code
        input.ki.dwFlags = 0;                   // 0 = key down
        SendInput(1, &input, sizeof(INPUT));    // Send the input to Windows
}

// Release a previously held key
void ReleaseKey(UINT vk) {
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = vk;
        input.ki.dwFlags = KEYEVENTF_KEYUP; // Key release flag
        SendInput(1, &input, sizeof(INPUT));
}

// Check if a key is currently being pressed
bool IsKeyPressed(UINT vk) {
        return (GetAsyncKeyState(vk) & 0x8000) != 0; // 0x8000 mask checks high-order bit
}

// Press a key briefly (used for spamming jump)
void PressKey(UINT vk, int durationMs) {
        HoldKey(vk);                                                            // Press key down
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));     // Wait
        ReleaseKey(vk);                                                         // Release key
}

// ------------------------------------------------ MAIN LOOP ------------------------------------------------
int main() {
        if (!LoadConfig("config.ini")) {    // Load configuration at startup
                std::cerr << "[ERROR] Could not load config.ini\n";
                return 1;
        }

        std::cout << "[CONFIG LOADED]\n";

        while (true) {
                // Wait until WalkForward + Sprint are pressed
                if (IsKeyPressed(cfg.WalkForward) && IsKeyPressed(cfg.Sprint)) {

                        std::this_thread::sleep_for(std::chrono::milliseconds(cfg.DelayBeforeCrouch));  // Wait before crouch

                        HoldKey(cfg.Crouch);                                                            // Start crouching/sliding

                        std::this_thread::sleep_for(std::chrono::milliseconds(cfg.DelayBeforeJump));    // Wait before jumping

                        // Loop: spam jump while WalkForward + Sprint are held
                        while (IsKeyPressed(cfg.WalkForward) && IsKeyPressed(cfg.Sprint)) {
                                PressKey(cfg.Jump, cfg.JumpInterval);                                   // Press jump
                                std::this_thread::sleep_for(std::chrono::milliseconds(cfg.JumpInterval)); // Wait interval
                        }

                        ReleaseKey(cfg.Crouch); // Stop crouching when loop ends
                }

                // Small delay to prevent CPU overuse
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        return 0;
}
