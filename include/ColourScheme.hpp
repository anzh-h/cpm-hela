#pragma once

#include <array>
#include <cstdint>
#include <map>

class ColourScheme {
public:
	int id;

	int rMin = 0;
	int rMax = 255;

	int gMin = 0;
	int gMax = 255;

	int bMin = 0;
	int bMax = 255;

	explicit ColourScheme(int id) : id(id) {}
	static void addScheme(const ColourScheme& scheme);
    static std::array<uint8_t, 4> generateColour(int i);

private:
    inline static std::map<int, ColourScheme> colourSchemes;
};