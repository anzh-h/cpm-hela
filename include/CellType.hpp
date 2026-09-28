#pragma once

#include <filesystem>
#include <vector>
#include <map>

class CellType {
    friend class SuperCell;
    friend void readConfig(const std::filesystem::path& filePath);

public:
    explicit CellType(int id) : id(id) {}
	static void addType(CellType t);
	static const CellType& getType(int i);

private:
    int id;
    std::vector<double> J;

    bool doesDivide = false;
    bool isStatic = false;
    bool ignoreVolume = false;

    double divMean = 0;
    double divSD = 0;
    int divType = 0;
    int divMinVolume = 0;
    double divMinRatio = 0.0;

    int colourScheme = -1;

    inline static std::map<int, CellType> cellTypes;
};