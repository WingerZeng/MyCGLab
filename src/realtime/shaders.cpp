#include "shaders.h"

namespace mcl{
	
	int clearAllShaders()
	{
		CommonShader::clear();
		LightShader::clear();
		LineShader::clear();
		PointShader::clear();
		LightPerFragShader::clear();
		GammaCorrectShader::clear();
		CubeShaodwMapShader::clear();
		return 0;
	}

}