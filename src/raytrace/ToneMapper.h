#pragma once
#include "mcl.h"
#include "types.h"

namespace mcl {
	class ToneMapper
	{
	public:
		virtual void map(Film& film) = 0;
	};

	class RegulateToneMapper : public ToneMapper
	{
	public:
		virtual void map(Film& film) override;
	private:

	};

	class SigmoidToneMapper: public RegulateToneMapper
	{
	public:
		virtual void map(Film& film) override;
	};

	class SqrtToneMapper : public RegulateToneMapper
	{
	public:
		virtual void map(Film& film) override;
	};

	class GammaCorrectMapper : public ToneMapper
	{
	public:
		virtual void map(Film& film) override;
	private:
		inline Float GammaCorrect(Float value) { // From PBRT-V3
			if (value <= 0.0031308f) return 12.92f * value;
			return 1.055f * std::pow(value, (Float)(1.f / 2.2f)) - 0.055f;
		}
	};
}

