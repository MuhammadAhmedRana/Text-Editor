#pragma once
#include <vector>
#include "Line.h"


class Paragraph {
private:
    vector<Line> lines;

public:
    Paragraph();

    int lineCount() const;
    Line& getLine(int idx);
    const Line& getLine(int idx) const;

    void addLine(const Line& line);
    void insertLine(int idx, const Line& line);
    void removeLine(int idx);
};