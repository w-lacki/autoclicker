#include <vector>
#include <windows.h>
#include <iostream>
#include <chrono>
#include <nlohmann/json.hpp>
#include "utils.h"
#include "recorder.h"
#include <fstream>
#include "hooks.h"

struct recorder {
    bool paused;
    long lastClick;
    long lastRelease;
    record *record;
};

constexpr int CLICK_DELAY_THRESHOLD = 200;

recorder recorder;

void write_record_to_json(nlohmann::json &j, record *p) {
    j = nlohmann::json{{"clicks", p->clicks}, {"delays", p->releases}};
}

void save_to_file() {
    std::cout << "Podaj sciezke zapisu " << std::endl;
    std::string path;
    std::cin >> path;

    std::cout << recorder.record->clicks.size() << std::endl;
    recorder.record->clicks.erase(recorder.record->clicks.begin());
    recorder.record->releases.erase(recorder.record->clicks.begin());
    std::cout << recorder.record->clicks.size() << std::endl;

    nlohmann::json json;
    write_record_to_json(json, recorder.record);

    // Write the JSON object to a file
    std::ofstream file(path);
    if (file.is_open()) {
        file << json.dump(4); // Indent with 4 spaces for readability
        file.close();
    } else {
        std::cerr << "Blad ze sciezka: " << path << std::endl;
        exit(1);
        return;
    }

    std::cout << "Zapisano" << std::endl;
    exit(0);
}

void recorder_unhook_keyboard() {
    UnhookWindowsHookEx(keyboard_hook);
}

void recorder_unhook_mouse() {
    UnhookWindowsHookEx(mouse_hook);
}

LRESULT CALLBACK recorder_handle_keyboard_message(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode != HC_ACTION || wParam != WM_KEYDOWN) {
        return CallNextHookEx(keyboard_hook, nCode, wParam, lParam);
    }

    auto keyboard = (KBDLLHOOKSTRUCT *) lParam;
    if (keyboard->vkCode == VK_F6) {
        recorder.paused = !recorder.paused;
        std::cout << (recorder.paused ? "ZATRZYMANO NAGRYWANIE" : "WZNOWIONO NAGRYWANIE") << std::endl;
    } else if (keyboard->vkCode == VK_F5) {
        recorder_unhook_keyboard();
        recorder_unhook_mouse();
        save_to_file();
    }

    return CallNextHookEx(keyboard_hook, nCode, wParam, lParam);
}


void recorder_hook_keyboard() {
    keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, recorder_handle_keyboard_message, NULL, 0);
    if (keyboard_hook == nullptr) {
        std::exit(1);
    }
}


LRESULT CALLBACK recorder_handle_mouse_message(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode != HC_ACTION || recorder.paused) {
        return CallNextHookEx(mouse_hook, nCode, wParam, lParam);
    }

    if (wParam == WM_LBUTTONDOWN) {
        recorder.lastClick = current_time_millis();
        auto clickDelay = recorder.lastClick - recorder.lastRelease;
        if (clickDelay < CLICK_DELAY_THRESHOLD) {
            recorder.record->clicks.push_back(clickDelay);
        }
    } else if (wParam == WM_LBUTTONUP) {
        recorder.lastRelease = current_time_millis();
        auto pressDelay = recorder.lastRelease - recorder.lastClick;
        if (pressDelay < CLICK_DELAY_THRESHOLD && !recorder.record->clicks.empty()) {
            recorder.record->releases.push_back(pressDelay);
        }
    }

    return CallNextHookEx(mouse_hook, nCode, wParam, lParam);
}

void recorder_hook_mouse() {
    mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, recorder_handle_mouse_message, NULL, 0);
    if (mouse_hook == NULL) {
        std::cerr << "Failed to set mouse hook!" << std::endl;
    }
}

void start_message_loop() {
    recorder_hook_mouse();
    recorder_hook_keyboard();
    MSG msg;
    std::cout << "Nagrywanie zastoponowane, odstopuj F6. Zapis F5" << std::endl;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    recorder_unhook_keyboard();
    recorder_unhook_mouse();
}

void run_recorder() {
    recorder.record = new record;
    recorder.paused = true;

    start_message_loop();

    delete recorder.record;
}
