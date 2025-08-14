#pragma once
#include <random>

class CRandomSys
{
public:
	CRandomSys(int min, int max);

private:
	std::mt19937 gen;
	std::uniform_int_distribution<> betAmountGen;

public:

	int GetRandom();
};