#include "Texture.h"
#include <QFile>
#include <QFileInfo>
//#TODO1 移除对opencv的依赖
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
namespace mcl{
	inline Color3f getPixelColor(const cv::Mat& image, const int& type, const int& nchannel, int x, int y) {
		switch (nchannel)
		{
		case 1:
		{
			Float val;
			switch (type)
			{
			case CV_8U:
				//此处x对应列，y对应行
				val = image.at<uchar>(y, x) / 255.0f;
				break;

			case CV_8S:
				val = image.at<char>(y, x) / 127.0f;
				break;

			case CV_16U:
				val = image.at<ushort>(y, x) / 65535.0f;
				break;

			case CV_16S:
				val = image.at<short>(y, x) / 32767.0f;
				break;

			case CV_32S:
				val = image.at<int>(y, x) / 2147483647.0f;
				break;

			case CV_32F:
				val = image.at<float>(y, x);
				break;

			case CV_64F:
				val = image.at<double>(y, x);
				break;

			default:
				LOG(FATAL) << "error image type!";
				exit(0);
			}
			return Color3f(val);
		}


		case 3:
		{
			Color3f ret;
			switch (type)
			{
			case CV_8U:
			{
				cv::Vec3b vec = image.at<cv::Vec3b>(y, x);
				Float inv = 1.0f / 255;
				ret.x() = (uchar)vec[2] * inv;
				ret.y() = (uchar)vec[1] * inv;
				ret.z() = (uchar)vec[0] * inv;
				break;
			}
			case CV_8S:
			{
				cv::Vec3b vec = image.at<cv::Vec3b>(y, x);
				Float inv = 1.0f / 127;
				ret.x() = (char)vec[2] * inv;
				ret.y() = (char)vec[1] * inv;
				ret.z() = (char)vec[0] * inv;
				break;
			}
			case CV_16U:
			{
				cv::Vec3s vec = image.at<cv::Vec3s>(y, x);
				Float inv = 1.0f / 65535;
				ret.x() = (ushort)vec[2] * inv;
				ret.y() = (ushort)vec[1] * inv;
				ret.z() = (ushort)vec[0] * inv;
				break;
			}
			case CV_16S:
			{
				cv::Vec3s vec = image.at<cv::Vec3s>(y, x);
				Float inv = 1.0f / 32767;
				ret.x() = (short)vec[2] * inv;
				ret.y() = (short)vec[1] * inv;
				ret.z() = (short)vec[0] * inv;
				break;
			}
			case CV_32S:
			{
				cv::Vec3i vec = image.at<cv::Vec3i>(y, x);
				Float inv = 1.0f / 2147483647;
				ret.x() = (int)vec[2] * inv;
				ret.y() = (int)vec[1] * inv;
				ret.z() = (int)vec[0] * inv;
				break;
			}
			case CV_32F:
			{
				cv::Vec3f vec = image.at<cv::Vec3f>(y, x);
				ret.x() = vec[2];
				ret.y() = vec[1];
				ret.z() = vec[0];
				break;
			}
			case CV_64F:
			{
				cv::Vec3d vec = image.at<cv::Vec3d>(y, x);
				ret.x() = vec[2];
				ret.y() = vec[1];
				ret.z() = vec[0];
				break;
			}
			default:
				LOG(FATAL) << "error image type " << type << "!";
				exit(0);
			}
			return ret;
		}

		case 4:
		{
			Color3f ret;
			switch (type)
			{
			case CV_8U:
			{
				cv::Vec4b vec = image.at<cv::Vec4b>(y, x);
				Float inv = 1.0f / 255;
				ret.x() = (uchar)vec[2] * inv;
				ret.y() = (uchar)vec[1] * inv;
				ret.z() = (uchar)vec[0] * inv;
				break;
			}
			case CV_8S:
			{
				cv::Vec4b vec = image.at<cv::Vec4b>(y, x);
				Float inv = 1.0f / 127;
				ret.x() = (char)vec[2] * inv;
				ret.y() = (char)vec[1] * inv;
				ret.z() = (char)vec[0] * inv;
				break;
			}
			case CV_16U:
			{
				cv::Vec4s vec = image.at<cv::Vec4s>(y, x);
				Float inv = 1.0f / 65535;
				ret.x() = (ushort)vec[2] * inv;
				ret.y() = (ushort)vec[1] * inv;
				ret.z() = (ushort)vec[0] * inv;
				break;
			}
			case CV_16S:
			{
				cv::Vec4s vec = image.at<cv::Vec4s>(y, x);
				Float inv = 1.0f / 32767;
				ret.x() = (short)vec[2] * inv;
				ret.y() = (short)vec[1] * inv;
				ret.z() = (short)vec[0] * inv;
				break;
			}
			case CV_32S:
			{
				cv::Vec4i vec = image.at<cv::Vec4i>(y, x);
				Float inv = 1.0f / 2147483647;
				ret.x() = (int)vec[2] * inv;
				ret.y() = (int)vec[1] * inv;
				ret.z() = (int)vec[0] * inv;
				break;
			}
			case CV_32F:
			{
				cv::Vec4f vec = image.at<cv::Vec4f>(y, x);
				ret.x() = vec[2];
				ret.y() = vec[1];
				ret.z() = vec[0];
				break;
			}
			case CV_64F:
			{
				cv::Vec4d vec = image.at<cv::Vec4d>(y, x);
				ret.x() = vec[2];
				ret.y() = vec[1];
				ret.z() = vec[0];
				break;
			}
			default:
				LOG(FATAL) << "error image type " << type << "!";
				exit(0);
			}
			return ret;
		}

		default:
			LOG(FATAL) << "error channel number!";
		}
	}

	PixelMapTexture::PixelMapTexture(const QString& filename)
	{
		static bool first = true;
		CHECK(QFile(filename).exists());
		image = std::make_shared<cv::Mat>(cv::imread(filename.toStdString(), cv::IMREAD_UNCHANGED));
		CHECK(image->data);
		nchannels = image->channels();
		CHECK(nchannels == 1 || nchannels == 3 || nchannels == 4);
		type = image->depth();
	}

	mcl::Color3f PixelMapTexture::value(const SurfaceRecord& rec)
	{
		return value(rec.uv); //#TODO3 反走样
	}

	mcl::Color3f PixelMapTexture::value(const Point2f& uv)
	{
		int npx = image->cols;
		int npy = image->rows;
		Float fx = uv[0] * (npx - 1);
		Float fy = (1 - uv[1]) * (npy - 1);
		int ix = int(fx);
		int iy = int(fy);
		Float xw = fx - ix;
		Float nxw = 1 - xw;
		Float yw = fy - iy;
		Float nyw = 1 - yw;

		ix = ix % npx;
		if (ix < 0) ix += npx;
		iy = iy % npy;
		if (iy < 0) iy += npy;
		int nix = (ix + 1) % npx;
		int niy = (iy + 1) % npy;

		//双线性插值
		auto getColor = [this](const int& ix, const int& iy) {
			return getPixelColor(*(this->image), this->type, this->nchannels, ix, iy);
			return Color3f(0);
		};
		return (getColor(ix,iy) * nxw + (xw ? getColor(nix,iy) * xw : Color3f(0))) * nyw +
			(yw ? 
			(getColor(ix,niy) * nxw + (xw ? getColor(nix, niy) * xw : Color3f(0))) * yw
				: Color3f(0));
	}

}