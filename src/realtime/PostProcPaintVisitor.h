#pragma once
#include "mcl.h"
#include "types.h"
#include "PaintVisitor.h"
namespace mcl {
	class PostProcPaintVisitor: public PaintVisitor
	{
	public:
		PostProcPaintVisitor() {};
	};

	class GammaPaintVisitor : public PostProcPaintVisitor
	{

	};
}

