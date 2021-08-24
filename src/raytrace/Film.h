#pragma once
#include "mcl.h"
#include "types.h"

class QLabel;

namespace mcl {
	class Film
	{
	public:
		Film(int px, int py);
		Film(const Film& rhs);

		enum PixelDataType
		{
			PDT_COLOR = 0,
			PDT_NORMAL,
			PDT_ALBEDO
		};

		Transform deviceToScreen() const; //设备坐标系到屏幕坐标系转换
		Transform screenToDevice() const;
		inline void setPixelColor(int x, int y, const Color3f& color);
		inline void setPixelColor(Point2i xy, const Color3f& color);
		inline Color3f getPixelColor(int x, int y) { return pixel(x, y).color; }

		inline void setPixelData(int x, int y, const PixelData& data);
		inline void setPixelData(Point2i xy, const PixelData& data);
		inline PixelData getPixelData(int x, int y) const { return pixel(x, y); }

		Color3f getMaxColor();

		Point2i getSize() const { return Point2i{ npx_,npy_ }; }

		void saveToBmp(QString path);
		void renderToQLabel(QLabel* label, PixelDataType t = PDT_COLOR);

	private:
		inline PixelData& pixel(int x, int y) {
			DCHECK(x < npx_&&y < npy_);
			return pixels_[y*npx_ + x];
		}
		inline const PixelData& pixel(int x, int y) const{
			DCHECK(x < npx_&&y < npy_);
			return pixels_[y*npx_ + x];
		}

		int npx_, npy_;
		std::unique_ptr<PixelData[]> pixels_;
		std::unique_ptr<uchar[]> bits;
	};

	inline void Film::setPixelColor(int x, int y, const Color3f& color)
	{
		pixel(x, y).color = color;
	}

	inline void Film::setPixelColor(Point2i xy, const Color3f& color)
	{
		pixel(xy.x(), xy.y()).color = color;
	}

	inline void Film::setPixelData(int x, int y, const PixelData& data)
	{
		pixel(x, y) = data;
	}

	inline void Film::setPixelData(Point2i xy, const PixelData& data)
	{
		pixel(xy.x(), xy.y()) = data;
	}
}

