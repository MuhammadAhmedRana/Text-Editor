#include "Paragraph.h"

Paragraph::Paragraph()
{
    lines.push_back(Line());
}

int Paragraph::lineCount() const
{
    return (int)lines.size();
}

Line& Paragraph::getLine(int idx)
{
    return lines[idx];
}

const Line& Paragraph::getLine(int idx) const
{
    return lines[idx];
}

void Paragraph::addLine(const Line& line)
{
    lines.push_back(line);
}

void Paragraph::insertLine(int idx, const Line& line)
{
    if (idx < 0)
    {
        idx = 0;
    }

    if (idx > (int)lines.size())
    {
        idx = (int)lines.size();
    }

    lines.insert(lines.begin() + idx, line);
}

void Paragraph::removeLine(int idx)
{
    if (idx < 0 or idx >= lines.size())
    {
        return;
    }

    if (lines.size() == 1)
    {
        lines[idx].setText("");
        return;
    }

    lines.erase(lines.begin() + idx);
}