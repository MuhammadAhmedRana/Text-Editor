#pragma once
#include <vector>
#include "Paragraph.h"


class Section {
private:
    vector<Paragraph> paragraphs;

public:
    Section();

    int paragraphCount() const;
    Paragraph& getParagraph(int idx);
    const Paragraph& getParagraph(int idx) const;

    void addParagraph(const Paragraph& paragraph);
};