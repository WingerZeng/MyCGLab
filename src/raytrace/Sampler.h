#pragma once
#include "types.h"
#include "Randomer.h"
#include <memory>
namespace mcl {
	struct SamplerRequestInfo
	{
		SamplerRequestInfo()
			:n1d(0), n2d(0), samplePerPixel(0, 0) {}

		SamplerRequestInfo(int myn1d, int myn2d, Point2i nsamplePerPixel = Point2i(0, 0))
			:n1d(myn1d), n2d(myn2d), samplePerPixel(nsamplePerPixel) {}

		SamplerRequestInfo operator+(const SamplerRequestInfo& rhs) {
			return SamplerRequestInfo(n1d + rhs.n1d, n2d + rhs.n2d, samplePerPixel);
		}

		static SamplerRequestInfo max(const SamplerRequestInfo& lhs, const SamplerRequestInfo& rhs) {
			return SamplerRequestInfo(std::max(lhs.n1d, rhs.n1d), std::max(lhs.n2d, rhs.n2d), lhs.samplePerPixel);
		}

		int n1d = 0;
		int n2d = 0;
		Point2i samplePerPixel;
	};

	//采用strati+jitter方法生成采样点
	class Sampler
	{
	public:
		Sampler();
		void setRequest(SamplerRequestInfo info);
		void startPixel(const Point2i& pixel);
		void toNextSample();
		Float get1DSample();
		Point2f get2DSample();
		std::unique_ptr<Sampler> clone();

		Point2i samplePerPixel() {
			return request.samplePerPixel;
		}
	private:
		template <typename T>
		void shuffle(std::vector<T> &vec);

		std::vector<int> permuteTable;
		static const int TableSize = 1024;

		//第一维为样本维度，第二维为样本点
		std::vector<std::vector<Float>> samples1d;
		std::vector<std::vector<Point2f>> samples2d;
		int curSample		= 0;
		int cur1dSample		= 0;
		int cur2dSample		= 0;
		SamplerRequestInfo request;

		Randomer rdm;
	};

	template <typename T>
	void mcl::Sampler::shuffle(std::vector<T> &vec)
	{
		for (int i = 0; i < vec.size()-1; i++) {
			int selected = rdm.uniformIntInterval(i, vec.size());
			std::swap(vec[i], vec[selected]);
		}
	}

	/*******************
	*	工具函数定义
	*********************/
	inline Vector3f cosSampleHemisphere(const Point2f &rand2d) {
		Float phi = 2 * PI*rand2d.x();
		Float sinTheta = std::sqrt(rand2d.y());
		Float cosTheta = std::sqrt(1 - rand2d.y());
		return Vector3f(std::cos(phi)*sinTheta, std::sin(phi)*sinTheta, cosTheta);
	}
}

