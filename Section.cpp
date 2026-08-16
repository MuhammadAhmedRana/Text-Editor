#include "Section.h"

Section::Section() {}

int Section::paragraphCount() const
{
    return (int)paragraphs.size();
}

Paragraph& Section::getParagraph(int idx)
{
    return paragraphs[idx];
}

const Paragraph& Section::getParagraph(int idx) const
{
    return paragraphs[idx];
}

void Section::addParagraph(const Paragraph& paragraph)
{
    paragraphs.push_back(paragraph);
}