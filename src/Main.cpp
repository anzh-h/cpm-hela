#include <fstream>
#include <thread>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <optional>

#include <SFML/Graphics.hpp>

#include "CellType.hpp"
#include "ColourScheme.hpp"
#include "cxxopts.hpp"
#include "DivisionHandler.hpp"
#include "RandomNumberGenerators.hpp"
#include "split.hpp"
#include "SquareCellGrid.hpp"
#include "SuperCell.hpp"
#include "SuperCellTemplate.hpp"

unsigned int MCS_HOUR_EST;
unsigned int MAX_MCS;
unsigned int PIXEL_SCALE;
unsigned int RENDER_FPS;
unsigned int LOG_EVERY_N_MCS = 1;
unsigned int LOG_EVERY_N_MCS_2 = 67;

double BOLTZ_TEMP;
double LAMBDA;

std::mutex mData;
std::mutex mNext;
std::mutex mLowPriority;

void highPriorityLock() {
	mNext.lock();
	mData.lock();
	mNext.unlock();
}

void highPriorityUnlock() {
	mData.unlock();
}

void lowPriorityLock() {
	mLowPriority.lock();
	mNext.lock();
	mData.lock();
	mNext.unlock();
}

void lowPriorityUnlock() {
	mData.unlock();
	mLowPriority.unlock();
}

int simLoop(const std::shared_ptr<SquareCellGrid>& grid, std::atomic<bool>& done);
void readConfig(const std::filesystem::path& filePath);
std::shared_ptr<SquareCellGrid> initializeGrid(const std::filesystem::path& imgPath);

std::map<int, int> templateColourMap;

int main(int argc, char *argv[]) {
    cxxopts::Options options("cpm_hela", "Cellular Potts Model Simulation");
    options.add_options()
            ("h, headless", "Run in headless mode", cxxopts::value<bool>()->default_value("false"))
            ("f, file", "Configuration file name to load", cxxopts::value<std::string>()->default_value("default"));

    auto result = options.parse(argc, argv);
    bool isHeadless = result["h"].as<bool>();
    std::string loadName = result["f"].as<std::string>();

    std::cout << "Loading configuration: " << loadName << ".cfg...\n";
    readConfig(loadName + ".cfg");

    auto grid = initializeGrid(loadName + ".pgm");

    if (!isHeadless) {
        sf::Texture displayTexture;
        auto texWidth = static_cast<unsigned int>(grid->getBoundaryWidth());
        auto texHeight = static_cast<unsigned int>(grid->getBoundaryHeight());
        if (!displayTexture.resize(sf::Vector2u{texWidth, texHeight})) {
            std::cerr << "Failed to create texture\n";
            return -1;
        }
        sf::Sprite sprite(displayTexture);
        sprite.setScale({static_cast<float>(PIXEL_SCALE), static_cast<float>(PIXEL_SCALE)});

        auto updateDisplayTexture = [&] {
            highPriorityLock();
            grid->fullTextureRefresh();
            highPriorityUnlock();
            displayTexture.update(grid->getPixels().data());
        };

        std::atomic<bool> done = false;
        std::thread simThread(simLoop, grid, std::ref(done));

        auto winWidth = static_cast<unsigned int>(PIXEL_SCALE * grid->getBoundaryWidth());
        auto winHeight = static_cast<unsigned int>(PIXEL_SCALE * grid->getBoundaryHeight());
        sf::RenderWindow window(sf::VideoMode({winWidth, winHeight}), "CPM Simulation");
        window.setFramerateLimit(RENDER_FPS);

        while (window.isOpen()) {
            while (const std::optional event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    done = true;
                }
            }

            updateDisplayTexture();
            window.clear();
            window.draw(sprite);
            window.display();

            if (done) {
                simThread.join();
                window.close();
            }
        }
    } else {
        std::cout << "Starting simulation in Headless mode...\n";
        std::atomic<bool> done = false;
        simLoop(grid, std::ref(done));
    }

    std::cout << "Simulation finished successfully.\n";
    return 0;
}

int simLoop(const std::shared_ptr<SquareCellGrid>& grid, std::atomic<bool>& done) {
    std::ofstream posLog("sim_positions.csv");
    std::ofstream velLog("sim_positions_2.csv");
    std::ofstream contactsLog("sim_contacts.csv");
    posLog << "MCS,SUPER_ID,X,Y,GENERATION\n";
    velLog << "MCS,SUPER_ID,X,Y,GENERATION\n";
    contactsLog << "MCS,CELL_A,CELL_B,CONTACT_LENGTH\n";

	unsigned int iMCS = grid->getInteriorWidth() * grid->getInteriorHeight();

	for (unsigned int m = 0; m < MAX_MCS; m++) {
        lowPriorityLock();

		if (done) {
			lowPriorityUnlock();
			break;
		}

		for (unsigned int i = 0; i < iMCS; i++) {
			int x = RandomNumberGenerators::rUnifInt(1, grid->getInteriorWidth());
			int y = RandomNumberGenerators::rUnifInt(1, grid->getInteriorHeight());
			grid->moveCell(x, y);
		}

        DivisionHandler::runDivisionLoop(*grid);

        bool log_pos = (m % LOG_EVERY_N_MCS == 0);
        bool log_vel = (m % LOG_EVERY_N_MCS_2 == 0);

        if (log_pos || log_vel) {
            auto centroids = grid->computeCentroids();
            size_t numCells = centroids.size();

            for (size_t cell_id = 0; cell_id < numCells; cell_id++) {
                auto& [posX, posY] = centroids[cell_id];
                int generation = SuperCell::getGeneration(static_cast<int>(cell_id));

                if (log_pos) {
                    posLog << m << "," << cell_id << "," << posX << "," << posY << "," << generation << "\n";
                }
                if (log_vel) {
                    velLog << m << "," << cell_id << "," << posX << "," << posY << "," << generation << "\n";
                }
            }

            if (log_vel) {
                auto contacts = grid->computeContacts(3);
                for (auto& [pairKey, length] : contacts) {
                    contactsLog << m << "," << pairKey.first << "," << pairKey.second << "," << length << "\n";
                }
            }
        }

        lowPriorityUnlock();
		SuperCell::increaseLastDiv();
	}

    posLog.close();
    velLog.close();
    contactsLog.close();

	done = true;
	return 0;
}

void readConfig(const std::filesystem::path& filePath) {
    std::ifstream ifs(filePath);
    if (!ifs) {
        std::cerr << "Failed to open file for reading: " << filePath << '\n';
        return;
    }

	std::string line;
	int lineNumber = 0;

	while (std::getline(ifs, line)) {
		lineNumber++;
        if (line.empty() || line[0] == '#') continue;

		auto V = split(line, ',');

		if (V[0] == "SIM_PARAM") {
			const std::string& P = V[1];
			const std::string& value = V[2];

            if      (P == "MCS_HOUR_EST") MCS_HOUR_EST = static_cast<unsigned int>(std::stoi(value));
            else if (P == "MAX_HOURS")    MAX_MCS = std::stoi(value) * MCS_HOUR_EST;
            else if (P == "PIXEL_SCALE")  PIXEL_SCALE = std::stoi(value);
            else if (P == "FPS")          RENDER_FPS = std::stoi(value);
            else if (P == "BOLTZ_TEMP")   BOLTZ_TEMP = std::stoi(value);
            else if (P == "LAMBDA")       LAMBDA = std::stod(value);

		} else if (V[0] == "CELL_TYPE") {
			CellType T(std::stoi(V[1]));

			while (std::getline(ifs, line)) {
				lineNumber++;
                if (line == "END_TYPE") break;

				V = split(line, ',');
                const std::string& P = V[0];

				if (P == "J") {
					std::vector<std::string> J = split(V[1], ':');
					for (const std::string& s : J) {
						T.J.push_back(std::stod(s));
					}
				} else if (P == "DO_DIVIDE")   T.doesDivide = (V[1] == "1");
				else if (P == "IS_STATIC")     T.isStatic = (V[1] == "1");
				else if (P == "IGNORE_VOLUME") T.ignoreVolume = (V[1] == "1");
				else if (P == "DIV_MEAN")      T.divMean = std::stod(V[1]) * MCS_HOUR_EST;
				else if (P == "DIV_SD")        T.divSD = std::stod(V[1]) * MCS_HOUR_EST;
				else if (P == "DIV_TYPE")      T.divType = std::stoi(V[1]);
				else if (P == "DIV_MIN_VOL")   T.divMinVolume = std::stoi(V[1]);
				else if (P == "DIV_MIN_RATIO") T.divMinRatio = std::stod(V[1]);
				else if (P == "COLOUR")        T.colourScheme = std::stoi(V[1]);
			}

			CellType::addType(std::move(T));

		} else if (V[0] == "TEMPLATE") {
			SuperCellTemplate T(std::stoi(V[1]));

			while (std::getline(ifs, line)) {
				lineNumber++;
                if (line == "END_TEMPLATE") break;

				V = split(line, ',');
				const std::string P = V[0];

				if (P == "TYPE")         T.type = std::stoi(V[1]);
				else if (P == "VOLUME")  T.volume = std::stoi(V[1]);
				else if (P == "SPECIAL") T.specialType = std::stoi(V[1]);
			}

			if (T.type != -1) SuperCellTemplate::addTemplate(T);

        } else if (V[0] == "MAP_TEMPLATE") {
            int startVal = std::stoi(V[1]);
            int tID = std::stoi(V[2]);

            if (V.size() > 3) {
                int count = std::stoi(V[3]);
                for (int i = startVal; i < startVal + count; i++) {
                    // Skip TrackMate IDs that were filtered out during dataset segmentation
                    if (i == 6 || i == 17 || i == 56 || i == 68) continue;
                    templateColourMap[i] = tID;
                }
            } else {
                templateColourMap[startVal] = tID;
            }

        } else if (V[0] == "COLOUR_SCHEME") {
            ColourScheme CS(std::stoi(V[1]));

            while (std::getline(ifs, line)) {
                lineNumber++;
                if (line == "END_COLOUR") break;

                V = split(line, ',');
                const std::string P = V[0];

                if (P == "R") {
                    CS.rMin = std::stoi(V[1]);
                    CS.rMax = std::stoi(V[2]);
                } else if (P == "G") {
                    CS.gMin = std::stoi(V[1]);
                    CS.gMax = std::stoi(V[2]);
                } else if (P == "B") {
                    CS.bMin = std::stoi(V[1]);
                    CS.bMax = std::stoi(V[2]);
                }
            }

            ColourScheme::addScheme(CS);

		} else {
			std::cout << "Unrecognised tag " << V[0] << " on line " << lineNumber << std::endl;
		}
	}
}

std::shared_ptr<SquareCellGrid> initializeGrid(const std::filesystem::path& imgPath) {
    std::ifstream ifs(imgPath);
    if (!ifs) {
        std::cerr << "Failed to open file for reading: " << imgPath << '\n';
        return nullptr;
    }

	std::string pgmString;
	getline(ifs, pgmString);
	getline(ifs, pgmString);
	getline(ifs, pgmString);
	auto widthHeight = split(pgmString, ' ');
	int SIM_WIDTH = std::stoi(widthHeight[0]);
	int SIM_HEIGHT = std::stoi(widthHeight[1]);
	getline(ifs, pgmString);
    ifs.get();

    int boundarySuper = 0, spaceSuper = 1;
	std::shared_ptr<SquareCellGrid> grid(nullptr);
    std::map<int, int> tempSuperMap;

    for (const auto& [cVal, superTemplate] : templateColourMap) {
        tempSuperMap[cVal] = SuperCell::makeNewSuperCell(SuperCellTemplate::getTemplate(superTemplate));
        int sType = SuperCellTemplate::getTemplate(superTemplate).specialType;

        if (sType == 1)      boundarySuper = tempSuperMap[cVal];
        else if (sType == 2) spaceSuper = tempSuperMap[cVal];
    }

    grid = std::make_shared<SquareCellGrid>(SIM_WIDTH, SIM_HEIGHT, boundarySuper, spaceSuper);

    for (int y = 1; y <= grid->getInteriorHeight(); y++) {
        for (int x = 1; x <= grid->getInteriorWidth(); x++) {
            auto it = tempSuperMap.find(ifs.get());
            if (it != tempSuperMap.end()) grid->setCell(x, y, it->second);
            else                          grid->setCell(x, y, 0);
        }
    }

	for (int c = 0; c < SuperCell::getNumSupers(); c++) {
		SuperCell::generateAndSetNewColour(c);
		if (SuperCell::doDivide(c)) SuperCell::setNextDiv(c, SuperCell::generateNewDivisionTime(c));
	}

	grid->BOLTZ_TEMP = BOLTZ_TEMP;
	grid->LAMBDA = LAMBDA;

	return grid;
}