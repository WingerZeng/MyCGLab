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
		DeferredDirectLightShader::clear();
		DeferredSsdoShader::clear();
		BloomFilterShader::clear();
		DeferredCompositeShader::clear();
		if (CommonShader::ptr() &&
			LightShader::ptr() &&
			LineShader::ptr() &&
			PointShader::ptr() &&
			LightPerFragShader::ptr() &&
			ToneMapShader::ptr() &&
			CubeShaodwMapShader::ptr() &&
			DeferredDirectLightShader::ptr() &&
			DeferredSsdoShader::ptr() &&
			BloomFilterShader::ptr() &&
			DeferredCompositeShader::ptr()) {
			Singleton<GlobalInfo>::getSingleton()->shaderReady = true;
			return 0;
		}
		else {
			Singleton<GlobalInfo>::getSingleton()->shaderReady = false;
			return -1;
		}
	}

}