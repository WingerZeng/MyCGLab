#pragma once
#include "mcl.h"
#include "types.h"
#include "Record.h"
#include <QImage>
namespace cv {
	class Mat;
}
namespace mcl {
	template<typename T>
	class Texture
	{
	public:
		virtual T value(const SurfaceRecord& rec) = 0;
		virtual T value(const Point2f& uv) = 0;

		virtual ~Texture() {};
	};

	template<typename T>
	class ConstantTexture :public Texture<T>
	{
	public:
		ConstantTexture(const T& val);
		T value(const SurfaceRecord& rec) override;
		T value(const Point2f& uv) override;
	private:
		T val;
	};
	//#TODO0 reverse Gamma
	class PixelMapTexture:public Texture<Color3f>
	{
	public:
		PixelMapTexture(const QString& filename);
		Color3f value(const SurfaceRecord& rec) override;
		Color3f value(const Point2f& uv) override;
	private:
		std::shared_ptr<cv::Mat> image;
		int type;
		int nchannels;
	};

	template<typename T>
	class AddTexture :public Texture<T>
	{
	public:
		AddTexture(const std::shared_ptr<Texture<T>> t1, const std::shared_ptr<Texture<T>> t2)
			:t1(t1), t2(t2) {}
		T value(const SurfaceRecord& rec) override;
		T value(const Point2f& uv) override;

	private:
		std::shared_ptr<Texture<T>> t1;
		std::shared_ptr<Texture<T>> t2;
	};

	template<typename T>
	class ProductTexture :public Texture<T>
	{
	public:
		ProductTexture(const std::shared_ptr<Texture<T>> t1, const std::shared_ptr<Texture<T>> t2)
			:t1(t1), t2(t2) {}
		T value(const SurfaceRecord& rec) override;
		T value(const Point2f& uv) override;

	private:
		std::shared_ptr<Texture<T>> t1;
		std::shared_ptr<Texture<T>> t2;
	};

	template<typename T>
	class DivideTextrue : public Texture<T>
	{
	public:
		DivideTextrue(const std::shared_ptr<Texture<T>>& tex, const Float& val)
			:tex(tex), val(val) {}
		T value(const SurfaceRecord& rec) override;
		T value(const Point2f& uv) override;

	private:
		std::shared_ptr<Texture<T>> tex;
		Float val;
	};

	/*************
		Some inline definitions
	**********/
	template<typename T>
	T mcl::ConstantTexture<T>::value(const SurfaceRecord& rec)
	{
		return val;
	}

	template<typename T>
	T mcl::ConstantTexture<T>::value(const Point2f& uv)
	{
		return val;
	}

	template<typename T>
	mcl::ConstantTexture<T>::ConstantTexture(const T& val)
		:val(val)
	{
	}

	template<typename T>
	T mcl::AddTexture<T>::value(const Point2f& uv)
	{
		return t1->value(uv) + t2->value(uv);
	}

	template<typename T>
	T mcl::AddTexture<T>::value(const SurfaceRecord& rec)
	{
		return t1->value(rec) + t2->value(rec);
	}

	template<typename T>
	T mcl::ProductTexture<T>::value(const Point2f& uv)
	{
		return t1->value(uv) & t2->value(uv);
	}

	template<typename T>
	T mcl::ProductTexture<T>::value(const SurfaceRecord& rec)
	{
		return t1->value(rec) & t2->value(rec);
	}

	inline std::shared_ptr<ConstantTexture<Color3f>> genTexture(Float r, Float g, Float b) {
		return std::make_shared<ConstantTexture<Color3f>>(Color3f(r,g,b));
	}

	inline std::shared_ptr<ConstantTexture<Float>> genTexture(Float val) {
		return std::make_shared<ConstantTexture<Float>>(val);
	}

	template<typename T>
	T mcl::DivideTextrue<T>::value(const Point2f& uv)
	{
		return tex->value(uv) / val;
	}

	template<typename T>
	T mcl::DivideTextrue<T>::value(const SurfaceRecord& rec)
	{
		return tex->value(rec) / val;
	}
}

