#include <windows.h>
#include <cstdio>
#include <cstring>
#include <vector>
#include <iostream>
#include "Document.h"
#include "Console.h"
using namespace std;

static const int Border_margin_Width = 5;
static const int STATUS_ROWS = 2;

static int topRow = 0;
static int leftCol = 0;

static void scrollView(Document& doc, int textRows, int textCols)
{
    if (doc.cursorRow < topRow)
    {
        topRow = doc.cursorRow;
    }
    if (doc.cursorRow >= topRow + textRows)
    {
        topRow = doc.cursorRow - textRows + 1;
    }
    if (topRow < 0)
    {
        topRow = 0;
    }

    if (doc.cursorCol < leftCol)
    {
        leftCol = doc.cursorCol;
    }
    if (doc.cursorCol >= leftCol + textCols)
    {
        leftCol = doc.cursorCol - textCols + 1;
    }
    if (leftCol < 0)
    {
        leftCol = 0;
    }
}

static bool clickToPos(Document& doc, int consoleRows, int mouseRow, int mouseCol, int& outRow, int& outCol)
{
    int textRows = consoleRows - STATUS_ROWS;
    if (mouseRow < 0 or mouseRow >= textRows)
    {
        return false;
    }

    int Border_margin = doc.lineNumbersOn ? Border_margin_Width : 0;
    int docRow = topRow + mouseRow;
    int docCol = mouseCol - Border_margin + leftCol;
    if (docCol < 0)
    {
        docCol = 0;
    }

    if (docRow >= doc.lineCount())
    {
        docRow = doc.lineCount() - 1;
    }
    if (docRow < 0)
    {
        docRow = 0;
    }

    int maxCol = doc.getLine(docRow).length();
    if (doc.mode == MODE_NORMAL and maxCol > 0)
    {
        maxCol -= 1;
    }
    if (docCol > maxCol)
    {
        docCol = maxCol;
    }

    outRow = docRow;
    outCol = docCol;
    return true;
}

static const char* modeLabel(EditorMode mode)
{
    switch (mode)
    {
        case MODE_NORMAL: return " NORMAL ";
        case MODE_INSERT: return " INSERT ";
        case MODE_COMMAND: return " COMMAND ";
    }
    return " ??? ";
}

static void setCell(vector<CHAR_INFO>& frame, int consoleCols, int consoleRows, int row, int col, char letter, WORD attr)
{
    if (row < 0 or row >= consoleRows or col < 0 or col >= consoleCols)
    {
        return;
    }
    CHAR_INFO& cell = frame[(size_t)row * consoleCols + col];
    cell.Char.AsciiChar = letter;
    cell.Attributes = attr;
}

static void drawText(vector<CHAR_INFO>& frame, Document& doc, int consoleCols, int consoleRows, int textRows, int textCols, int Border_margin)
{
    int selR1 = -1, selC1 = -1, selR2 = -1, selC2 = -1;
    if (doc.selecting)
    {
        doc.getSelectionRange(selR1, selC1, selR2, selC2);
    }

    for (int screenRow = 0; screenRow < textRows; screenRow++)
    {
        int docRow = topRow + screenRow;

        if (Border_margin > 0)
        {
            char numBuf[16];
            if (docRow < doc.lineCount())
            {
                snprintf(numBuf, sizeof(numBuf), "%3d |", docRow + 1);
            }
            else
            {
                snprintf(numBuf, sizeof(numBuf), "     ");
            }
            for (int i = 0; i < Border_margin; i++)
            {
                setCell(frame, consoleCols, consoleRows, screenRow, i, numBuf[i] ? numBuf[i] : ' ', 8);
            }
        }

        if (docRow < doc.lineCount())
        {
            MyString text = doc.getLine(docRow).getText();
            int len = text.length();
            for (int screenCol = 0; screenCol < textCols; screenCol++)
            {
                int docCol = leftCol + screenCol;
                bool inSelection = doc.selecting and
                    ((docRow > selR1 and docRow < selR2) or
                     (docRow == selR1 and docRow == selR2 and docCol >= selC1 and docCol < selC2) or
                     (docRow == selR1 and docRow != selR2 and docCol >= selC1) or
                     (docRow == selR2 and docRow != selR1 and docCol < selC2));
                char letter = (docCol < len) ? text[docCol] : ' ';
                setCell(frame, consoleCols, consoleRows, screenRow, Border_margin + screenCol, letter, inSelection ? 112 : 7);
            }
        }
        else
        {
            setCell(frame, consoleCols, consoleRows, screenRow, Border_margin, '~', 8);
            for (int i = 1; i < textCols; i++)
            {
                setCell(frame, consoleCols, consoleRows, screenRow, Border_margin + i, ' ', 7);
            }
        }
    }
}

static void drawStatusBar(vector<CHAR_INFO>& frame, Document& doc, int consoleCols, int consoleRows, int textRows)
{
    WORD statusAttr = doc.mode == MODE_NORMAL ? 96 : (doc.mode == MODE_INSERT ? 34 : 78);

    char left[256];
    snprintf(left, sizeof(left), "%s %s%s", modeLabel(doc.mode), doc.filename.c_str(), doc.modified ? " [+]" : "");
    char right[64];
    snprintf(right, sizeof(right), "Ln %d, Col %d", doc.cursorRow + 1, doc.cursorCol + 1);
    int leftLen = (int)strlen(left);
    int rightLen = (int)strlen(right);

    for (int c = 0; c < consoleCols; c++)
    {
        char letter = ' ';
        if (c < leftLen)
        {
            letter = left[c];
        }
        else if (c >= consoleCols - rightLen)
        {
            letter = right[c - (consoleCols - rightLen)];
        }
        setCell(frame, consoleCols, consoleRows, textRows, c, letter, statusAttr);
    }
}

static int drawCommandLine(vector<CHAR_INFO>& frame, Document& doc, int consoleCols, int consoleRows, int textRows)
{
    char bottom[512];
    if (doc.mode == MODE_COMMAND)
    {
        snprintf(bottom, sizeof(bottom), ":%s", doc.commandBuffer.c_str());
    }
    else
    {
        snprintf(bottom, sizeof(bottom), "%s", doc.statusMessage.c_str());
    }

    int bottomLen = (int)strlen(bottom);
    for (int c = 0; c < consoleCols; c++)
    {
        setCell(frame, consoleCols, consoleRows, textRows + 1, c, c < bottomLen ? bottom[c] : ' ', 7);
    }
    return bottomLen;
}

static void render(Document& doc, int consoleCols, int consoleRows)
{
    int textRows = consoleRows - STATUS_ROWS;
    int Border_margin = doc.lineNumbersOn ? Border_margin_Width : 0;
    int textCols = consoleCols - Border_margin;
    if (textRows < 1)
    {
        textRows = 1;
    }
    if (textCols < 1)
    {
        textCols = 1;
    }

    scrollView(doc, textRows, textCols);

    static vector<CHAR_INFO> frame;
    frame.assign((size_t)consoleCols * consoleRows, CHAR_INFO{});

    drawText(frame, doc, consoleCols, consoleRows, textRows, textCols, Border_margin);
    drawStatusBar(frame, doc, consoleCols, consoleRows, textRows);
    int bottomLen = drawCommandLine(frame, doc, consoleCols, consoleRows, textRows);

    writeConsoleFrame(frame.data(), consoleCols, consoleRows);

    int cursorScreenRow = doc.cursorRow - topRow;
    int cursorScreenCol = Border_margin + (doc.cursorCol - leftCol);
    showConsoleCursor();
    if (doc.mode == MODE_COMMAND)
    {
        gotoRowCol(textRows + 1, bottomLen);
    }
    else
    {
        gotoRowCol(cursorScreenRow, cursorScreenCol);
    }
}

static void onClick(Document& doc, int consoleRows, const MOUSE_EVENT_RECORD& mouse)
{
    bool leftClick = mouse.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED;
    if (!leftClick or (doc.mode != MODE_NORMAL and doc.mode != MODE_INSERT))
    {
        return;
    }

    int clickRow = mouse.dwMousePosition.Y;
    int clickCol = mouse.dwMousePosition.X;
    int docRow, docCol;
    if (clickToPos(doc, consoleRows, clickRow, clickCol, docRow, docCol))
    {
        doc.clearSelection();
        doc.cursorRow = docRow;
        doc.cursorCol = docCol;
        doc.fixCursor();
    }
}

static void normalMode(Document& doc, WORD key, char ch, bool ctrl, int textRows)
{
    static bool pendingG = false;
    static bool pendingD = false;
    static bool pendingY = false;

    if (pendingG)
    {
        pendingG = false;
        if (ch == 'g')
        {
            doc.moveToDocStart();
        }
        return;
    }
    if (pendingD)
    {
        pendingD = false;
        if (ch == 'd')
        {
            doc.deleteCurrentLine();
        }
        return;
    }
    if (pendingY)
    {
        pendingY = false;
        if (ch == 'y')
        {
            doc.yankCurrentLine();
        }
        return;
    }

    if ((ctrl and key == 'D') or ch == 4)
    {
        doc.scrollHalfPageDown(textRows);
        return;
    }
    if ((ctrl and key == 'U') or ch == 21)
    {
        doc.scrollHalfPageUp(textRows);
        return;
    }
    if ((ctrl and key == 'R') or ch == 18)
    {
        doc.redo();
        return;
    }

    switch (ch)
    {
        case 'h': doc.moveLeft(); break;
        case 'l': doc.moveRight(); break;
        case 'j': doc.moveDown(); break;
        case 'k': doc.moveUp(); break;
        case 'w': doc.moveWordForward(); break;
        case 'b': doc.moveWordBackward(); break;
        case '0': doc.moveToLineStart(); break;
        case '$': doc.moveToLineEnd(); break;
        case 'g': pendingG = true; break;
        case 'G': doc.moveToDocEnd(); break;
        case 'd': pendingD = true; break;
        case 'D': doc.deleteToEndOfLine(); break;
        case 'x': doc.deleteChar(); break;
        case 'y': pendingY = true; break;
        case 'p': doc.pasteAfter(); break;
        case 'P': doc.pasteBefore(); break;
        case 'u': doc.undo(); break;
        case '.': doc.repeatLastCommand(); break;
        case '~': doc.toggleCase(); break;
        case 'i': doc.mode = MODE_INSERT; break;
        case ':': doc.mode = MODE_COMMAND; doc.commandBuffer.clear(); break;
        default: break;
    }

    if (key == VK_LEFT)
    {
        doc.moveLeft();
    }
    else if (key == VK_RIGHT)
    {
        doc.moveRight();
    }
    else if (key == VK_UP)
    {
        doc.moveUp();
    }
    else if (key == VK_DOWN)
    {
        doc.moveDown();
    }
}

static void insertMode(Document& doc, WORD key, char ch, bool ctrl, bool shift)
{
    if (key == VK_ESCAPE)
    {
        doc.mode = MODE_NORMAL;
        doc.clearSelection();
        doc.fixCursor();
        return;
    }
    if ((ctrl and key == 'C') or ch == 3)
    {
        doc.copySelection();
        return;
    }
    if ((ctrl and key == 'X') or ch == 24)
    {
        doc.cutSelection();
        return;
    }
    if ((ctrl and key == 'V') or ch == 22)
    {
        doc.pasteHere();
        return;
    }

    bool isArrow = (key == VK_LEFT or key == VK_RIGHT or key == VK_UP or key == VK_DOWN);
    if (isArrow)
    {
        if (shift)
        {
            doc.startSelect();
        }
        else
        {
            doc.clearSelection();
        }

        switch (key)
        {
            case VK_LEFT: doc.moveLeft(); break;
            case VK_RIGHT: doc.moveRight(); break;
            case VK_UP: doc.moveUp(); break;
            case VK_DOWN: doc.moveDown(); break;
        }
        return;
    }

    if (key == VK_RETURN)
    {
        doc.newLine();
        return;
    }
    if (key == VK_BACK)
    {
        doc.backspace();
        return;
    }
    if (key == VK_DELETE)
    {
        doc.deleteForward();
        return;
    }
    if (key == VK_TAB)
    {
        if (shift)
        {
            doc.unindent();
        }
        else
        {
            doc.indent();
        }
        return;
    }
    if (key == VK_HOME)
    {
        doc.moveToLineStart();
        return;
    }
    if (key == VK_END)
    {
        doc.moveToLineEnd();
        return;
    }

    if ((unsigned char)ch >= 32)
    {
        doc.typeChar(ch);
    }
}

static void commandMode(Document& doc, WORD key, char ch, bool ctrl)
{
    if (key == VK_ESCAPE or ((ctrl and key == 'C') or ch == 3))
    {
        doc.mode = MODE_NORMAL;
        doc.commandBuffer.clear();
        return;
    }
    if (key == VK_RETURN)
    {
        MyString msg = doc.executeCommand(doc.commandBuffer);
        doc.commandBuffer.clear();
        if (!doc.quitRequested)
        {
            doc.mode = MODE_NORMAL;
        }
        doc.statusMessage = msg;
        return;
    }
    if (key == VK_BACK)
    {
        int len = doc.commandBuffer.length();
        if (len > 0)
        {
            doc.commandBuffer.eraseAt(len - 1, 1);
        }
        return;
    }

    if ((unsigned char)ch >= 32)
    {
        doc.commandBuffer += ch;
    }
}

static void askFileName(int argc, char* argv[], char* fileBuf, int bufSize)
{
    if (argc > 1)
    {
        strncpy(fileBuf, argv[1], bufSize - 1);
        return;
    }

    showConsoleCursor();
    gotoRowCol(0, 0);
    cout << "OOPX Text Editor\n";
    cout << "Enter a filename to open (existing or new), or press Enter for a blank file: ";
    cin.getline(fileBuf, bufSize);
}

static Document openDoc(const char* fileBuf)
{
    Document doc;

    if (fileBuf[0] == '\0')
    {
        doc.statusMessage = MyString("Welcome! Press i to insert, : for commands, Esc for normal mode.");
        return doc;
    }

    if (doc.loadFromFile(fileBuf))
    {
        doc.statusMessage = MyString("Loaded");
    }
    else
    {
        doc.filename = MyString(fileBuf);
        doc.statusMessage = MyString("New file");
    }
    return doc;
}

int main(int argc, char* argv[])
{
    char fileBuf[260] = { 0 };

    setConsoleSettings();
    askFileName(argc, argv, fileBuf, sizeof(fileBuf));
    enableEditorInputMode();
    clearScreen();

    Document doc = openDoc(fileBuf);

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD event;
    DWORD n;

    while (!doc.quitRequested)
    {
        int cols, rows;
        getConsoleSize(cols, rows);
        render(doc, cols, rows);

        if (!ReadConsoleInputA(hIn, &event, 1, &n))
        {
            continue;
        }

        if (event.EventType == MOUSE_EVENT)
        {
            onClick(doc, rows, event.Event.MouseEvent);
            continue;
        }

        if (event.EventType != KEY_EVENT or !event.Event.KeyEvent.bKeyDown)
        {
            continue;
        }

        KEY_EVENT_RECORD ke = event.Event.KeyEvent;
        WORD key = ke.wVirtualKeyCode;
        char ch = ke.uChar.AsciiChar;
        bool ctrl = (ke.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;
        bool shift = (ke.dwControlKeyState & SHIFT_PRESSED) != 0;

        doc.statusMessage = MyString("");

        if (doc.mode == MODE_NORMAL)
        {
            normalMode(doc, key, ch, ctrl, rows - STATUS_ROWS);
        }
        else if (doc.mode == MODE_INSERT)
        {
            insertMode(doc, key, ch, ctrl, shift);
        }
        else if (doc.mode == MODE_COMMAND)
        {
            commandMode(doc, key, ch, ctrl);
        }
    }

    clearScreen();
    showConsoleCursor();
    gotoRowCol(0, 0);
    return 0;
}
