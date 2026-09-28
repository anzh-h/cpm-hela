#pragma once

#include <map>
#include <filesystem>

class SquareCellGrid;

class SuperCellTemplate {
    friend class SuperCell;
    friend void readConfig(const std::filesystem::path& filePath);
    friend std::shared_ptr<SquareCellGrid> initializeGrid(const std::filesystem::path& imgPath);

public:
    SuperCellTemplate() = default;
    explicit SuperCellTemplate(int id) : id(id) {}

	static bool addTemplate(const SuperCellTemplate& t);
	static const SuperCellTemplate& getTemplate(int i);

private:
    int id = -1;
    int type = -1;
    int volume = 0;
    int specialType = 0;

    inline static std::map<int, SuperCellTemplate> scTemplates;
};