#pragma once
#include <memory>
#include <vector>
#include <QMatrix4x4>

namespace mcl {
	class Light;

	enum FillMode
	{
		FILL = 0,
		WIREFRAME,
		FILL_WIREFRAME
	};

	class PaintInfomation
	{
	public:
		QMatrix4x4 projMat, viewMat;
		std::vector<std::shared_ptr<Light>> lights;
		FillMode fillmode;
		double lineWidth;
		double pointSize;
		double width,height;
	};
}