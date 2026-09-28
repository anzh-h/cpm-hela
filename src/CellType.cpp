#include "CellType.hpp"

void CellType::addType(CellType t) {
    cellTypes.insert_or_assign(t.id, std::move(t));
}

const CellType& CellType::getType(int i) {
    return cellTypes.at(i);
}