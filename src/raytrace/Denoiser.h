#pragma once
#ifdef USE_OPTIX
#include "mcl.h"
#include "types.h"
#include "osc/CUDABuffer.h"
#include "osc/optix7.h"
class Film;
namespace mcl {
	class Denoiser
	{
	public:
		enum DenoiserOption {
			COLOR_ONLY = 0,
			USE_NORMAL = 1 << 1,
			USE_ALBEDO = 1 << 2,
			USE_HDR = 1 << 3
		};
		Denoiser(std::shared_ptr<Film> film, DenoiserOption opt = DenoiserOption(COLOR_ONLY | USE_HDR));
		~Denoiser();
		void denoise();
	private:
		void initOptix();
		std::shared_ptr<Film> film;
		DenoiserOption opt;

		CUcontext          cudaContext;
		CUstream           stream;
		OptixDeviceContext optixContext;
		OptixDenoiser denoiser;
		cudaDeviceProp     deviceProps;
		osc::CUDABuffer    denoiserScratch;
		osc::CUDABuffer    denoiserState;
		osc::CUDABuffer    denoiserIntensity;

		osc::CUDABuffer fbColor;
		osc::CUDABuffer fbNormal;
		osc::CUDABuffer fbAlbedo;
		osc::CUDABuffer denoisedBuffer;
	};
}
#endif // USE_OPTIX


