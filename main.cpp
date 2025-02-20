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
#define ID_PARAGRAPHS 4
#define ID_SENTENCES 5
#define ID_WORDS 6

std::mutex mtx;
std::vector<std::wstring> wordList = { L"Lorem", L"ipsum", L"dolor", L"sit", L"amet", L"consectetur", L"adipiscing", L"elit" };
std::mt19937 rng(std::random_device{}());
HWND hwndTextBox, hwndButtonGenerate, hwndButtonSave, hwndParagraphs, hwndSentences, hwndWords;

std::wstring generateText(size_t numParagraphs, size_t numSentences, size_t wordsPerSentence) {
    std::wostringstream text;
    std::uniform_int_distribution<size_t> wordDist(0, wordList.size() - 1);
    for (size_t p = 0; p < numParagraphs; ++p) {
        for (size_t i = 0; i < numSentences; ++i) {
            std::lock_guard<std::mutex> lock(mtx);
            for (size_t j = 0; j < wordsPerSentence; ++j) {
                text << wordList[wordDist(rng)] << L" ";
            }
            text << L". ";
        }
        text << L"\n\n";
    }
    return text.str();
}

void onGenerate() {
    wchar_t bufferP[10], bufferS[10], bufferW[10];
    GetWindowText(hwndParagraphs, bufferP, 10);
    GetWindowText(hwndSentences, bufferS, 10);
    GetWindowText(hwndWords, bufferW, 10);

    size_t numParagraphs = _wtoi(bufferP);
    size_t numSentences = _wtoi(bufferS);
    size_t wordsPerSentence = _wtoi(bufferW);

    auto futureText = std::async(std::launch::async, generateText, numParagraphs, numSentences, wordsPerSentence);
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
            10, 250, 100, 30, hwnd, (HMENU)ID_GENERATE, NULL, NULL);
        hwndButtonSave = CreateWindowW(L"BUTTON", L"Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            120, 250, 100, 30, hwnd, (HMENU)ID_SAVE, NULL, NULL);
        hwndParagraphs = CreateWindowW(L"EDIT", L"1", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            10, 220, 50, 20, hwnd, (HMENU)ID_PARAGRAPHS, NULL, NULL);
        hwndSentences = CreateWindowW(L"EDIT", L"5", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            70, 220, 50, 20, hwnd, (HMENU)ID_SENTENCES, NULL, NULL);
        hwndWords = CreateWindowW(L"EDIT", L"8", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            130, 220, 50, 20, hwnd, (HMENU)ID_WORDS, NULL, NULL);
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
        100, 100, 500, 320, NULL, NULL, hInst, NULL);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
