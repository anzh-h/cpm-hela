#include "ColourScheme.hpp"
#include "RandomNumberGenerators.hpp"

void ColourScheme::addScheme(const ColourScheme& scheme) {
    colourSchemes.insert_or_assign(scheme.id, scheme);
}

std::array<uint8_t, 4> ColourScheme::generateColour(int i) {
    std::array<uint8_t, 4> defaultColor{255, 255, 255, 255};
    if (i == -1) return defaultColor;

    if (auto it = colourSchemes.find(i); it != colourSchemes.end()) {
        const auto& CS = it->second;
        return std::array<uint8_t, 4>{
                static_cast<uint8_t>(RandomNumberGenerators::rUnifInt(CS.rMin, CS.rMax)),
                static_cast<uint8_t>(RandomNumberGenerators::rUnifInt(CS.gMin, CS.gMax)),
                static_cast<uint8_t>(RandomNumberGenerators::rUnifInt(CS.bMin, CS.bMax)),
                255
        };
    }

	return defaultColor;
}