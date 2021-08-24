#include "RTCamera.h"
#include "Sampler.h"
#include "Film.h"
namespace mcl{
	
	RTOrthoCamera::RTOrthoCamera(Float left, Float right, Float bottom, Float top, Transform worldToView)
		:RTCamera(worldToView),left(left),right(right),bottom(bottom),top(top)
	{

	}

	mcl::Ray RTOrthoCamera::getRay(Sampler& sampler, const Film& film, const Point2i& pixel)
	{
		Point2f xySample(sampler.get2DSample());
		Point2i xySize = film.getSize();
		Point3f xyz(xySample.x() + pixel.x(), xySample.y() + pixel.y(),0.0f);
		Point3f xyzDevice = film.screenToDevice() * xyz;
		xyzDevice = (xyzDevice + Point3f(1, 1, 0))/2;
		Point3f pRay(left + (right - left)*xyzDevice.x(), bottom + (top - bottom)*xyzDevice.y(),0);
		return viewToWorld()(Ray(pRay, Vector3f{ 0,0,-1 }, 0, MaxFloat, timer->getTime(sampler), 0));
	}

	mcl::SamplerRequestInfo RTOrthoCamera::getSamplerRequest() const
	{
		return SamplerRequestInfo(0, 1) + timer->samplerRequest();
	}

	RTCamera::RTCamera(Transform worldToView /*= Transform()*/)
		:toview(worldToView), toworld(worldToView.inverse()), timer(new ConstantTimer(0))
	{

	}

	Transform RTCamera::viewToWorld()
	{
		return toworld;
	}

	Transform RTCamera::worldToView()
	{
		return toview;
	}

	RTPerspectiveCamera::RTPerspectiveCamera(const Float& fov, const Float& dxdy, const Transform& worldToView /*= Transform()*/)
		:RTCamera(worldToView), halfy(std::tan(fov/2)), dxdy(dxdy)
	{

	}

	RTPerspectiveCamera::RTPerspectiveCamera(const Float& fov, const Transform& worldToView /*= Transform()*/)
		:RTCamera(worldToView), halfy(std::tan(fov / 2)), dxdy(-1)
	{

	}

	mcl::Ray RTPerspectiveCamera::getRay(Sampler& sampler, const Film& film, const Point2i& pixel)
	{
		const Point2i size = film.getSize();
		
		Float halfx = halfy * (dxdy <= 0 ? size.x()*1.0 / size.y() : dxdy);

		Point2f xySample(sampler.get2DSample());
		Point3f xyz(xySample.x() + pixel.x(), xySample.y() + pixel.y(), 0.0f);
		Point3f xyzDevice = film.screenToDevice() * xyz;
		xyzDevice = xyzDevice & Point3f(halfx, halfy, 0);
		xyzDevice.z() = -1;

		return viewToWorld()(Ray(Point3f(0),Normalize(Vector3f(xyzDevice)),0,MaxFloat,timer->getTime(sampler),0));
	}

	mcl::SamplerRequestInfo RTPerspectiveCamera::getSamplerRequest() const
	{
		return SamplerRequestInfo(0, 1) + timer->samplerRequest();
	}

}