#pragma once
#include <windows.h>

void setConsoleSettings();
void enableEditorInputMode();

void getRowColbyLeftClick(int& rpos, int& cpos);
void gotoRowCol(int rpos, int cpos);
void color(int k);
void hideConsoleCursor();

void showConsoleCursor();
void clearScreen();
void getConsoleSize(int& cols, int& rows);
void writePadded(const char* text, int width);

void writeConsoleFrame(CHAR_INFO* buffer, int cols, int rows);
