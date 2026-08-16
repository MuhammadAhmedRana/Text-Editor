#include "Console.h"
#include <windows.h>
#include <cstdio>

static HANDLE hOut = nullptr;

static HANDLE inHandle()
{
    return GetStdHandle(STD_INPUT_HANDLE);
}

static HANDLE out()
{
    if (!hOut)
    {
        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    return hOut;
}

void setConsoleSettings()
{
    SetConsoleTitleA("OOPX Text Editor - VIM-style Modal Editor");

    HANDLE h = out();

    COORD bufferSize = { 120, 40 };
    SetConsoleScreenBufferSize(h, bufferSize);

    SMALL_RECT windowSize = { 0, 0, 119, 34 };
    SetConsoleWindowInfo(h, TRUE, &windowSize);

    CONSOLE_FONT_INFOEX fontInfo;
    ZeroMemory(&fontInfo, sizeof(fontInfo));
    fontInfo.cbSize = sizeof(fontInfo);
    fontInfo.nFont = 0;
    fontInfo.dwFontSize.X = 0;
    fontInfo.dwFontSize.Y = 18;
    fontInfo.FontFamily = FF_DONTCARE;
    fontInfo.FontWeight = FW_NORMAL;
    wcscpy(fontInfo.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(h, FALSE, &fontInfo);

    color(7);
    hideConsoleCursor();
}

void enableEditorInputMode()
{
    SetConsoleMode(inHandle(), ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
}

void getRowColbyLeftClick(int& rpos, int& cpos)
{
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD events;
    INPUT_RECORD inputRecord;
    SetConsoleMode(hInput, ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);

    do
    {
        ReadConsoleInput(hInput, &inputRecord, 1, &events);
        if (inputRecord.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
        {
            cpos = inputRecord.Event.MouseEvent.dwMousePosition.X;
            rpos = inputRecord.Event.MouseEvent.dwMousePosition.Y;
            break;
        }
    } while (true);
}

void gotoRowCol(int rpos, int cpos)
{
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD screenPos;
    screenPos.X = cpos;
    screenPos.Y = rpos;
    SetConsoleCursorPosition(hOutput, screenPos);
}

void color(int k)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, k);
}

void hideConsoleCursor()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void showConsoleCursor()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = true;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void clearScreen()
{
    HANDLE h = out();
    CONSOLE_SCREEN_BUFFER_INFO screenInfo;
    GetConsoleScreenBufferInfo(h, &screenInfo);
    DWORD size = screenInfo.dwSize.X * screenInfo.dwSize.Y;
    COORD topLeft = { 0, 0 };
    DWORD written;
    FillConsoleOutputCharacterA(h, ' ', size, topLeft, &written);
    FillConsoleOutputAttribute(h, screenInfo.wAttributes, size, topLeft, &written);
    SetConsoleCursorPosition(h, topLeft);
}

void getConsoleSize(int& cols, int& rows)
{
    CONSOLE_SCREEN_BUFFER_INFO screenInfo;
    GetConsoleScreenBufferInfo(out(), &screenInfo);
    cols = screenInfo.srWindow.Right - screenInfo.srWindow.Left + 1;
    rows = screenInfo.srWindow.Bottom - screenInfo.srWindow.Top + 1;
}

void writeConsoleFrame(CHAR_INFO* buffer, int cols, int rows)
{
    COORD bufSize = { (SHORT)cols, (SHORT)rows };
    COORD bufCoord = { 0, 0 };
    SMALL_RECT region = { 0, 0, (SHORT)(cols - 1), (SHORT)(rows - 1) };
    WriteConsoleOutputA(out(), buffer, bufSize, bufCoord, &region);
}

void writePadded(const char* text, int width)
{
    int textLen = 0;
    while (text[textLen] != '\0')
    {
        textLen++;
    }

    fputs(text, stdout);
    for (int i = textLen; i < width; i++)
    {
        fputc(' ', stdout);
    }
}
