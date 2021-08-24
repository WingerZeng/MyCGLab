#include "Denoiser.h"
#include "Film.h"
#include <optix_function_table_definition.h>
namespace mcl{

	Denoiser::Denoiser(std::shared_ptr<Film> film, DenoiserOption opt /*= DenoiserOption(COLOR_ONLY | USE_HDR)*/)
		:film(film), opt(opt)
	{
		initOptix();
		// for this sample, do everything on one device
		const int deviceID = 0;
		CUDA_CHECK(SetDevice(deviceID));
		CUDA_CHECK(StreamCreate(&stream));
		cudaGetDeviceProperties(&deviceProps, deviceID);
		std::cout << "#osc: running on device: " << deviceProps.name << std::endl;

		CUresult  cuRes = cuCtxGetCurrent(&cudaContext);
		if (cuRes != CUDA_SUCCESS)
			fprintf(stderr, "Error querying current context: error code %d\n", cuRes);

		OPTIX_CHECK(optixDeviceContextCreate(cudaContext, 0, &optixContext));

		OptixDenoiserOptions denoiserOptions = { opt & USE_ALBEDO, opt & USE_NORMAL };
		auto hdr_opt = (opt & USE_HDR) ? OPTIX_DENOISER_MODEL_KIND_HDR : OPTIX_DENOISER_MODEL_KIND_LDR;
		OPTIX_CHECK(optixDenoiserCreate(optixContext, hdr_opt, &denoiserOptions, &denoiser));
		OptixDenoiserSizes denoiserReturnSizes;
		OPTIX_CHECK(optixDenoiserComputeMemoryResources(denoiser, film->getSize().x(), film->getSize().y(), &denoiserReturnSizes));

		denoiserScratch.resize(std::max(denoiserReturnSizes.withOverlapScratchSizeInBytes,
			denoiserReturnSizes.withoutOverlapScratchSizeInBytes));
		denoiserState.resize(denoiserReturnSizes.stateSizeInBytes);

		denoisedBuffer.resize(film->getSize().x() * film->getSize().y() * sizeof(float4));
		fbColor.resize(film->getSize().x() * film->getSize().y() * sizeof(float4));
		fbNormal.resize(film->getSize().x() * film->getSize().y() * sizeof(float3));
		fbAlbedo.resize(film->getSize().x() * film->getSize().y() * sizeof(float4));
		
		OPTIX_CHECK(optixDenoiserSetup(denoiser, 0,
			film->getSize().x(), film->getSize().y(),
			denoiserState.d_pointer(),
			denoiserState.size(),
			denoiserScratch.d_pointer(),
			denoiserScratch.size()));
	}

	Denoiser::~Denoiser()
	{
		OPTIX_CHECK(optixDenoiserDestroy(denoiser));		
		
		denoiserScratch.free();
		denoiserState.free();
		denoiserIntensity.free();

		fbColor.free();
		fbNormal.free();
		fbAlbedo.free();
		denoisedBuffer.free();
	}

	void Denoiser::denoise()
	{
		std::unique_ptr<Float[]> colors, albedos, normals;
		int pcnt = film->getSize().x() *  film->getSize().y();
		int nx = film->getSize().x();
		int ny = film->getSize().y();
		colors.reset(new Float[pcnt * 4]);
		albedos.reset(new Float[pcnt * 4]);
		normals.reset(new Float[pcnt * 3]);

		for (int y = 0; y < ny; y++) {
			for (int x = 0; x < nx; x++) {
				for (int i = 0; i < 3; i++) {
					colors[y * nx * 4 + x * 4 + i] = film->getPixelData(x, y).color[i];
					albedos[y * nx * 4 + x * 4 + i] = film->getPixelData(x, y).albedo[i];
					normals[y * nx * 3 + x * 3 + i] = film->getPixelData(x, y).normal[i];
				}
				colors[y * nx * 4 + x * 4 + 3] = 0;
				albedos[y * nx * 4 + x * 4 + 3] = 0;
			}
		}
		//是否要先分配内存？
		fbColor.upload(colors.get(), pcnt * 4);
		fbAlbedo.upload(albedos.get(), pcnt * 4);
		fbNormal.upload(normals.get(), pcnt * 3);

		denoiserIntensity.resize(sizeof(float));
		OptixDenoiserParams denoiserParams;
		denoiserParams.denoiseAlpha = 1;
		if (denoiserIntensity.sizeInBytes != sizeof(float))
			denoiserIntensity.alloc(sizeof(float));
		if (opt & USE_HDR)
			denoiserParams.hdrIntensity = denoiserIntensity.d_pointer();
		else
			denoiserParams.hdrIntensity = 0;
		denoiserParams.blendFactor = 0;

		OptixImage2D inputLayer[3];
		inputLayer[0].data = fbColor.d_pointer();
		/// Width of the image (in pixels)
		inputLayer[0].width = nx;
		/// Height of the image (in pixels)
		inputLayer[0].height = ny;
		/// Stride between subsequent rows of the image (in bytes).
		inputLayer[0].rowStrideInBytes = nx * sizeof(float4);
		/// Stride between subsequent pixels of the image (in bytes).
		/// For now, only 0 or the value that corresponds to a dense packing of pixels (no gaps) is supported.
		inputLayer[0].pixelStrideInBytes = sizeof(float4);
		/// Pixel format.
		inputLayer[0].format = OPTIX_PIXEL_FORMAT_FLOAT4;

		// ..................................................................
		inputLayer[2].data = fbNormal.d_pointer();
		/// Width of the image (in pixels)
		inputLayer[2].width = nx;
		/// Height of the image (in pixels)
		inputLayer[2].height = ny;
		/// Stride between subsequent rows of the image (in bytes).
		inputLayer[2].rowStrideInBytes = nx * sizeof(float3);
		/// Stride between subsequent pixels of the image (in bytes).
		/// For now, only 0 or the value that corresponds to a dense packing of pixels (no gaps) is supported.
		inputLayer[2].pixelStrideInBytes = sizeof(float3);
		/// Pixel format.
		inputLayer[2].format = OPTIX_PIXEL_FORMAT_FLOAT3;

		// ..................................................................
		inputLayer[1].data = fbAlbedo.d_pointer();
		/// Width of the image (in pixels)
		inputLayer[1].width = nx;
		/// Height of the image (in pixels)
		inputLayer[1].height = ny;
		/// Stride between subsequent rows of the image (in bytes).
		inputLayer[1].rowStrideInBytes = nx * sizeof(float4);
		/// Stride between subsequent pixels of the image (in bytes).
		/// For now, only 0 or the value that corresponds to a dense packing of pixels (no gaps) is supported.
		inputLayer[1].pixelStrideInBytes = sizeof(float4);
		/// Pixel format.
		inputLayer[1].format = OPTIX_PIXEL_FORMAT_FLOAT4;

		OptixImage2D outputLayer;
		outputLayer.data = denoisedBuffer.d_pointer();
		/// Width of the image (in pixels)
		outputLayer.width = nx;
		/// Height of the image (in pixels)
		outputLayer.height = ny;
		/// Stride between subsequent rows of the image (in bytes).
		outputLayer.rowStrideInBytes = nx * sizeof(float4);
		/// Stride between subsequent pixels of the image (in bytes).
		/// For now, only 0 or the value that corresponds to a dense packing of pixels (no gaps) is supported.
		outputLayer.pixelStrideInBytes = sizeof(float4);
		/// Pixel format.
		outputLayer.format = OPTIX_PIXEL_FORMAT_FLOAT4;
		
		OPTIX_CHECK(optixDenoiserComputeIntensity
		(denoiser,
			/*stream*/0,
			&inputLayer[0],
			(CUdeviceptr)denoiserIntensity.d_pointer(),
			(CUdeviceptr)denoiserScratch.d_pointer(),
			denoiserScratch.size()));

		OptixDenoiserGuideLayer denoiserGuideLayer = {};
		if (opt & USE_ALBEDO) {
			denoiserGuideLayer.albedo = inputLayer[1];
		}
		if (opt & USE_NORMAL) {
			denoiserGuideLayer.normal = inputLayer[2];
		}
		OptixDenoiserLayer denoiserLayer = {};
		denoiserLayer.input = inputLayer[0];
		denoiserLayer.output = outputLayer;

		OPTIX_CHECK(optixDenoiserInvoke(denoiser,
			0,
			&denoiserParams,
			denoiserState.d_pointer(),
			denoiserState.size(),
			&denoiserGuideLayer,
			&denoiserLayer, 1,
			0, 0,
			denoiserScratch.d_pointer(),
			denoiserScratch.size()));
		denoisedBuffer.download(colors.get(), pcnt * 4);

		for (int y = 0; y < ny; y++) {
			for (int x = 0; x < nx; x++) {
				film->setPixelColor(x, y, Color3f(
					colors[y * nx * 4 + x * 4 + 0],
					colors[y * nx * 4 + x * 4 + 1],
					colors[y * nx * 4 + x * 4 + 2])
				);
			}
		}
	}

	void Denoiser::initOptix()
	{
		cudaFree(0);
		int numDevices;
		cudaGetDeviceCount(&numDevices);
		if (numDevices == 0)
			throw std::runtime_error("#osc: no CUDA capable devices found!");
		OPTIX_CHECK(optixInit());
	}

}