#pragma once
#include <vector>
#include "Section.h"


class Chapter {
private:
   vector<Section> sections;

public:
    Chapter();

    int sectionCount() const;
    Section& getSection(int idx);
    const Section& getSection(int idx) const;

    void addSection(const Section& section);
};