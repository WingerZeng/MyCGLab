#pragma once
#include "types.h"
#include "Singleton.h"
namespace mcl {

	class Randomer
	{
	public:
		Randomer(int seed = 0);
		void setSeed(int seed);
		Float uniform0_1();
		int uniformIntInterval(int begin,int end);
	};
}

