#pragma once

#include <filesystem>
#include <vector>
#include <cstdint>
#include <map>

#include "Vector2D.hpp"

class SquareCellGrid {
    friend std::shared_ptr<SquareCellGrid> initializeGrid(const std::filesystem::path& imgPath);

public:
	SquareCellGrid(int w, int h, int boundarySC, int spaceSC);

    int getInteriorWidth() const { return interiorWidth; }
    int getInteriorHeight() const { return interiorHeight; }
    int getBoundaryWidth() const { return boundaryWidth; }
    int getBoundaryHeight() const { return boundaryHeight; }

	void setCell(int row, int col, int superCell);
	int divideCellShortAxis(int c);
	int cleaveCell(int c);
	void moveCell(int x, int y);
	void fullTextureRefresh();
    const std::vector<uint8_t>& getPixels() const;
    std::vector<std::pair<double, double>> computeCentroids() const;
    std::map<std::pair<int, int>, int> computeContacts(int minContactPixels = 3) const;

private:
    int interiorWidth;
    int interiorHeight;
    int boundaryWidth;
    int boundaryHeight;

    double LAMBDA = 0;
    double BOLTZ_TEMP = 0;

    std::vector<std::vector<int>> internalGrid;
    std::vector<uint8_t> pixels;

    double getAdhesionDelta(int sourceX, int sourceY, int destX, int destY);
    double getVolumeDelta(int sourceX, int sourceY, int destX, int destY);

    static std::array<Vector2D<int>, 8> getNeighboursCoords(int row, int col);
};