#include "Chapter.h"

Chapter::Chapter() {}

int Chapter::sectionCount() const
{
    return (int)sections.size();
}

Section& Chapter::getSection(int idx)
{
    return sections[idx];
}

const Section& Chapter::getSection(int idx) const
{
    return sections[idx];
}

void Chapter::addSection(const Section& section)
{
    sections.push_back(section);
}