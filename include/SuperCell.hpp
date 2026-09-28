#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <limits>

class SuperCellTemplate;

class SuperCell {
public:
	static int makeNewSuperCell(int type, int gen, int targetV);
	static int makeNewSuperCell(int c);
	static int makeNewSuperCell(const SuperCellTemplate& t);

    static int getNumSupers();

	static bool doDivide(int c);
    static bool isStatic(int c);
	static bool ignoreVolume(int c);

	static double getDivMean(int c);
	static double getDivSD(int c);
	static int getDivType(int c);
	static int getDivMinVol(int c);
	static double getDivMinRatio(int c);

	static int getGeneration(int c);
	static void increaseGeneration(int c);

    static void setLastDiv(int c, int i);
	static int getLastDiv(int c);
	static void increaseLastDiv();

    static void setNextDiv(int c, int i);
	static int getNextDiv(int c);
    static int generateNewDivisionTime(int c);

    static void setVolume(int i, int v);
    static int getVolume(int i);
	static void changeVolume(int i, int delta);

    static int getTargetVolume(int c);
    static void setTargetVolume(int i, int target);

	static int getCellType(int c);
	static const std::vector<double>& getJ(int c);

	static int getColourScheme(int c);
    static const std::array<uint8_t, 4>& getColour(int c);
	static void generateAndSetNewColour(int c);

private:
	int ID = static_cast<int>(superCells.size());
    int cellType;
	int generation;

    int volume = 0;
	int targetVolume;

	int lastDivMCS = 0;
	int nextDivMCS = std::numeric_limits<int>::max();

    SuperCell(int type, int gen, int targetV): cellType(type), generation(gen), targetVolume(targetV) {}

    std::array<uint8_t, 4> colour{0, 0, 0, 255};
    static std::vector<SuperCell> superCells;
};