#include <random>

#include "RandomNumberGenerators.hpp"

static std::mt19937 randGen(std::random_device{}());
static std::uniform_real_distribution<double> rUnif(0.0, 1.0);

double RandomNumberGenerators::rUnifProb() {
	return rUnif(randGen);
}

int RandomNumberGenerators::rUnifInt(int min, int max) {
	std::uniform_int_distribution<int> rInt(min, max);
	return rInt(randGen);
}

double RandomNumberGenerators::rNormalDouble(double mu, double stdev) {
	std::normal_distribution<double> rNorm(mu, stdev);
	return rNorm(randGen);
}