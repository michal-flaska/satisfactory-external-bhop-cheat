/*
   I -> Michal Flaška (@pilot2254) created this program.
   Although it was created for personal educational purposes and may seem simple to you, it's still mine.
   Please have some respect and don't steal it, publish it as your own, or sell it.

   Have a nice day!
   - Mike
*/

/*
   This program automates a specific key sequence for games.
   Functionality:
   1. Reads user-configurable key bindings and delays from a config.ini file.
   2. Waits for the player to hold "Walk Forward" and "Sprint".
   3. After a set delay, holds "Crouch" to slide.
   4. After another delay, repeatedly presses "Jump" at a set interval.
   5. Stops when the player releases Walk Forward or Sprint.

   Core Concepts:
   - Virtual-Key codes: numeric identifiers for keyboard keys used by the Windows API.
   - SendInput: Windows function to simulate keyboard input at the system level.
   - GetAsyncKeyState: Windows function to check the current state of a key (pressed or not).
   - Configurable parameters loaded from an INI file.
*/

#include <windows.h>    // Required for SendInput, GetAsyncKeyState, and VK key codes
#include <iostream>
#include <fstream>      // For reading files (config.ini)
#include <sstream>      // For converting strings to numbers (e.g., hex to int)
#include <string>
#include <thread>
#include <chrono>
#include <map>          // For storing key=value pairs from the config file

/* ------------------------- CONFIGURATION STRUCT -------------------------
   This struct holds all the settings loaded from config.ini.
   By using a struct, we group related data together for easier handling.
*/
struct Config {
        UINT WalkForward;       // Virtual-Key code for the "Walk Forward" action
        UINT Sprint;            // Virtual-Key code for the "Sprint" action
        UINT Jump;              // Virtual-Key code for the "Jump" action
        UINT Crouch;            // Virtual-Key code for the "Crouch" action
        int DelayBeforeCrouch;  // Delay (ms) before crouch after walking+sprinting
        int DelayBeforeJump;    // Delay (ms) before starting jump spam after crouch
        int JumpInterval;       // Time (ms) between jump presses
};

/* Create a single global instance of Config called cfg */
Config cfg;

/* ------------------------- FUNCTION: HexStringToUINT -------------------------
   Purpose:
   - Converts a hexadecimal string (like "0x57") into an unsigned integer (UINT).
   - This is necessary because Virtual-Key codes in config.ini are written in hex.
*/
UINT HexStringToUINT(const std::string& str) {
        UINT val;               // Variable to store the converted number
        std::stringstream ss;   // Stringstream object for parsing
        ss << std::hex << str;  // Tell stringstream to read in hexadecimal mode
        ss >> val;              // Extract the value into 'val'
        return val;             // Return the result
}

/* ------------------------- FUNCTION: LoadConfig -------------------------
   Purpose:
   - Reads key bindings and delays from config.ini into the cfg struct.
   Steps:
   1. Open config.ini for reading.
   2. Read each line, ignoring empty lines or comments (# at start).
   3. Split the line into key and value (by '=').
   4. Store key-value pairs in a std::map.
   5. Convert strings to numbers and store in cfg.
*/
bool LoadConfig(const std::string& filename) {
        std::ifstream file(filename);           // Open config.ini
        if (!file.is_open()) return false;      // If file can't be opened, fail

        std::map<std::string, std::string> values;      // Temporary key-value storage
        std::string line;                               // Current line from the file

        // Read each line from the config file
        while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;   // Skip empty lines and comments
                size_t eq = line.find('=');                     // Find position of '='
                if (eq == std::string::npos) continue;          // If no '=', skip line
                std::string key = line.substr(0, eq);           // Text before '=' is the key
                std::string value = line.substr(eq + 1);        // Text after '=' is the value
                values[key] = value;                            // Store in map
        }

        // Assign the loaded values to cfg struct, converting strings to numbers
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
                return false;   // If any key is missing or invalid, fail
        }
        return true;            // Config loaded successfully
}

/* ------------------------- FUNCTION: HoldKey -------------------------
   Purpose:
   - Simulates pressing and holding a key using SendInput.
   - This sends a "key down" event without releasing it.
*/
void HoldKey(UINT vk) {
        INPUT input{};                          // INPUT struct for SendInput
        input.type = INPUT_KEYBOARD;            // Keyboard input
        input.ki.wVk = vk;                      // Set the Virtual-Key code
        input.ki.dwFlags = 0;                   // 0 = key down
        SendInput(1, &input, sizeof(INPUT));    // Send event to Windows
}

/* ------------------------- FUNCTION: ReleaseKey -------------------------
   Purpose:
   - Simulates releasing a key that was previously pressed.
*/
void ReleaseKey(UINT vk) {
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = vk;
        input.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP = key release
        SendInput(1, &input, sizeof(INPUT));
}

/* ------------------------- FUNCTION: IsKeyPressed -------------------------
   Purpose:
   - Checks if a specific key is currently pressed down.
   - Uses GetAsyncKeyState to check the high-order bit (0x8000).
*/
bool IsKeyPressed(UINT vk) {
        return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

/* ------------------------- FUNCTION: PressKey -------------------------
   Purpose:
   - Presses a key for a short time (key down, wait, key up).
   - Used for jump spamming.
*/
void PressKey(UINT vk, int durationMs) {
        HoldKey(vk);                                                            // Key down
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));     // Wait
        ReleaseKey(vk);                                                         // Key up
}

/* ------------------------- MAIN FUNCTION -------------------------
   This is where the program starts executing.
   Steps:
   1. Load config file.
   2. Continuously check for the activation keys (WalkForward + Sprint).
   3. Perform crouch + jump spam when active.
*/
int main() {
        // Load the configuration from config.ini
        if (!LoadConfig("config.ini")) {
                std::cerr << "[ERROR] Could not load config.ini\n";
                return 1; // Exit program with error code
        }

        std::cout << "[CONFIG LOADED]\n";

        // Infinite loop: program runs until manually closed
        while (true) {
                // Check if both WalkForward and Sprint are being pressed
                if (IsKeyPressed(cfg.WalkForward) && IsKeyPressed(cfg.Sprint)) {

                        // Wait before crouch (slide)
                        std::this_thread::sleep_for(std::chrono::milliseconds(cfg.DelayBeforeCrouch));

                        // Hold crouch key
                        HoldKey(cfg.Crouch);

                        // Wait before starting jump spam
                        std::this_thread::sleep_for(std::chrono::milliseconds(cfg.DelayBeforeJump));

                        // Loop to spam jump while keys are held
                        while (IsKeyPressed(cfg.WalkForward) && IsKeyPressed(cfg.Sprint)) {
                                PressKey(cfg.Jump, cfg.JumpInterval); // Jump press
                                std::this_thread::sleep_for(std::chrono::milliseconds(cfg.JumpInterval)); // Wait
                        }

                        // Release crouch when stopping
                        ReleaseKey(cfg.Crouch);
                }

                // Small sleep to prevent CPU from maxing out
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        return 0; // Program ends (never reached here in this design)
}
