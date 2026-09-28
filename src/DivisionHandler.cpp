#include "DivisionHandler.hpp"
#include "SuperCell.hpp"
#include "SquareCellGrid.hpp"

namespace DivisionHandler {

    void runDivisionLoop(SquareCellGrid& grid) {
        const int totalSupers = SuperCell::getNumSupers();
        for (int c = 0; c < totalSupers; c++) {
            if (!SuperCell::doDivide(c) || SuperCell::getLastDiv(c) <= SuperCell::getNextDiv(c) ||
                SuperCell::getVolume(c) < SuperCell::getDivMinVol(c)) {
                continue;
            }

            const int divType = SuperCell::getDivType(c);
            int newSuper = -1;

            switch (divType) {
                case 1:
                    newSuper = grid.divideCellShortAxis(c);
                    break;
                case 2:
                    newSuper = grid.cleaveCell(c);
                    break;
                default:
                    break;
            }

            if (newSuper != -1) {
                SuperCell::setNextDiv(newSuper, SuperCell::generateNewDivisionTime(c));
                SuperCell::generateAndSetNewColour(newSuper);
            }

            SuperCell::setNextDiv(c, SuperCell::generateNewDivisionTime(c));
        }
    }

}