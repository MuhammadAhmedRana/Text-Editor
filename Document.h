#pragma once
#include <vector>
#include "Chapter.h"
#include "MyString.h"

enum EditorMode { MODE_NORMAL, MODE_INSERT, MODE_COMMAND };

struct Snapshot {
    vector<MyString> texts;
    int row;
    int col;
};

enum RepeatableCmd { REPEAT_NONE, REPEAT_DD, REPEAT_D, REPEAT_X, REPEAT_TILDE, REPEAT_YY, REPEAT_P, REPEAT_SHIFT_P };

struct Spot {
    int chapter, section, paragraph, line;
    bool found;
};

class Document {
private:
    vector<Chapter> chapters;
    vector<Line*> lines;
    vector<Spot> spots;

    void rebuildLines();
    Spot findSpot(int row) const;
    void insertLineAfter(int row, const Line& newLine);
    void insertLineBefore(int row, const Line& newLine);
    void removeLine(int row);

public:
    int cursorRow, cursorCol;
    EditorMode mode;
    MyString filename;
    bool lineNumbersOn;
    bool modified;
    bool quitRequested;
    MyString statusMessage;

    MyString commandBuffer;

    vector<MyString> clipboardLines;
    bool wholeLine;

    bool selecting;
    int selAnchorRow, selAnchorCol;

    MyString lastSearchPattern;
    bool lastSearchForward;

    vector<Snapshot> undoStack;
    vector<Snapshot> redoStack;

    RepeatableCmd lastCmd;

    Document();

    bool loadFromFile(const char* path);
    bool saveToFile(const char* path = nullptr);

    int lineCount() const;
    Line& getLine(int row);
    const Line& getLine(int row) const;

    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    void moveWordForward();
    void moveWordBackward();
    void moveToLineStart();
    void moveToLineEnd();
    void moveToDocStart();
    void moveToDocEnd();
    void scrollHalfPageDown(int pageSize);
    void scrollHalfPageUp(int pageSize);
    void fixCursor();

    void pushUndo();
    void undo();
    void redo();

    void deleteCurrentLine();
    void deleteToEndOfLine();
    void deleteChar();
    void toggleCase();
    void yankCurrentLine();
    void pasteAfter();
    void pasteBefore();
    void repeatLastCommand();

    void typeChar(char c);
    void newLine();
    void backspace();
    void deleteForward();
    void indent();
    void unindent();

    void startSelect();
    void clearSelection();
    void getSelectionRange(int& r1, int& c1, int& r2, int& c2) const;
    void copySelection();
    void cutSelection();
    void pasteHere();

    bool searchFor(const MyString& pattern, bool forward);
    bool findNext();
    bool findPrevious();

    MyString executeCommand(const MyString& cmd);
    void replaceAll(const MyString& oldSub, const MyString& newSub);
};
