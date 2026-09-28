#include <cmath>
#include <array>
#include <cstring>

#include "SquareCellGrid.hpp"
#include "RandomNumberGenerators.hpp"
#include "SuperCell.hpp"

SquareCellGrid::SquareCellGrid(int w, int h, int boundarySC, int spaceSC):
        interiorWidth(w), interiorHeight(h), boundaryWidth(w + 2), boundaryHeight(h + 2),
        internalGrid(w + 2, std::vector<int>(h + 2, spaceSC)), pixels((w + 2) * (h + 2) * 4) {

    for (int x = 0; x < boundaryWidth; x++) {
        internalGrid[x][0] = boundarySC;
        internalGrid[x][boundaryHeight - 1] = boundarySC;
    }

    for (int y = 0; y < boundaryHeight; y++) {
        internalGrid[0][y] = boundarySC;
        internalGrid[boundaryWidth - 1][y] = boundarySC;
    }

    const int interiorVolume = interiorWidth * interiorHeight;
    const int totalVolume = boundaryWidth * boundaryHeight;

    SuperCell::setVolume(spaceSC, interiorVolume);
    SuperCell::setVolume(boundarySC, totalVolume - interiorVolume);
}

void SquareCellGrid::setCell(int x, int y, int superCell) {
    if (internalGrid[x][y] == superCell) return;

    int originalSuper = internalGrid[x][y];

    SuperCell::changeVolume(originalSuper, -1);
    SuperCell::changeVolume(superCell, 1);

    internalGrid[x][y] = superCell;
}

int SquareCellGrid::divideCellShortAxis(int c) {
	std::vector<Vector2D<int>> cellList;
    cellList.reserve(SuperCell::getVolume(c));

	for (int x = 1; x <= interiorWidth; x++) {
		for (int y = 1; y <= interiorHeight; y++) {
			if (internalGrid[x][y] == c) {
				cellList.emplace_back(x, y);
			}
		}
	}

	if (cellList.size() <= 1) return -1;

    auto m00 = static_cast<double>(cellList.size());
    double m10 = 0, m01 = 0, m20 = 0, m02 = 0, m11 = 0;

    for (const auto& p : cellList) {
        double x = p[0], y = p[1];
        m10 += x, m01 += y, m20 += x * x, m02 += y * y, m11 += x * y;
    }

	double xBar = m10 / m00, yBar = m01 / m00;

	double mu20 = m20 / m00 - xBar * xBar;
	double mu02 = m02 / m00 - yBar * yBar;
	double mu11 = m11 / m00 - xBar * yBar;

	double covTrace = mu20 + mu02;
	double covDet = mu20 * mu02 - mu11 * mu11;

    double discriminant = std::sqrt(covTrace * covTrace - 4 * covDet);
	double eigA = (covTrace + discriminant) * 0.5;
	double eigB = (covTrace - discriminant) * 0.5;

	double smallEig = std::min(std::abs(eigA), std::abs(eigB));
    double largeEig = std::max(std::abs(eigA), std::abs(eigB));

    int newSuperCell = -1;
    double minRatio = SuperCell::getDivMinRatio(c);

    if (smallEig > 0 && (largeEig / smallEig) > minRatio) {
        Vector2D<double> eigVec(mu11, smallEig - mu20);
        double grad = eigVec[1] / eigVec[0];

        SuperCell::increaseGeneration(c);
        newSuperCell = SuperCell::makeNewSuperCell(c);
        SuperCell::setLastDiv(newSuperCell, 0);

        for (const auto& k : cellList) {
            if (k[1] > grad * (k[0] - xBar) + yBar) {
                setCell(k[0], k[1], newSuperCell);
            }
        }
    }

	SuperCell::setLastDiv(c, 0);
	return newSuperCell;
}

int SquareCellGrid::cleaveCell(int c) {
    int superCellB = divideCellShortAxis(c);
    if (superCellB == -1) return -1;

    int targetVolume = SuperCell::getTargetVolume(c);
    int halfVolume = targetVolume / 2;

    SuperCell::setTargetVolume(c, halfVolume);
    SuperCell::setTargetVolume(superCellB, targetVolume - halfVolume);

    return superCellB;
}

void SquareCellGrid::moveCell(int x, int y) {
    auto neighbours = getNeighboursCoords(x, y);

    int r = RandomNumberGenerators::rUnifInt(0, 7);

    int targetX = neighbours[r][0], targetY = neighbours[r][1];
    int origin = internalGrid[x][y], target = internalGrid[targetX][targetY];

    if (origin == target || SuperCell::isStatic(origin) || SuperCell::isStatic(target)) return;

    double deltaH = getAdhesionDelta(x, y, targetX, targetY)
                    + LAMBDA * getVolumeDelta(x, y, targetX, targetY);

    if (deltaH <= 0 || RandomNumberGenerators::rUnifProb() < std::exp(-deltaH / BOLTZ_TEMP)) {
        setCell(targetX, targetY, origin);
    }
}

std::array<Vector2D<int>, 8> SquareCellGrid::getNeighboursCoords(int row, int col) {
    return {
            Vector2D<int>{row - 1, col - 1}, Vector2D<int>{row - 1, col}, Vector2D<int>{row - 1, col + 1},
            Vector2D<int>{row, col - 1}, Vector2D<int>{row, col + 1},
            Vector2D<int>{row + 1, col - 1}, Vector2D<int>{row + 1, col}, Vector2D<int>{row + 1, col + 1}
    };
}

double SquareCellGrid::getAdhesionDelta(int sourceX, int sourceY, int destX, int destY) {
	int sourceSuper = internalGrid[sourceX][sourceY];
	int destSuper = internalGrid[destX][destY];

	const auto& sourceJ = SuperCell::getJ(sourceSuper);
	const auto& destJ = SuperCell::getJ(destSuper);

	double initAD = 0, postAD = 0;
	auto neighbours = getNeighboursCoords(destX, destY);

	for (int i = 0; i < 8; i++) {
		int nSuper = internalGrid[neighbours[i][0]][neighbours[i][1]];
		int nType = SuperCell::getCellType(nSuper);

		if (nSuper != destSuper) initAD += destJ[nType];
        if (nSuper != sourceSuper) postAD += sourceJ[nType];
	}

	return postAD - initAD;
}

double SquareCellGrid::getVolumeDelta(int sourceX, int sourceY, int destX, int destY) {
	int destSuper = internalGrid[destX][destY];
    int sourceSuper = internalGrid[sourceX][sourceY];

	if (SuperCell::getVolume(destSuper) == 1) return 1e6;

	double volumeD = 0;

    if (!SuperCell::ignoreVolume(sourceSuper)) {
        volumeD += 2 * (SuperCell::getVolume(sourceSuper) - SuperCell::getTargetVolume(sourceSuper)) + 1;
    }

    if (!SuperCell::ignoreVolume(destSuper)) {
        volumeD += -2 * (SuperCell::getVolume(destSuper) - SuperCell::getTargetVolume(destSuper)) + 1;
    }

	return volumeD;
}

void SquareCellGrid::fullTextureRefresh() {
    uint8_t* ptr = pixels.data();

    for (int y = 0; y < boundaryHeight; y++) {
        for (int x = 0; x < boundaryWidth; x++) {
            int cellId = internalGrid[x][y];
            std::memcpy(ptr, SuperCell::getColour(cellId).data(), 4);
            ptr += 4;
        }
    }
}

const std::vector<uint8_t>& SquareCellGrid::getPixels() const{
	return pixels;
}

std::vector<std::pair<double, double>> SquareCellGrid::computeCentroids() const {
    struct Accumulator {
        double sumX = 0;
        double sumY = 0;
        int count = 0;

        void add(double x, double y) {
            sumX += x;
            sumY += y;
            count++;
        }
    };

    int numCells = SuperCell::getNumSupers();
    std::vector<Accumulator> acc(numCells);

    for (int y = 1; y <= interiorHeight; y++) {
        for (int x = 1; x <= interiorWidth; x++) {
            int id = internalGrid[x][y];
            if (SuperCell::getCellType(id) != 2) continue;
            acc[id].add(x, y);
        }
    }

    std::vector<std::pair<double, double>> centroids(numCells, {0.0, 0.0});

    for (int id = 0; id < numCells; id++) {
        double invCount = 1.0 / acc[id].count;
        centroids[id] = {acc[id].sumX * invCount, acc[id].sumY * invCount};
    }

    return centroids;
}

std::map<std::pair<int, int>, int> SquareCellGrid::computeContacts(int minContactPixels) const {
    int numCells = SuperCell::getNumSupers();
    std::vector<int> contactMatrix(numCells * numCells, 0);

    for (int y = 1; y <= interiorHeight; y++) {
        for (int x = 1; x <= interiorWidth; x++) {
            int currentID = internalGrid[x][y];
            if (SuperCell::getCellType(currentID) != 2) continue;

            auto addContact = [&](int neighborID) {
                if (SuperCell::getCellType(neighborID) == 2 && currentID != neighborID) {
                    int minId = std::min(currentID, neighborID);
                    int maxId = std::max(currentID, neighborID);
                    contactMatrix[minId * numCells + maxId]++;
                }
            };

            if (x + 1 <= interiorWidth)  addContact(internalGrid[x + 1][y]);
            if (y + 1 <= interiorHeight) addContact(internalGrid[x][y + 1]);
        }
    }

    std::map<std::pair<int, int>, int> filteredContacts;
    for (int i = 0; i < numCells; i++) {
        for (int j = i + 1; j < numCells; j++) {
            int length = contactMatrix[i * numCells + j];
            if (length >= minContactPixels) {
                filteredContacts.emplace(std::make_pair(i, j), length);
            }
        }
    }

    return filteredContacts;
}