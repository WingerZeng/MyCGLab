#include "Sampler.h"

namespace mcl{
	
	Sampler::Sampler()
	{
		permuteTable.reserve(TableSize);
		for (int i = 0; i < TableSize; i++) {
			permuteTable.emplace_back(i);
		}
		shuffle(permuteTable);
	}

	void Sampler::setRequest(SamplerRequestInfo info)
	{
		request = info;
	}

	void Sampler::startPixel(const Point2i& pixel)
	{
		// init
		curSample = 0;
		cur1dSample = 0;
		cur2dSample = 0;
		samples1d.clear();
		samples2d.clear();
		samples1d.reserve(request.n1d);
		samples2d.reserve(request.n2d);

		int n1dPerPixel = request.samplePerPixel.x() * request.samplePerPixel.y();
		rdm.setSeed(permuteTable[(permuteTable[pixel.x() % TableSize] + pixel.y()) % TableSize]);

		//generate samples of the pixel
		for (int i = 0; i < request.n1d; i++) {
			samples1d.emplace_back();
			samples1d.back().reserve(n1dPerPixel);

			for (int j = 0; j < n1dPerPixel; j++) {
				Float interval = 1.0 / n1dPerPixel;
				samples1d.back().emplace_back((j + rdm.uniform0_1()) / n1dPerPixel);
			}
			shuffle(samples1d.back());
		}

		for (int i = 0; i < request.n2d; i++) {
			samples2d.emplace_back();
			samples2d.back().reserve(n1dPerPixel);

			for (int j = 0; j < request.samplePerPixel.y(); j++) {
				for (int k = 0; k < request.samplePerPixel.x(); k++) {
					samples2d.back().emplace_back((k + rdm.uniform0_1()) / request.samplePerPixel.x(), (j + rdm.uniform0_1()) / request.samplePerPixel.y());
				}
			}
			shuffle(samples2d.back());
		}
	}

	void Sampler::toNextSample()
	{
		curSample++;
		cur1dSample = 0;
		cur2dSample = 0;
	}

	Float Sampler::get1DSample()
	{
		if (cur1dSample >= samples1d.size()) return rdm.uniform0_1();
		return samples1d[cur1dSample++][curSample];
	}

	mcl::Point2f Sampler::get2DSample()
	{
		if (cur2dSample >= samples2d.size()) return Point2f(rdm.uniform0_1(),rdm.uniform0_1());
		return samples2d[cur2dSample++][curSample];
	}

	std::unique_ptr<mcl::Sampler> Sampler::clone()
	{
		std::unique_ptr<Sampler> ptr(new Sampler);
		ptr->setRequest(request);
		return ptr;
	}

}