#pragma once
#include "mcl.h"
#include "types.h"
#include "Singleton.h"

#define  PROPTR  (Singleton<ProjectMagager>::getSingleton())
namespace mcl {
	class ProjectMagager: Singleton<ProjectMagager>
	{
	public:
		friend Singleton<ProjectMagager>;
		~ProjectMagager();

		QString swPath() const { return SoftwarePath(); }
		QString SoftwarePath() const { return softwarePath_; }
		void setSoftwarePath(QString val) { softwarePath_ = val; }
	private:
		ProjectMagager();

		QString softwarePath_;
	};
}

