#include "SuperCellTemplate.hpp"

bool SuperCellTemplate::addTemplate(const SuperCellTemplate& t) {
    auto [iter, inserted] = scTemplates.try_emplace(t.id, t);
    return inserted;
}

const SuperCellTemplate& SuperCellTemplate::getTemplate(int i) {
    return scTemplates.at(i);
}