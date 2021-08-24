#include "Film.h"
#include <QPixmap>
#include <QBitmap>
#include <QImage>
#include <QLabel>
#include <algorithm>
namespace mcl{
	Film::Film(int px, int py)
		:npx_(px),npy_(py)
	{
		CHECK(px > 0 && py > 0);
		pixels_.reset(new PixelData[px*py]);

	}

	Film::Film(const Film& rhs)
	{
		npy_ = rhs.npy_;
		npx_ = rhs.npx_;
		pixels_.reset(new PixelData[npx_*npy_]);
		for (int i = 0; i < npx_; i++) {
			for (int j = 0; j < npy_; j++) {
				setPixelData(i, j, rhs.getPixelData(i, j));
			}
		}
	}

	Transform Film::deviceToScreen() const
	{
		return Transform::scale(Vector3f{ npx_ / 2.0f,npy_ / 2.0f,1 })*Transform::translate(Vector3f{ 1,1,0 })*Transform::scale(Vector3f{ 1,-1,1 });
	}

	Transform Film::screenToDevice() const
	{
		return deviceToScreen().inverse();
	}

	mcl::Color3f Film::getMaxColor()
	{
		Color3f ret (0,0,0);

		for (int y = 0; y < npy_; y++) {
			for (int x = 0; x < npx_; x++) {
				ret = ret.maxWith(getPixelColor(x, y));
			}
		}

		return ret;
	}

	void Film::saveToBmp(QString path)
	{
		int align = 4;
		bits.reset(new uchar[npx_*npy_*align]);
		for (int y = 0; y < npy_; y++) {
			for (int x = 0; x < npx_; x++) {
				auto i1 = bits[y*npx_ * align + x * align + 0] = uchar(std::clamp<int>(std::llroundf((pixel(x, y).color.z() * 255)), 0, 255));
				auto i2 = bits[y*npx_ * align + x * align + 1] = uchar(std::clamp<int>(std::llroundf((pixel(x, y).color.y() * 255)), 0, 255));
				auto i3 = bits[y*npx_ * align + x * align + 2] = uchar(std::clamp<int>(std::llroundf((pixel(x, y).color.x() * 255)), 0, 255));
				auto i4 = bits[y*npx_ * align + x * align + 3] = 255;
			}
		}
		QImage image(bits.get(), npx_, npy_, QImage::Format_ARGB32);
		QPixmap pixmap = QPixmap::fromImage(image);
		pixmap.save(path,"bmp");
	}

	void Film::renderToQLabel(QLabel* label, PixelDataType t)
	{
		int align = 4;
		bits.reset(new uchar[npx_*npy_*align]);
		for (int y = 0; y < npy_; y++) {
			for (int x = 0; x < npx_; x++) {
				Color3f data;
				switch (t)
				{
				case mcl::Film::PDT_COLOR:
					data = pixel(x, y).color;
					break;
				case mcl::Film::PDT_NORMAL:
					data = Color3f(pixel(x, y).normal);
					break;
				case mcl::Film::PDT_ALBEDO:
					data = pixel(x, y).albedo;
					break;
				default:
					break;
				}

				auto i1 = bits[y*npx_ * align + x * align + 0] = uchar(std::clamp<int>(std::llroundf((data.z() * 255)), 0, 255));
				auto i2 = bits[y*npx_ * align + x * align + 1] = uchar(std::clamp<int>(std::llroundf((data.y() * 255)), 0, 255));
				auto i3 = bits[y*npx_ * align + x * align + 2] = uchar(std::clamp<int>(std::llroundf((data.x() * 255)), 0, 255));
				auto i4 = bits[y*npx_ * align + x * align + 3] = 255;
			}
		}
		QImage image(bits.get(), npx_, npy_, QImage::Format_ARGB32);
		QSize picSize(label->width(), label->height());
		label->setPixmap(QPixmap::fromImage(image).scaled(picSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}
}