#include "CRandomSys.h"



CRandomSys::CRandomSys(int min, int max)
{
	std::random_device rd;
	gen = std::mt19937(rd());
	betAmountGen = std::uniform_int_distribution<>(min, max);
}


int CRandomSys::GetRandom()
{
	return betAmountGen(gen);
}