#pragma once
#include "MyString.h"

class Line {
private:
    MyString content;

public:
    Line();
    Line(const char* text);
    Line(const MyString& text);

    int length() const;
    MyString getText() const;
    char charAt(int pos) const;

    void setText(const MyString& text);
    void setText(const char* text);

    void insertChar(int pos, char c);
    void insertText(int pos, const MyString& text);
    void deleteChar(int pos);
    void deleteRange(int start, int count);

    MyString deleteToEnd(int pos);
    MyString splitAt(int pos);

    void appendText(const MyString& text);
    void toggleCaseAt(int pos);
};