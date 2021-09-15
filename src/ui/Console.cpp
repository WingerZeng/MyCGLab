#include "Console.h"
#include <sstream>
#include <QRegularExpression>
#include "CadInterface.h"
#include "MainWindow.h"
#include "realtime/Scene.h"
#include "utilities.h"
#include "primitives/Subdivide.h"
#include "primitives/PPolygonMesh.h"
#include "ItemManager.h"
#include "GlobalInfo.h"
#include "MyCGLabConfig.h"
#define  BEGIN_FUNC_REGISTER\
	std::function<int(std::iostream& sin)> lambda;
	
//绑定静态函数，将函数调用作为lambda绑定到字典树中
#define REGISTER_CMD_FUNC(FuncName,Func) {\
	std::function<std::remove_pointer_t<decltype(Func)>> func(Func); \
	lambda = std::function<int(std::iostream& sin)>([](std::iostream& sin){\
		std::function<std::remove_pointer_t<decltype(Func)>> func(Func); \
		return callFuncByInput(func, sin); \
	});\
	triemap[#FuncName] = lambda; \
	funcArgNameList[#FuncName] = getFuncArgTypeNames(func);}  //生成函数形参字符串列表，用于生成帮助

//绑定类成员函数，将函数调用作为lambda绑定到字典树中
#define REGISTER_CLASS_CMD_FUNC(FuncName,ClassPtr,ClassName,Func) {\
	auto ptr = ClassPtr;\
	lambda = std::function<int(std::iostream& sin)>([ptr](std::iostream& sin){\
		return callFuncByInput(classFuncToStdFunc<ClassName>((ptr),(&ClassName::Func)), sin); \
	});\
	triemap[#FuncName] = lambda;\
	auto func = classFuncToStdFunc<ClassName>((ptr),(&ClassName::Func)); \
	funcArgNameList[#FuncName] = getFuncArgTypeNames(func);}   //生成函数形参字符串列表，用于生成帮助

namespace mcl{
	Console::Console(QWidget* parent)
		:QConsole(parent,welcomeText)
	{
		connect(this, &QConsole::execCommand, [this](const QString& cmd) { EXEPTR->exec(cmd, this); });
		setPrompt(">>");
		setOutColor(QColor(0.0,0.0,1.0));
		QFont font("Arial",9);
		setFont(font);
	}

	Console::~Console()
	{
	}

	const QString Console::welcomeText = "Welcome to MyCGLab! Version " +  QString::number(MyCGLab_VERSION_MAJOR) + "." + QString::number(MyCGLab_VERSION_MINOR) + "\nType \"help;\" command to get help.";

	QStringList Console::suggestCommand(const QString &cmd, QString &prefix)
	{
		return EXEPTR->suggestCommand(cmd, prefix);
	}

	bool Console::isCommandComplete(const QString &command)
	{
;		if (command.indexOf(";") == -1) return false;
		else return true;
	}

	QString Console::addCommandToHistory(const QString &command)
	{
		QString modifiedCommand = command;
		modifiedCommand = modifiedCommand.left(modifiedCommand.indexOf(';') + 1);
		modifiedCommand.replace("\n", " ");
		while (modifiedCommand[0] == " ") modifiedCommand = modifiedCommand.right(modifiedCommand.size() - 1);
		//Add the command to the recordedScript list
		recordedScript.append(modifiedCommand);
		//update the history and its index
		history.append(modifiedCommand);
		historyIndex = history.size();
		//emit the commandExecuted signal
		Q_EMIT commandAddedToHistory(modifiedCommand);
		return "";
	}

	void Executor::exec(const QString& command, Console* con)
	{
		std::vector<std::string> cmds;
		int idx;
		QString cmd = modifyCmd(command);
		//对于多行输入(脚本)，先分解为多个命令行
		while ((idx = cmd.indexOf(";")) != -1) {
			cmds.push_back(cmd.left(idx).toStdString());
			cmd = cmd.right(cmd.size() - idx - 1);
		}

		con_ = con;
		int ret = 0;
		for (const auto& cmd : cmds) {
			std::stringstream sin(cmd);
			std::string funcname;
			sin >> funcname;
			if (con) con->printCommandExecutionResults(QString::fromStdString(">Do " + funcname), QConsole::Partial);
			if (triemap.find(funcname) != triemap.end()) {
				//通过函数名调用字典树中预先绑定的函数
				ret = triemap[funcname](sin);
				if (ret) {
					break;
				}
			}
			else {
				ret = -2;
				break;
			}
			con->addCommandToHistory(QString::fromStdString(cmd) + ";");
		}

		if (con_) {
			if (ret) {
				if (ret == -2) {
					con->printCommandExecutionResults("Valid command!", QConsole::Error);
				}
				else 
					con->printCommandExecutionResults("Failed!", QConsole::Error);
			}
			else
				con->printCommandExecutionResults("Done.");
		}
		
		MAIPTR->getScene()->update();
	}

	int Executor::getHelp()
	{
		const QString msg(\
			"Please use commands to operate the software. Every command ends with a \';\'.		\n"
			"******All Commands******		\n"
			"/* Cad operators */  \n"
			"mvfs	[Coord]			\n"
			"mev	[VertexID]	[LoopID]	[Coord]						\n"
			"mef	[VertexID]	[VertexID]	[LoopID]					\n"
			"kfmrh	[FaceID]	[FaceID]								\n"
			"kemr	[LoopID]	[HalfEdgeID]							\n"
			"sweep	[SolidID]	[FaceID]	[Vector]	[Float]			\n"
			"createface	[CoordLoops]							\n"
			"toPolygonMesh [SolidID]						\n"
			"triangulatePolygonMesh [PolygonID]				\n"
			"\n"
			"/* Subdividing operators */					\n"
			"DooSabinPolygons		[PolygonID] [nLevels]	\n"
			"CatmullClarkPolygons	[PolygonID] [nLevels]	\n"
			"DooSabinTriangles		[TriangleID] [nLevels]	\n"
			"CatmullClarkTriangles	[TriangleID] [nLevels]	\n"
			"LoopTriangles			[TriangleID] [nLevels]	\n"
			"\n"
			"/* Scene operators */ \n"
			"clone		[PrimitiveID]\n"
			"scale		[PrimitiveID]	[Vector]\n"
			"rotate		[PrimitiveID]	[Vector]	[Degree]\n"
			"translate	[PrimitiveID]	[Vector]\n"
			"wireframe	[Bool]\n"
			"cameraZoom	[Vector]\n"
			"cameraRotate	[Vector]	[Degree]\n"
			"setLookAt	[POSITION_POINT]	[UP_VECTOR]	[CENTER_POINT]\n"
			"printLookAt\n"
			"setFov [Degree]\n"
			"resize [X_INT] [Y_INT]\n"
			"\n"
			"/* Software rendering operators */ \n"
			"softrender\n"
			"\n"
			"/* Offline rendering operators */ \n"
			"loadObj	[PATH]\n"
			"set samplePerPixelDim [NUM_INT]\n"
			"set threadNum [NUM_INT]\n"
			"raytrace\n"
			"setSkybox	[PATH]\n"
			"setMaterial	[ID_INT]\n"
			"setMaterial	[ID_INT]	[MATERIALTYPE_STRING]	[PARAMETERS]\n"
			"saveBmp [PATH]\n"
			"\n"
			"******Ui help******		\n"
			"Press left mouse button to rotate. Rotate mid wheel to scale.						\n"
			"Press WASD to move around.`"
			"Use item list to determine the id of elements.	\n"
			"\n"
			"******More******	\n"
			"For more more examples and details, please enter script list in the menu.   		\n"
		);
		EXEPTR->printMessage(msg, QConsole::Partial);
		return 0;
	}

	int Executor::printMessage(const QString& msg, Console::ResultType type)
	{
		std::cout << msg.toStdString() << std::endl;
		if (con_) {
			con_->printCommandExecutionResults(msg, type);
			con_->update();
		}
		return 0;
	}

	QStringList Executor::suggestCommand(const QString &cmd, QString &prefix)
	{
		auto itpair = triemap.equal_prefix_range(cmd.toStdString());
		QStringList ret;
		for (auto it = itpair.first; it != itpair.second; it++) {
			ret << QString::fromStdString(it.key());
		}
		return ret;
	}

	Executor::Executor()
	{
		//注册命令行函数
		BEGIN_FUNC_REGISTER
			REGISTER_CMD_FUNC(help, Executor::getHelp)
			REGISTER_CLASS_CMD_FUNC(mvfs, MAIPTR->CadApi(), CadInterface, mvfs)
			REGISTER_CLASS_CMD_FUNC(mev, MAIPTR->CadApi(), CadInterface, mev)
			REGISTER_CLASS_CMD_FUNC(mef, MAIPTR->CadApi(), CadInterface, mef)
			REGISTER_CLASS_CMD_FUNC(kfmrh, MAIPTR->CadApi(), CadInterface, kfmrh)
			REGISTER_CLASS_CMD_FUNC(kemr, MAIPTR->CadApi(), CadInterface, kemr)
			REGISTER_CLASS_CMD_FUNC(sweep, MAIPTR->CadApi(), CadInterface, sweep)
			REGISTER_CLASS_CMD_FUNC(createface, MAIPTR->CadApi(), CadInterface, createFaceFromLoop)
			REGISTER_CLASS_CMD_FUNC(toPolygonMesh, MAIPTR->CadApi(), CadInterface, solidToPolygonMesh)
			REGISTER_CLASS_CMD_FUNC(softrender, MAIPTR, MainWindow, softRenderer)
			REGISTER_CLASS_CMD_FUNC(set, MAIPTR, MainWindow, setData)
			REGISTER_CLASS_CMD_FUNC(raytrace, MAIPTR, MainWindow, runRayTrace)
			REGISTER_CLASS_CMD_FUNC(saveBmp, MAIPTR, MainWindow, saveRtFilm)
			REGISTER_CMD_FUNC(DooSabinPolygons, dooSabinSubdivPolygonMesh)
			REGISTER_CMD_FUNC(CatmullClarkPolygons, catmullClarkSubdivPolygonMesh)
			REGISTER_CMD_FUNC(DooSabinTriangles, dooSabinSubdivTriangles)
			REGISTER_CMD_FUNC(CatmullClarkTriangles, catmullClarkSubdivTriangles)
			REGISTER_CMD_FUNC(LoopTriangles, loopSubdivideTriangles)
			REGISTER_CMD_FUNC(triangulatePolygonMesh, triangulatePolygonMesh)
			REGISTER_CLASS_CMD_FUNC(clone, MAIPTR->getItemMng().get(), ItemManager, clonePrimitive)
			REGISTER_CLASS_CMD_FUNC(scale, MAIPTR->getItemMng().get(), ItemManager, scale)
			REGISTER_CLASS_CMD_FUNC(rotate, MAIPTR->getItemMng().get(), ItemManager, rotate)
			REGISTER_CLASS_CMD_FUNC(loadObj, MAIPTR->getItemMng().get(), ItemManager, loadObj)
			REGISTER_CLASS_CMD_FUNC(translate, MAIPTR->getItemMng().get(), ItemManager, translate)
			REGISTER_CLASS_CMD_FUNC(setMaterial, MAIPTR->getItemMng().get(), ItemManager, setMaterial)
			REGISTER_CLASS_CMD_FUNC(wireframe, MAIPTR->getScene(), Scene, wireFrameMode)
			REGISTER_CLASS_CMD_FUNC(setSkybox, MAIPTR->getScene(), Scene, setSkyBox)
			REGISTER_CLASS_CMD_FUNC(setLookAt, MAIPTR->getScene(), Scene, setCameraLookAt)
			REGISTER_CLASS_CMD_FUNC(setFov, MAIPTR->getScene(), Scene, setCameraFov)
			REGISTER_CLASS_CMD_FUNC(resize, MAIPTR, MainWindow, resize)
			REGISTER_CLASS_CMD_FUNC(printLookAt, MAIPTR->getScene(), Scene, printCameraLookAt)
			REGISTER_CLASS_CMD_FUNC(cameraZoom, &(MAIPTR->getScene()->getCamera()), Camera, scaleZoom)
			REGISTER_CLASS_CMD_FUNC(cameraRotate, &(MAIPTR->getScene()->getCamera()), Camera, rotate)
	}

	QString Executor::modifyCmd(const QString& command)
	{
		QString modifiedCommand = command;
		modifiedCommand = modifiedCommand.left(modifiedCommand.lastIndexOf(';') + 1);

		QRegularExpression re("//.*\n");
		modifiedCommand.replace(re, "");

		QStringList list = GlobalInfo::getSingleton()->getKeyList();
		for (const auto& key : list) {
			QString cmdkey = "$(" + key + ")";
			modifiedCommand.replace(cmdkey, GlobalInfo::getSingleton()->getDataString(key));
		}

		return modifiedCommand;
	}
}