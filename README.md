# autoclicker

A high-performance Windows autoclicker built in C++20 that records and replays mouse patterns with human-like timing. The application uses low-level system hooks to achieve high precision and avoids static detection by applying randomized multipliers to recorded click data.

## Features

* **Pattern Recording**: Captures the exact duration of mouse presses and the delays between clicks using `WH_MOUSE_LL`.
* **Pattern Replay**: Reconstructs recorded sequences from JSON files using the `SendInput` API.
* **Algorithmic Randomization**: Applies a random multiplier (0.9 to 1.1) to each delay to prevent identical playback cycles.
* **High-Precision Timing**: Implements a custom synchronization loop utilizing Win32 Waitable Timers and spin-locks for sub-millisecond accuracy.
* **Global Hotkeys**: Controls the application via system-wide keyboard hooks even when the window is not in focus.

## Technical Details

* **Language**: C++20.
* **Dependencies**: `nlohmann/json` for data serialization.
* **Platform**: Windows (requires `Windows.h`).
* **Build System**: CMake 3.29+.

## Controls

The application prompts for a mode selection (`c` for clicker, `n` for recorder) upon startup.

### Recorder Mode (`n`)
* **F6**: Toggle recording (starts in a paused state).
* **F5**: Stop recording and save the data to a specified file path.

### Clicker Mode (`c`)
* **F4**: Global toggle to enable or disable automated clicking.
* **Left Shift**: Holding this key temporarily suspends the clicking logic.
* **File Input**: Requires the path to a JSON file generated during a recording session.

## Build Instructions

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
