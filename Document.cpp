#include "Document.h"
#include <cstring>
#include <fstream>
using namespace std;

Document::Document()
{
    cursorRow = 0;
    cursorCol = 0;
    mode = MODE_NORMAL;
    filename = MyString("untitled.txt");
    lineNumbersOn = false;
    modified = false;
    quitRequested = false;
    statusMessage = MyString("");
    wholeLine = true;
    selecting = false;
    selAnchorRow = 0;
    selAnchorCol = 0;
    lastSearchForward = true;
    lastCmd = REPEAT_NONE;

    Chapter chapter;
    Section section;
    section.addParagraph(Paragraph());
    chapter.addSection(section);
    chapters.push_back(chapter);
    rebuildLines();
}

void Document::rebuildLines()
{
    lines.clear();
    spots.clear();

    for (int c = 0; c < (int)chapters.size(); c++)
    {
        Chapter& chapter = chapters[c];

        for (int s = 0; s < chapter.sectionCount(); s++)
        {
            Section& section = chapter.getSection(s);

            for (int p = 0; p < section.paragraphCount(); p++)
            {
                Paragraph& paragraph = section.getParagraph(p);

                for (int l = 0; l < paragraph.lineCount(); l++)
                {
                    lines.push_back(&paragraph.getLine(l));

                    Spot spot;
                    spot.chapter = c;
                    spot.section = s;
                    spot.paragraph = p;
                    spot.line = l;
                    spot.found = true;
                    spots.push_back(spot);
                }
            }
        }
    }
}

Spot Document::findSpot(int row) const
{
    if (row < 0 or row >= (int)spots.size())
    {
        Spot missing{ 0, 0, 0, 0, false };
        return missing;
    }
    return spots[row];
}

void Document::insertLineAfter(int row, const Line& newLine)
{
    if (row < 0)
    {
        row = 0;
    }
    if (row >= (int)lines.size())
    {
        row = (int)lines.size() - 1;
    }

    Spot spot = findSpot(row);

    if (!spot.found)
    {
        return;
    }

    Paragraph& paragraph = chapters[spot.chapter].getSection(spot.section).getParagraph(spot.paragraph);
    paragraph.insertLine(spot.line + 1, newLine);
    rebuildLines();
}

void Document::insertLineBefore(int row, const Line& newLine)
{
    if (row < 0)
    {
        row = 0;
    }
    if (row >= (int)lines.size())
    {
        row = (int)lines.size() - 1;
    }

    Spot spot = findSpot(row);

    if (!spot.found)
    {
        return;
    }

    Paragraph& paragraph = chapters[spot.chapter].getSection(spot.section).getParagraph(spot.paragraph);
    paragraph.insertLine(spot.line, newLine);
    rebuildLines();
}

void Document::removeLine(int row)
{
    if (row < 0 or row >= (int)lines.size())
    {
        return;
    }

    Spot spot = findSpot(row);

    if (!spot.found)
    {
        return;
    }

    Paragraph& paragraph = chapters[spot.chapter].getSection(spot.section).getParagraph(spot.paragraph);
    paragraph.removeLine(spot.line);
    rebuildLines();
}

int Document::lineCount() const
{
    return (int)lines.size();
}

Line& Document::getLine(int row)
{
    return *lines[row];
}

const Line& Document::getLine(int row) const
{
    return *lines[row];
}

bool Document::loadFromFile(const char* path)
{
    ifstream ifs(path, ios::binary);

    if (!ifs.is_open())
    {
        return false;
    }

    Chapter chapter;
    Section section;
    Paragraph para;
    bool hasText = false;

    char buf[4096];

    while (ifs.getline(buf, sizeof(buf)))
    {
        int len = (int)strlen(buf);

        while (len > 0 and (buf[len - 1] == '\n' or buf[len - 1] == '\r'))
        {
            buf[--len] = '\0';
        }

        MyString text(buf);

        if (text.empty())
        {
            if (hasText)
            {
                section.addParagraph(para);
                para = Paragraph();
                hasText = false;
            }
            section.addParagraph(Paragraph());
        }
        else if (!hasText)
        {
            para.getLine(0).setText(text);
            hasText = true;
        }
        else
        {
            para.addLine(Line(text));
        }
    }

    if (hasText)
    {
        section.addParagraph(para);
    }
    ifs.close();

    if (section.paragraphCount() == 0)
    {
        section.addParagraph(Paragraph());
    }

    chapter.addSection(section);
    chapters.clear();
    chapters.push_back(chapter);
    rebuildLines();

    filename = MyString(path);
    cursorRow = 0;
    cursorCol = 0;
    modified = false;
    undoStack.clear();
    redoStack.clear();
    return true;
}

bool Document::saveToFile(const char* path)
{
    MyString target;

    if (path)
    {
        target = MyString(path);
    }
    else
    {
        target = filename;
    }

    ofstream ofs(target.c_str(), ios::binary | ios::trunc);

    if (!ofs.is_open())
    {
        return false;
    }

    for (int i = 0; i < (int)lines.size(); i++)
    {
        ofs << lines[i]->getText().c_str() << '\n';
    }
    ofs.close();

    filename = target;
    modified = false;
    return true;
}

void Document::fixCursor()
{
    if (cursorRow < 0)
    {
        cursorRow = 0;
    }
    if (cursorRow >= lineCount())
    {
        cursorRow = lineCount() - 1;
    }

    int maxCol = getLine(cursorRow).length();

    if (mode == MODE_NORMAL and maxCol > 0)
    {
        maxCol -= 1;
    }
    if (maxCol < 0)
    {
        maxCol = 0;
    }

    if (cursorCol > maxCol)
    {
        cursorCol = maxCol;
    }
    if (cursorCol < 0)
    {
        cursorCol = 0;
    }
}

void Document::moveLeft()
{
    if (cursorCol > 0)
    {
        cursorCol--;
    }
}

void Document::moveRight()
{
    int maxCol = getLine(cursorRow).length();

    if (mode == MODE_NORMAL and maxCol > 0)
    {
        maxCol -= 1;
    }
    if (cursorCol < maxCol)
    {
        cursorCol++;
    }
}

void Document::moveUp()
{
    if (cursorRow > 0)
    {
        cursorRow--;
        fixCursor();
    }
}

void Document::moveDown()
{
    if (cursorRow < lineCount() - 1)
    {
        cursorRow++;
        fixCursor();
    }
}

static bool isWordChar(char c)
{
    return (c >= '0' and c <= '9') or (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or c == '_';
}

void Document::moveWordForward()
{
    int row = cursorRow;
    int col = cursorCol;
    Line* line = &getLine(row);
    int len = line->length();

    while (col < len and isWordChar(line->charAt(col)))
    {
        col++;
    }

    while (true)
    {
        while (col < len and !isWordChar(line->charAt(col)))
        {
            col++;
        }
        if (col < len)
        {
            break;
        }
        if (row >= lineCount() - 1)
        {
            if (len > 0)
            {
                col = len - 1;
            }
            else
            {
                col = 0;
            }
            break;
        }
        row++;
        col = 0;
        line = &getLine(row);
        len = line->length();
        if (len == 0)
        {
            break;
        }
    }

    cursorRow = row;
    cursorCol = col;
    fixCursor();
}

void Document::moveWordBackward()
{
    int row = cursorRow;
    int col = cursorCol;
    Line* line = &getLine(row);

    if (col > 0)
    {
        col--;
    }

    while (true)
    {
        while (col > 0 and !isWordChar(line->charAt(col)))
        {
            col--;
        }
        if (col > 0 or (col == 0 and isWordChar(line->charAt(0))))
        {
            break;
        }
        if (row == 0)
        {
            col = 0;
            break;
        }

        row--;
        line = &getLine(row);

        if (line->length() > 0)
        {
            col = line->length() - 1;
        }
        else
        {
            col = 0;
        }

        if (line->length() == 0)
        {
            break;
        }
    }

    while (col > 0 and isWordChar(line->charAt(col - 1)))
    {
        col--;
    }

    cursorRow = row;
    cursorCol = col;
    fixCursor();
}

void Document::moveToLineStart()
{
    cursorCol = 0;
}

void Document::moveToLineEnd()
{
    int len = getLine(cursorRow).length();
    cursorCol = (len > 0) ? len - 1 : 0;

    if (mode != MODE_NORMAL)
    {
        cursorCol = len;
    }
}

void Document::moveToDocStart()
{
    cursorRow = 0;
    cursorCol = 0;
}

void Document::moveToDocEnd()
{
    cursorRow = lineCount() - 1;
    fixCursor();
}

void Document::scrollHalfPageDown(int pageSize)
{
    int half = pageSize / 2;

    if (half < 1)
    {
        half = 1;
    }
    cursorRow += half;
    fixCursor();
}

void Document::scrollHalfPageUp(int pageSize)
{
    int half = pageSize / 2;

    if (half < 1)
    {
        half = 1;
    }
    cursorRow -= half;
    fixCursor();
}

void Document::pushUndo()
{
    Snapshot snap;
    snap.row = cursorRow;
    snap.col = cursorCol;

    for (int i = 0; i < lineCount(); i++)
    {
        snap.texts.push_back(getLine(i).getText());
    }

    undoStack.push_back(snap);

    if (undoStack.size() > 200)
    {
        undoStack.erase(undoStack.begin());
    }
    redoStack.clear();
}

void Document::undo()
{
    if (undoStack.empty())
    {
        statusMessage = MyString("Already at oldest change");
        return;
    }

    Snapshot current;
    current.row = cursorRow;
    current.col = cursorCol;

    for (int i = 0; i < lineCount(); i++)
    {
        current.texts.push_back(getLine(i).getText());
    }
    redoStack.push_back(current);

    Snapshot snap = undoStack.back();
    undoStack.pop_back();

    Chapter chapter;
    Section section;
    Paragraph para;
    para.getLine(0).setText(snap.texts[0]);

    for (int i = 1; i < (int)snap.texts.size(); i++)
    {
        para.addLine(Line(snap.texts[i]));
    }

    section.addParagraph(para);
    chapter.addSection(section);
    chapters.clear();
    chapters.push_back(chapter);
    rebuildLines();

    cursorRow = snap.row;
    cursorCol = snap.col;
    fixCursor();
    modified = true;
    statusMessage = MyString("Undo");
}

void Document::redo()
{
    if (redoStack.empty())
    {
        statusMessage = MyString("Already at newest change");
        return;
    }

    Snapshot current;
    current.row = cursorRow;
    current.col = cursorCol;

    for (int i = 0; i < lineCount(); i++)
    {
        current.texts.push_back(getLine(i).getText());
    }
    undoStack.push_back(current);

    Snapshot snap = redoStack.back();
    redoStack.pop_back();

    Chapter chapter;
    Section section;
    Paragraph para;
    para.getLine(0).setText(snap.texts[0]);

    for (int i = 1; i < (int)snap.texts.size(); i++)
    {
        para.addLine(Line(snap.texts[i]));
    }
    section.addParagraph(para);
    chapter.addSection(section);
    chapters.clear();
    chapters.push_back(chapter);
    rebuildLines();

    cursorRow = snap.row;
    cursorCol = snap.col;
    fixCursor();
    modified = true;
    statusMessage = MyString("Redo");
}

void Document::deleteCurrentLine()
{
    pushUndo();
    MyString text = getLine(cursorRow).getText();
    clipboardLines.clear();
    clipboardLines.push_back(text);
    wholeLine = true;
    removeLine(cursorRow);

    if (cursorRow >= lineCount())
    {
        cursorRow = lineCount() - 1;
    }
    cursorCol = 0;
    fixCursor();
    modified = true;
    lastCmd = REPEAT_DD;
}

void Document::deleteToEndOfLine()
{
    pushUndo();
    MyString removed = getLine(cursorRow).deleteToEnd(cursorCol);
    clipboardLines.clear();
    clipboardLines.push_back(removed);
    wholeLine = false;
    fixCursor();
    modified = true;
    lastCmd = REPEAT_D;
}

void Document::deleteChar()
{
    Line& line = getLine(cursorRow);

    if (line.length() == 0)
    {
        return;
    }

    pushUndo();
    line.deleteChar(cursorCol);
    fixCursor();
    modified = true;
    lastCmd = REPEAT_X;
}

void Document::toggleCase()
{
    Line& line = getLine(cursorRow);

    if (line.length() == 0)
    {
        return;
    }

    pushUndo();
    line.toggleCaseAt(cursorCol);

    if (cursorCol < line.length() - 1)
    {
        cursorCol++;
    }
    modified = true;
    lastCmd = REPEAT_TILDE;
}

void Document::yankCurrentLine()
{
    clipboardLines.clear();
    clipboardLines.push_back(getLine(cursorRow).getText());
    wholeLine = true;
    statusMessage = MyString("1 line yanked");
    lastCmd = REPEAT_YY;
}

void Document::pasteAfter()
{
    if (clipboardLines.empty())
    {
        return;
    }

    pushUndo();

    if (wholeLine)
    {
        int insertAt = cursorRow;

        for (int i = 0; i < (int)clipboardLines.size(); i++)
        {
            insertLineAfter(insertAt, Line(clipboardLines[i]));
            insertAt++;
        }
        cursorRow++;
        cursorCol = 0;
    }
    else
    {
        Line& line = getLine(cursorRow);
        int pos = line.length() > 0 ? cursorCol + 1 : 0;
        line.insertText(pos, clipboardLines[0]);
        cursorCol = pos;
    }

    fixCursor();
    modified = true;
    lastCmd = REPEAT_P;
}

void Document::pasteBefore()
{
    if (clipboardLines.empty())
    {
        return;
    }

    pushUndo();

    if (wholeLine)
    {
        int insertAt = cursorRow;
        for (int i = 0; i < (int)clipboardLines.size(); i++)
        {
            insertLineBefore(insertAt, Line(clipboardLines[i]));
            insertAt++;
        }
        cursorCol = 0;
    }
    else
    {
        Line& line = getLine(cursorRow);
        line.insertText(cursorCol, clipboardLines[0]);
    }

    fixCursor();
    modified = true;
    lastCmd = REPEAT_SHIFT_P;
}

void Document::repeatLastCommand()
{
    switch (lastCmd)
    {
        case REPEAT_DD: deleteCurrentLine(); 
            break;
        case REPEAT_D: deleteToEndOfLine(); 
            break;
        case REPEAT_X: deleteChar(); 
            break;
        case REPEAT_TILDE: toggleCase(); 
            break;
        case REPEAT_YY: yankCurrentLine(); 
            break;
        case REPEAT_P: pasteAfter(); 
            break;
        case REPEAT_SHIFT_P: pasteBefore(); 
            break;
        default: statusMessage = MyString("Nothing to repeat"); 
            break;
    }
}

void Document::typeChar(char c)
{
    Line& line = getLine(cursorRow);
    line.insertChar(cursorCol, c);
    cursorCol++;
    modified = true;
}

void Document::newLine()
{
    Line& line = getLine(cursorRow);
    MyString tail = line.splitAt(cursorCol);
    insertLineAfter(cursorRow, Line(tail));
    cursorRow++;
    cursorCol = 0;
    modified = true;
}

void Document::backspace()
{
    if (cursorCol > 0)
    {
        Line& line = getLine(cursorRow);
        line.deleteChar(cursorCol - 1);
        cursorCol--;
        modified = true;
    }
    else if (cursorRow > 0)
    {
        MyString text = getLine(cursorRow).getText();
        int newCol = getLine(cursorRow - 1).length();
        getLine(cursorRow - 1).appendText(text);
        removeLine(cursorRow);
        cursorRow--;
        cursorCol = newCol;
        modified = true;
    }
}

void Document::deleteForward()
{
    Line& line = getLine(cursorRow);
    if (cursorCol < line.length())
    {
        line.deleteChar(cursorCol);
        modified = true;
    }
    else if (cursorRow < lineCount() - 1)
    {
        MyString text = getLine(cursorRow + 1).getText();
        getLine(cursorRow).appendText(text);
        removeLine(cursorRow + 1);
        modified = true;
    }
}

void Document::indent()
{
    Line& line = getLine(cursorRow);
    line.insertText(cursorCol, MyString("    "));
    cursorCol += 4;
    modified = true;
}

void Document::unindent()
{
    Line& line = getLine(cursorRow);
    int removeCount = 0;

    for (int i = 0; i < 4 and i < line.length() and line.charAt(i) == ' '; i++)
    {
        removeCount++;
    }

    if (removeCount > 0)
    {
        line.deleteRange(0, removeCount);
        cursorCol -= removeCount;

        if (cursorCol < 0)
        {
            cursorCol = 0;
        }
        modified = true;
    }
}

void Document::startSelect()
{
    if (!selecting)
    {
        selecting = true;
        selAnchorRow = cursorRow;
        selAnchorCol = cursorCol;
    }
}

void Document::clearSelection()
{
    selecting = false;
}

void Document::getSelectionRange(int& r1, int& c1, int& r2, int& c2) const
{
    r1 = selAnchorRow;
    c1 = selAnchorCol;
    r2 = cursorRow;
    c2 = cursorCol;

    if (r1 > r2 or (r1 == r2 and c1 > c2))
    {
        int tmpRow = r1;
        int tmpCol = c1;
        r1 = r2;
        c1 = c2;
        r2 = tmpRow;
        c2 = tmpCol;
    }
}

void Document::copySelection()
{
    if (!selecting)
    {
        statusMessage = MyString("Nothing selected - hold Shift+Arrow to select text first");
        return;
    }

    int r1, c1, r2, c2;
    getSelectionRange(r1, c1, r2, c2);
    clipboardLines.clear();

    if (r1 == r2)
    {
        clipboardLines.push_back(getLine(r1).getText().substr(c1, c2 - c1));
    }
    else
    {
        clipboardLines.push_back(getLine(r1).getText().substr(c1, getLine(r1).length() - c1));

        for (int r = r1 + 1; r < r2; r++)
        {
            clipboardLines.push_back(getLine(r).getText());
        }
        clipboardLines.push_back(getLine(r2).getText().substr(0, c2));
    }

    wholeLine = false;
    statusMessage = MyString("Selection copied");
}

void Document::cutSelection()
{
    if (!selecting)
    {
        statusMessage = MyString("Nothing selected - hold Shift+Arrow to select text first");
        return;
    }

    pushUndo();
    copySelection();

    int r1, c1, r2, c2;
    getSelectionRange(r1, c1, r2, c2);

    if (r1 == r2)
    {
        getLine(r1).deleteRange(c1, c2 - c1);
    }
    else
    {
        MyString tail = getLine(r2).getText().substr(c2, getLine(r2).length() - c2);
        getLine(r1).deleteToEnd(c1);
        getLine(r1).appendText(tail);
        for (int r = r2; r > r1; r--)
        {
            removeLine(r);
        }
    }

    cursorRow = r1;
    cursorCol = c1;
    clearSelection();
    fixCursor();
    modified = true;
    statusMessage = MyString("Selection cut");
}

void Document::pasteHere()
{
    if (clipboardLines.empty())
    {
        statusMessage = MyString("Clipboard is empty - copy or cut something first");
        return;
    }

    pushUndo();
    if (clipboardLines.size() == 1)
    {
        getLine(cursorRow).insertText(cursorCol, clipboardLines[0]);
        cursorCol += clipboardLines[0].length();
    }
    else
    {
        Line& line = getLine(cursorRow);
        MyString tail = line.splitAt(cursorCol);
        line.appendText(clipboardLines[0]);

        int insertAt = cursorRow;
        for (int i = 1; i < (int)clipboardLines.size(); i++)
        {
            insertLineAfter(insertAt, Line(clipboardLines[i]));
            insertAt++;
        }

        getLine(insertAt).appendText(tail);
        cursorRow = insertAt;
        cursorCol = clipboardLines.back().length();
    }

    modified = true;
}

bool Document::searchFor(const MyString& pattern, bool forward)
{
    lastSearchPattern = pattern;
    lastSearchForward = forward;
    return forward ? findNext() : findPrevious();
}

bool Document::findNext()
{
    if (lastSearchPattern.empty())
    {
        return false;
    }

    int total = lineCount();
    for (int step = 1; step <= total; step++)
    {
        int r = (cursorRow + step) % total;
        int pos = getLine(r).getText().find(lastSearchPattern, 0);
        if (pos != -1)
        {
            cursorRow = r;
            cursorCol = pos;
            return true;
        }
    }

    int pos = getLine(cursorRow).getText().find(lastSearchPattern, cursorCol + 1);
    if (pos != -1)
    {
        cursorCol = pos;
        return true;
    }

    statusMessage = MyString("Pattern not found");
    return false;
}

bool Document::findPrevious()
{
    if (lastSearchPattern.empty())
    {
        return false;
    }

    int total = lineCount();
    for (int step = 1; step <= total; step++)
    {
        int r = ((cursorRow - step) % total + total) % total;
        MyString text = getLine(r).getText();

        int lastPos = -1;
        int from = 0;
        while (true)
        {
            int pos = text.find(lastSearchPattern, from);
            if (pos == -1)
            {
                break;
            }
            lastPos = pos;
            from = pos + 1;
        }

        if (lastPos != -1)
        {
            cursorRow = r;
            cursorCol = lastPos;
            return true;
        }
    }

    statusMessage = MyString("Pattern not found");
    return false;
}

void Document::replaceAll(const MyString& oldSub, const MyString& newSub)
{
    pushUndo();
    for (int i = 0; i < lineCount(); i++)
    {
        MyString replaced = getLine(i).getText().replaceAll(oldSub, newSub);
        getLine(i).setText(replaced);
    }
    modified = true;
}

MyString Document::executeCommand(const MyString& cmdIn)
{
    MyString cmd = cmdIn;
    const char* text = cmd.c_str();
    int len = cmd.length();

    if (cmd == MyString("w"))
    {
        return saveToFile() ? MyString("Saved") : MyString("Failed to save");
    }

    if (cmd == MyString("q"))
    {
        if (modified)
        {
            return MyString("No write since last change (use :q! to override)");
        }
        quitRequested = true;
        return MyString("");
    }

    if (cmd == MyString("q!"))
    {
        quitRequested = true;
        return MyString("");
    }

    if (cmd == MyString("wq"))
    {
        saveToFile();
        quitRequested = true;
        return MyString("");
    }

    if (cmd == MyString("set number"))
    {
        lineNumbersOn = true;
        return MyString("Line numbers on");
    }

    if (cmd == MyString("set nonumber"))
    {
        lineNumbersOn = false;
        return MyString("Line numbers off");
    }

    if (cmd == MyString("n"))
    {
        return findNext() ? MyString("") : MyString("Pattern not found");
    }

    if (cmd == MyString("N"))
    {
        return findPrevious() ? MyString("") : MyString("Pattern not found");
    }

    if (len > 0 and text[0] == '/')
    {
        MyString pattern = cmd.substr(1, len - 1);
        return searchFor(pattern, true) ? MyString("") : MyString("Pattern not found");
    }

    if (len > 0 and text[0] == '?')
    {
        MyString pattern = cmd.substr(1, len - 1);
        return searchFor(pattern, false) ? MyString("") : MyString("Pattern not found");
    }

    if (len > 2 and text[0] == '%' and text[1] == 's' and text[2] == '/')
    {
        MyString rest = cmd.substr(3, len - 3);

        int firstSlash = -1;
        for (int i = 0; i < rest.length(); i++)
        {
            if (rest[i] == '/')
            {
                firstSlash = i;
                break;
            }
        }
        if (firstSlash == -1)
        {
            return MyString("Malformed substitute command");
        }

        MyString oldSub = rest.substr(0, firstSlash);
        MyString afterOld = rest.substr(firstSlash + 1, rest.length() - firstSlash - 1);

        int secondSlash = -1;
        for (int i = 0; i < afterOld.length(); i++)
        {
            if (afterOld[i] == '/')
            {
                secondSlash = i;
                break;
            }
        }
        MyString newSub = (secondSlash == -1) ? afterOld : afterOld.substr(0, secondSlash);

        replaceAll(oldSub, newSub);
        return MyString("Substitution complete");
    }

    return MyString("Unknown command");
}
