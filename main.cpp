#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <thread>
#include <mutex>
#include <future>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

#define ID_GENERATE 1
#define ID_SAVE 2
#define ID_EXIT 3

std::mutex mtx;
std::vector<std::wstring> wordList = { L"Lorem", L"ipsum", L"dolor", L"sit", L"amet", L"consectetur", L"adipiscing", L"elit" };
std::mt19937 rng(std::random_device{}());
HWND hwndTextBox, hwndButtonGenerate, hwndButtonSave;

// Convert `std::string` to `std::wstring` (for wide-char compatibility)
std::wstring stringToWString(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::wstring generateText(size_t numSentences, size_t wordsPerSentence) {
    std::wostringstream text;
    std::uniform_int_distribution<size_t> wordDist(0, wordList.size() - 1);
    for (size_t i = 0; i < numSentences; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        for (size_t j = 0; j < wordsPerSentence; ++j) {
            text << wordList[wordDist(rng)] << L" ";
        }
        text << L". ";
    }
    return text.str();
}

void onGenerate() {
    auto futureText = std::async(std::launch::async, generateText, 10, 8);
    std::wstring generatedText = futureText.get();
    SetWindowText(hwndTextBox, generatedText.c_str());
}

void onSave() {
    wchar_t buffer[2048];
    GetWindowText(hwndTextBox, buffer, sizeof(buffer) / sizeof(wchar_t));
    std::wofstream file(L"output.txt");
    if (file) {
        file << buffer;
    }
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case ID_GENERATE:
            onGenerate();
            break;
        case ID_SAVE:
            onSave();
            break;
        case ID_EXIT:
            PostQuitMessage(0);
            break;
        }
        break;
    case WM_CREATE:
        hwndTextBox = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
            10, 10, 460, 200, hwnd, NULL, NULL, NULL);
        hwndButtonGenerate = CreateWindowW(L"BUTTON", L"Generate", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 220, 100, 30, hwnd, (HMENU)ID_GENERATE, NULL, NULL);
        hwndButtonSave = CreateWindowW(L"BUTTON", L"Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            120, 220, 100, 30, hwnd, (HMENU)ID_SAVE, NULL, NULL);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR lpCmd, int nCmdShow) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProcedure;
    wc.hInstance = hInst;
    wc.lpszClassName = L"TextGeneratorApp";
    RegisterClass(&wc);
    HWND hwnd = CreateWindowW(L"TextGeneratorApp", L"Text Generator", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, 500, 300, NULL, NULL, hInst, NULL);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
