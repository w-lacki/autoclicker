#include "clicker.h"
#include <atomic>
#include <iostream>
#include <fstream>
#include <thread>
#include <nlohmann/json.hpp>
#include <Windows.h>
#include "recorder.h"
#include "hooks.h"
#include "utils.h"

using json = nlohmann::json;

struct Clicker {
    std::atomic<bool> chill = false;
    std::atomic<bool> active = false;
    std::atomic<bool> holding = false;
    std::atomic<bool> skipNextPress = false;
    std::atomic<bool> skipNextRelease = false;
    float multiplier = 1.0f;
    int click = 0;
};

Clicker clicker;

void mark_active() {
    clicker.holding = true;
    clicker.chill = true;
}

LRESULT CALLBACK handle_keyboard_message(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode != HC_ACTION) return CallNextHookEx(keyboard_hook, nCode, wParam, lParam);
    auto *keyboardHook = (KBDLLHOOKSTRUCT *) lParam;

    if (wParam == WM_KEYDOWN) {
        if (keyboardHook->vkCode == VK_LSHIFT) {
            clicker.active = false;
        } else if (keyboardHook->vkCode == VK_F4) {
            clicker.active = !clicker.active;
            std::cout << (clicker.active ? "WLACZONY" : "WYLACZONY") << std::endl;
        }
    } else if (wParam == WM_KEYUP) {
        if (keyboardHook->vkCode == VK_LSHIFT) {
            mark_active();
        }
    }

    return CallNextHookEx(keyboard_hook, nCode, wParam, lParam);
}

void hook_keyboard() {
    keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, handle_keyboard_message, NULL, 0);
    if (keyboard_hook == nullptr) {
        std::cerr << "kibord huk sie zjebau!" << std::endl;
        exit(1);
    }
}

void unhook_keyboard() {
    UnhookWindowsHookEx(keyboard_hook);
}

LRESULT CALLBACK handle_mouse_message(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode != HC_ACTION) return CallNextHookEx(mouse_hook, nCode, wParam, lParam);

    if (wParam == WM_LBUTTONDOWN) {
        if (!clicker.skipNextPress) {
            mark_active();
        }

        clicker.skipNextPress = false;
    } else if (wParam == WM_LBUTTONUP) {
        if (!clicker.skipNextRelease) {
            clicker.holding = false;
        }

        clicker.skipNextRelease = false;
    }

    return CallNextHookEx(mouse_hook, nCode, wParam, lParam);
}

void hook_mouse() {
    mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, handle_mouse_message, NULL, 0);
    if (mouse_hook == NULL) {
        std::cerr << "mals huk sie zjebau!" << std::endl;
        exit(1);
    }
}

void unhook_mouse() {
    UnhookWindowsHookEx(mouse_hook);
}

void clicker_start_message_loop() {
    hook_mouse();
    hook_keyboard();
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    unhook_keyboard();
    unhook_mouse();
}

void read_record_from_json(record &record, json json) {
    record.clicks = json["clicks"].get<std::vector<int> >();
    record.releases = json["delays"].get<std::vector<int> >();
}

void get_record_from_input(record &record) {
    std::cout << "Podaj sciezke twardzielu: " << std::endl;

    std::string path;
    std::cin >> path;

    std::ifstream input_file(path);
    if (!input_file) {
        std::cerr << "Nie znaleziono pliku " << path << std::endl;
        sleep_for(2000);
        get_record_from_input(record);
        return;
    }

    json json_data;
    input_file >> json_data;
    read_record_from_json(record, json_data);
    std::cerr << "ZALADOWANO" << std::endl;
}

void next_click(int delays[2], const record &record) {
    auto size = std::min(record.clicks.size(), record.releases.size());

    if (!clicker.click) {
        clicker.click = static_cast<int>(random_between(0, size - 1));
        double multiplier = random_between(0.9, 1.1);
        clicker.multiplier = multiplier;
    } else {
        clicker.click = (clicker.click + 1) % size;
    }


    delays[0] = static_cast<int>(record.clicks[clicker.click] * clicker.multiplier);
    delays[1] = static_cast<int>(record.releases[clicker.click] * clicker.multiplier);
}

void mouse_down() {
    static INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    clicker.skipNextPress = true;
    SendInput(1, &input, sizeof(INPUT));
}

void mouse_up() {
    static INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    clicker.skipNextRelease = true;
    SendInput(1, &input, sizeof(INPUT));
}

[[noreturn]] void start_clicker(const record &record) {
    std::cout << "ZASTOPOWANY" << std::endl;

    while (true) {
        if (clicker.holding && clicker.active) {
            static int delays[2];
            next_click(delays, record);

            if (!clicker.chill) {
                mouse_down();
            }

            const auto next_mouse_down_delay = delays[1];
            sleep_for(next_mouse_down_delay);

            if (!clicker.chill) {
                mouse_up();
            }

            clicker.chill = false;

            const auto next_mouse_up_delay = delays[0];
            sleep_for(next_mouse_up_delay);
        } else {
            sleep_for(1);
        }
    }
}

void run_clicker() {
    record record;
    get_record_from_input(record);
    std::thread clicker_thread(start_clicker, record);
    clicker_start_message_loop();
    clicker_thread.join();
}
