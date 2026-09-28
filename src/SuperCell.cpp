#include "SuperCell.hpp"
#include "CellType.hpp"
#include "ColourScheme.hpp"
#include "RandomNumberGenerators.hpp"
#include "SuperCellTemplate.hpp"

std::vector<SuperCell> SuperCell::superCells;

int SuperCell::makeNewSuperCell(int type, int gen, int targetV) {
    superCells.push_back(SuperCell(type, gen, targetV));
    return getNumSupers() - 1;
}

int SuperCell::makeNewSuperCell(int c) {
	return SuperCell::makeNewSuperCell(SuperCell::getCellType(c), SuperCell::getGeneration(c), SuperCell::getTargetVolume(c));
}

int SuperCell::makeNewSuperCell(const SuperCellTemplate& t) {
	return SuperCell::makeNewSuperCell(t.type, 0, t.volume);
}

int SuperCell::getNumSupers() {
    return static_cast<int>(superCells.size());
}

bool SuperCell::doDivide(int c) {
    return CellType::getType(superCells[c].cellType).doesDivide;
}

bool SuperCell::isStatic(int c) {
	return CellType::getType(superCells[c].cellType).isStatic;
}

bool SuperCell::ignoreVolume(int c) {
	return CellType::getType(superCells[c].cellType).ignoreVolume;
}

double SuperCell::getDivMean(int c) {
	return CellType::getType(superCells[c].cellType).divMean;
}

double SuperCell::getDivSD(int c) {
	return CellType::getType(superCells[c].cellType).divSD;
}

int SuperCell::getDivType(int c) {
	return CellType::getType(superCells[c].cellType).divType;
}

int SuperCell::getDivMinVol(int c) {
	return CellType::getType(superCells[c].cellType).divMinVolume;
}

double SuperCell::getDivMinRatio(int c) {
	return CellType::getType(superCells[c].cellType).divMinRatio;
}

int SuperCell::getGeneration(int c) {
	return superCells[c].generation;
}

void SuperCell::increaseGeneration(int c) {
	superCells[c].generation++;
}

void SuperCell::setLastDiv(int c, int i) {
    superCells[c].lastDivMCS = i;
}

int SuperCell::getLastDiv(int c) {
	return superCells[c].lastDivMCS;
}

void SuperCell::increaseLastDiv() {
    for (auto& superCell : superCells) {
		superCell.lastDivMCS++;
	}
}

void SuperCell::setNextDiv(int c, int i) {
    superCells[c].nextDivMCS = i;
}

int SuperCell::getNextDiv(int c) {
	return superCells[c].nextDivMCS;
}

int SuperCell::generateNewDivisionTime(int c) {
    return static_cast<int>(RandomNumberGenerators::rNormalDouble(getDivMean(c), getDivSD(c)));
}

void SuperCell::setVolume(int c, int v) {
    superCells[c].volume = v;
}

int SuperCell::getVolume(int c) {
    return superCells[c].volume;
}

void SuperCell::changeVolume(int c, int delta) {
    superCells[c].volume += delta;
}

int SuperCell::getTargetVolume(int c) {
	return superCells[c].targetVolume;
}

void SuperCell::setTargetVolume(int c, int target) {
	superCells[c].targetVolume = target;
}

int SuperCell::getCellType(int c) {
	return superCells[c].cellType;
}

const std::vector<double>& SuperCell::getJ(int c) {
	return CellType::getType(superCells[c].cellType).J;
}

int SuperCell::getColourScheme(int c) {
	return CellType::getType(superCells[c].cellType).colourScheme;
}

const std::array<uint8_t, 4>& SuperCell::getColour(int c) {
    return superCells[c].colour;
}

void SuperCell::generateAndSetNewColour(int c) {
    superCells[c].colour = ColourScheme::generateColour(getColourScheme(c));
}