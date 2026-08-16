#include "Line.h"

Line::Line()
{
    content = "";
}

Line::Line(const char* text)
{
    content = text;
}

Line::Line(const MyString& text)
{
    content = text;
}

int Line::length() const
{
    return content.length();
}

MyString Line::getText() const
{
    return content;
}

char Line::charAt(int pos) const
{
    if (pos < 0 or pos >= content.length())
    {
        return '\0';
    }
    return content[pos];
}

void Line::setText(const MyString& text)
{
    content = text;
}

void Line::setText(const char* text)
{
    content = MyString(text);
}

void Line::insertChar(int pos, char c)
{
    content.insertAt(pos, c);
}

void Line::insertText(int pos, const MyString& text)
{
    content.insertAt(pos, text);
}

void Line::deleteChar(int pos)
{
    content.eraseAt(pos, 1);
}

void Line::deleteRange(int start, int count)
{
    content.eraseAt(start, count);
}

MyString Line::deleteToEnd(int pos)
{
    MyString removed = content.substr(pos, content.length() - pos);
    content.eraseAt(pos, content.length() - pos);
    return removed;
}

MyString Line::splitAt(int pos)
{
    MyString tail = content.substr(pos, content.length() - pos);
    content.eraseAt(pos, content.length() - pos);
    return tail;
}

void Line::appendText(const MyString& text)
{
    content += text;
}

void Line::toggleCaseAt(int pos)
{
    content.toggleCaseChar(pos);
}