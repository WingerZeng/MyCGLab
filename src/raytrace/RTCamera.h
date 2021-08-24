#pragma once
#include "mcl.h"
#include "types.h"
#include "Sampler.h"
namespace mcl {
	class RTCamera
	{
	public:
		RTCamera(Transform worldToView = Transform());
		virtual Ray getRay(Sampler& sampler, const Film& film, const Point2i& pixel) = 0;
		virtual SamplerRequestInfo getSamplerRequest() const = 0 ;

		Transform viewToWorld();
		Transform worldToView();

		virtual ~RTCamera() {};
	protected:
		std::unique_ptr<TimeController> timer;
		Transform toworld, toview;
	};

	class RTOrthoCamera:public RTCamera
	{
	public:
		RTOrthoCamera(Float left, Float right, Float bottom, Float top, Transform worldToView = Transform());
		Ray getRay(Sampler& sampler, const Film& film, const Point2i& pixel) override;
		SamplerRequestInfo getSamplerRequest() const override;
	private:
		Float left, right, bottom, top;
	};

	class RTPerspectiveCamera: public RTCamera
	{
	public:
		/**
		 * @brief fov为视场角(采用弧度制)，dxdy为长宽比，若不指定该项，则长宽比等于film的长宽方向像素之比
		 */
		RTPerspectiveCamera(const Float& fov, const Float& dxdy, const Transform& worldToView = Transform());
		RTPerspectiveCamera(const Float& fov, const Transform& worldToView = Transform());


		virtual Ray getRay(Sampler& sampler, const Film& film, const Point2i& pixel) override;


		virtual SamplerRequestInfo getSamplerRequest() const override;

	private:
		const Float halfy;
		const Float dxdy;
	};

	class TimeController {
	public:
		virtual Float getTime(Sampler& sampler) = 0;
		virtual SamplerRequestInfo samplerRequest() const{
			return SamplerRequestInfo();
		}
	};

	class ConstantTimer : public TimeController {
	public:
		ConstantTimer(Float time)
			:time(time) {}

		Float getTime(Sampler& sampler) override { return time; } ;
	private:
		Float time;
	};
}

