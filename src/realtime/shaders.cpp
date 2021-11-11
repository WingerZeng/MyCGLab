#include "shaders.h"
#include "GlobalInfo.h"
namespace mcl{
	
	int initAllShaders()
	{
		CommonShader::clear();
		LightShader::clear();
		LineShader::clear();
		PointShader::clear();
		LightPerFragShader::clear();
		ToneMapShader::clear();
		CubeShaodwMapShader::clear();
		DirectLightShader::clear();
		DirectLightFilterShader::clear();
		SsrShader::clear();
		SsrFilterShader::clear();
		SsdoShader::clear();
		SsdoFilterShader::clear();
		BloomFilterShader::clear();
		FxaaShader::clear();
		if (CommonShader::ptr() &&
			LightShader::ptr() &&
			LineShader::ptr() &&
			PointShader::ptr() &&
			LightPerFragShader::ptr() &&
			ToneMapShader::ptr() &&
			CubeShaodwMapShader::ptr() &&
			DirectLightShader::ptr() &&
			DirectLightFilterShader::ptr() &&
			SsrShader::ptr() &&
			SsrFilterShader::ptr() &&
			SsdoShader::ptr() &&
			SsdoFilterShader::ptr() &&
			BloomFilterShader::ptr() &&
			FxaaShader::ptr()) {
			Singleton<GlobalInfo>::getSingleton()->shaderReady = true;
			return 0;
		}
		else {
			Singleton<GlobalInfo>::getSingleton()->shaderReady = false;
			return -1;
		}
	}

}