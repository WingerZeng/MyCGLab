#include "ToneMapper.h"
#include "Film.h"
#include "algorithms.h"
namespace mcl{
	
	void SigmoidToneMapper::map(Film& film)
	{
		for (int x = 0; x < film.getSize().x(); x++) {
			for (int y = 0; y < film.getSize().y(); y++) {
				Color3f toned = film.getPixelColor(x, y);

				toned.x() = sigmoid(toned.x());
				toned.y() = sigmoid(toned.y());
				toned.z() = sigmoid(toned.z());
				film.setPixelColor(x, y, toned);
			}
		}

		RegulateToneMapper::map(film);
	}

	void RegulateToneMapper::map(Film& film)
	{
		Color3f max = film.getMaxColor();

		for (int x = 0; x < film.getSize().x(); x++) {
			for (int y = 0; y < film.getSize().y(); y++) {
				film.setPixelColor(x, y, film.getPixelColor(x, y) / max);
				assert(film.getPixelColor(x, y).x() <= 1);
			}
		}
	}

	void SqrtToneMapper::map(Film& film)
	{
		for (int x = 0; x < film.getSize().x(); x++) {
			for (int y = 0; y < film.getSize().y(); y++) {
				Color3f toned = film.getPixelColor(x, y);
				toned.x() = std::sqrt(toned.x());
				toned.y() = std::sqrt(toned.y());
				toned.z() = std::sqrt(toned.z());
				film.setPixelColor(x, y, toned);
			}
		}

		RegulateToneMapper::map(film);
	}

	void GammaCorrectMapper::map(Film& film)
	{
		for (int x = 0; x < film.getSize().x(); x++) {
			for (int y = 0; y < film.getSize().y(); y++) {
				Color3f toned = film.getPixelColor(x, y);
				toned.x() = GammaCorrect(toned.x());
				toned.y() = GammaCorrect(toned.y());
				toned.z() = GammaCorrect(toned.z());
				film.setPixelColor(x, y, toned);
			}
		}
	}

}