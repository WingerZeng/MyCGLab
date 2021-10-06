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
		GammaCorrectShader::clear();
		CubeShaodwMapShader::clear();
		DeferedShader::clear();
		if (CommonShader::ptr() &&
			LightShader::ptr() &&
			LineShader::ptr() &&
			PointShader::ptr() &&
			LightPerFragShader::ptr() &&
			GammaCorrectShader::ptr() &&
			CubeShaodwMapShader::ptr() &&
			DeferedShader::ptr()) {
			Singleton<GlobalInfo>::getSingleton()->shaderReady = true;
			return 0;
		}
		else {
			Singleton<GlobalInfo>::getSingleton()->shaderReady = false;
			return -1;
		}
	}

}