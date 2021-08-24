#include "Randomer.h"
#include <algorithm>
namespace mcl{
	
	Randomer::Randomer(int seed)
	{
		srand(seed);
	}

	void Randomer::setSeed(int seed)
	{
		srand(seed);
	}

	Float Randomer::uniform0_1()
	{
		return (rand() * 1.0f + 1) / (RAND_MAX + 2);
	}

	int Randomer::uniformIntInterval(int begin, int end)
	{
		return std::clamp(int(begin + (end - begin) * uniform0_1()), begin, end - 1);
	}

}