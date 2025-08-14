# Satisfactory External "Bhop" Cheat

While exploring my Satisfactory savefile, I noticed an interesting movement bug. If you `sprint`, `walk forward`, and then `slide` for a second, your character will slide and then, after ~500 ms, repeatedly pressing the `jump` button while still holding the first three inputs allows you to move approximately **5x** faster.

I decided to automate this behavior using a fully configurable C++ application. The program waits for you to hold the walk-forward and sprint keys, then automatically performs the slide and repeated jump actions, effectively "bhopping" without manual input.

> [!NOTE]
> This project is for educational purposes only. While it simulates a movement exploit, the focus is learning Windows API, input simulation, config management, and C++ development. It has been a challenging project because I was only familiar with arrays when I started, but I completed it.

## What I learned

- Reading and parsing configuration files in C++
- Simulating keyboard input using Windows API (`SendInput`)
- Using `std::this_thread::sleep_for` to manage timing and CPU load
- Structs and maps for clean, modular code
- Continuous input monitoring with `GetAsyncKeyState`

---

## Features

- Configurable keys for `Walk Forward`, `Sprint`, `Crouch`, and `Jump`
- Configurable delays:
  - Delay before crouch
  - Delay before jump spam
  - Interval between jump presses
- Works both in Notepad for testing and in Satisfactory
- Uses Windows API `SendInput` to simulate key presses
- Minimal CPU usage (via `std::this_thread::sleep_for`)

## Setup

1. Clone the repository:
```bash
git clone https://github.com/michal-flaska/satisfactory-external-bhop-cheat.git
```

2. Open the project in Visual Studio.
3. Ensure `config.ini` is set to Copy to Output Directory in the project settings.
4. Build the project in Release mode.
5. Run the generated `.exe` while Satisfactory is running.

## Configuration

edit the `config.ini` file.

> Hex codes must follow Windows Virtual-Key codes. You can find the list here: [Virtual-Key Codes](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)

## Usage

1. Launch the executable.
2. Press and hold Walk `Forward` + `Sprint`.
3. The cheat will automatically handle crouching and jumping based on your config.
4. Release `Walk Forward` or `Sprint` to stop.

## Videos

- Normal bug recreation: https://youtu.be/6szJuAJ-xHI
- Cheat in action: https://youtu.be/ArtDfgGEQSM

## License

MIT
