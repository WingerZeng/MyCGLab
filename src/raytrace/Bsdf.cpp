#include "Bsdf.h"
#include "algorithms.h"
namespace mcl{
	
	BsdfGroup::BsdfGroup(const Vector3f& nvec, const Vector3f& uvec, const Vector3f& geo_nvec)
		:Bsdf(NULL_Surface,NULL_Scatter),geonvec(geo_nvec)
	{	
		Vector3f vvec = nvec.cross(uvec);
		Vector3f nuvec = vvec.cross(nvec);
		toSurface = Transform::tranToCoordinateSystem(nuvec, vvec, nvec);
		toWorld = toSurface.inverse();
	}

	void BsdfGroup::addBsdf(std::unique_ptr<Bsdf> bsdf)
	{
		sft = SurfaceType(sft | bsdf->surfaceType());
		sct = ScatterType(sct | bsdf->scatterType());
		bsdfs.emplace_back(std::move(bsdf));
	}

	Vector3f BsdfGroup::worldToSurface(const Vector3f& wvec) const
	{
		return Normalize(toSurface(wvec));
	}

	Vector3f BsdfGroup::surfaceToWorld(const Vector3f& svec) const
	{
		return Normalize(toWorld(svec));
	}

	mcl::Color3f BsdfGroup::calF(const Vector3f& out, const Vector3f& in, SurfaceType sft /*= ALL_Surface*/, ScatterType sct /*= ALL_Scatter*/) const
	{
		mcl::Color3f ret;
		Vector3f sout = worldToSurface(out);
		Vector3f sin = worldToSurface(in);
		if (sout.z() == 0 || sin.z() == 0) {
			return Color3f(0);
		}
		sct = ScatterType(sct & ((out.dot(geonvec)*in.dot(geonvec) > 0) ? Reflect : Refract));
		for (const auto& bsdf : bsdfs) {
			if (bsdf->hasType(sft, sct)) {
				ret += bsdf->calF(sout, sin, sft, sct);
			}
		}
		return ret;
	}

	mcl::Color3f BsdfGroup::calIn(const Vector3f& out, Vector3f& in, Float& possibility, Sampler& sampler, SurfaceType sft /*= ALL_Surface*/, ScatterType sct /*= ALL_Scatter*/) const
	{
		int activeBsdfCount = 0;
		for (const auto& bsdf : bsdfs) {
			if (bsdf->hasType(sft, sct))
				++activeBsdfCount;
		}
		if (!activeBsdfCount) {
			possibility = 0;
			return Color3f(0, 0, 0);
		}

		int selectedIndex = std::clamp(int(activeBsdfCount * sampler.get1DSample()), 0, activeBsdfCount - 1);
		int startIndex = selectedIndex;

		Vector3f sout = worldToSurface(out);
		//任何bsdf都忽略平行于表面的光线
		if (sout.z() == 0.0) {
			possibility = 0;
			return Color3f(0, 0, 0);
		}
		Vector3f sin;
		Color3f ret;
		Bsdf* selectedBsdf;
		ScatterType actualSct = sct;

		const int retryTime = 4;

		do   
		{
			int index = 0;
			for (const auto& bsdf : bsdfs) {
				if (bsdf->hasType(sft, sct)) {
					if (index == selectedIndex) {
						selectedBsdf = bsdf.get();
						break;
					}
					++index;
				}
			}

			for (int i = 0; i < retryTime; i++) {
				ret = selectedBsdf->calIn(sout, sin, possibility, sampler, sft, sct);
				if(ret == Color3f(0,0,0)) continue;
				in = surfaceToWorld(sin);
				actualSct = ScatterType(sct & ((out.dot(geonvec)*in.dot(geonvec) > 0) ? Reflect : Refract));
				if (selectedBsdf->hasType(sft, actualSct)) break;
				ret = Color3f(0);
			}

			selectedIndex = (selectedIndex + 1) % activeBsdfCount;
		} while (!possibility && selectedIndex != startIndex); //循环直到possibility不为0

		if (possibility == 0) {
			return Color3f(0);
		}

		for (const auto& bsdf : bsdfs) {
			if (bsdf.get() != selectedBsdf) {
				possibility += bsdf->calPossibility(sout, sin, sft, actualSct);
				ret += bsdf->calF(sout, sin, sft, actualSct);
			}
		}

		possibility /= activeBsdfCount;
		return ret;
	}

	Float BsdfGroup::calPossibility(const Vector3f& out, const Vector3f& in, SurfaceType sft /*= ALL_Surface*/, ScatterType sct /*= ALL_Scatter*/) const
	{
		Float ret = 0;
		for (const auto& bsdf : bsdfs) {
			ret += bsdf->calPossibility(out, in);
		}
		return ret;
	}

	mcl::SamplerRequestInfo BsdfGroup::getSamplerRequest() const
	{
		SamplerRequestInfo requset;
		for (const auto& bsdf : bsdfs) {
			requset = SamplerRequestInfo::max(requset, bsdf->getSamplerRequest());
		}
		++requset.n1d; //在同时有多个bsdf符合条件时，BsdfGroup需要额外一个一维随机数
		return requset;
	}

	mcl::Color3f BsdfGroup::albedo() const
	{
		Color3f a;
		for (int i = 0; i < bsdfs.size(); i++) {
			a += bsdfs[i]->albedo();
		}
		return a;
	}

}