#include "Scene.h"
#include <QOpenGLDebugLogger>
#include "Primitive.h"
#include "primitives/PPolygon.h"
#include "Light.h"
#include "PerspectiveCamera.h"

#include "RTPrimitive.h"
#include "Film.h"
#include "Sampler.h"
#include "RayTracer.h"
#include "GLFrameBufferObject.h"
#include "PTriMesh.h"
#include "RTScene.h"
#include "ToneMapper.h"
#include "material.h"
#include "Bsdf.h"
#include "RTLight.h"
#include <QLabel>
#include "utilities.h"
#include "ui/Console.h"
#include "ui/MainWindow.h"
#include "DefaultPaintVisitor.h"
#include "CubeShadowMapPaintVisitor.h"
#include "PostProcPaintVisitor.h"
#include "GLFunctions.h"
#include "GlobalInfo.h"
#include "shaders.h"
#include "GLTexture.h"
#include "Painter.h"

namespace mcl {
Scene::Scene(QWidget* parent)
	: QOpenGLWidget(parent)
{
	QSurfaceFormat format;
	//输出调试信息
	format.setMajorVersion(4);
	format.setMinorVersion(3);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setOption(QSurfaceFormat::DebugContext);
	this->setFormat(format);
	wfmode_ = true;

	setFocusPolicy(Qt::StrongFocus);

	camera.reset(new PerspectiveCamera);
	//lights_.push_back(std::shared_ptr<Light>(new ConstantLight(Vector3f(0.5, 0.5, 0.5), Vector3f(0.9, 0.9, 0.9), Vector3f(0.0f), Point3f(4000, 4000, 5000))));
}

QOpenGLDebugLogger* Scene::logger = nullptr;

Scene::~Scene()
{
	
}

void Scene::initializeGL()
{
	GLFUNC->initializeOpenGLFunctions();
	logger = new QOpenGLDebugLogger(this);
	logger->initialize();

	GLFUNC->glClearColor(0.7f, 0.7f, 0.8f, 1.0f);
	GLFUNC->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//相机初始化
	camera->initialize(width(), height());

	//坐标轴初始化
	axis.initialize();
	axis.updateViewMat(camera->getViewMatrixForAxis());
	axis.updateProjMat(camera->getProjMatrixForAxis());

	pingpongFbo[0] = std::make_shared<GLColorFrameBufferObject>();
	pingpongFbo[1] = std::make_shared<GLColorFrameBufferObject>();
	pingpongFbo[2] = std::make_shared<GLColorFrameBufferObject>();

	lightSMPainter = std::make_shared<LightsShadowMapPainter>();
	lightSMPainter->init(&info);
	mtrPainter = std::make_shared<DeferredMtrPainter>();
	mtrPainter->init(&info);
	directLightPainter = std::shared_ptr<DirectLightPainter>(new DirectLightPainter(pingpongFbo[0]));
	directLightPainter->init(&info);
	directLightFilterPainter = std::make_shared<DirectLightFilterPainter>(pingpongFbo[2]);
	directLightFilterPainter->init(&info);
	ssdoPainter = std::make_shared<SsdoPainter>(pingpongFbo[0]);
	ssdoPainter->init(&info);
	ssdoFilterPainter = std::make_shared<SsdoFilterPainter>(pingpongFbo[1]);
	ssdoFilterPainter->init(&info);
	hizFilterPainter = std::make_shared<HiZFilterPainter>(10);
	hizFilterPainter->init(&info);
	ssrPainter = std::make_shared<HiZSsrPainter>(pingpongFbo[0]);
	ssrPainter->init(&info);
	ssrFilterPainter = std::make_shared<SsrFilterPainter>(pingpongFbo[2]);
	ssrFilterPainter->init(&info);
	bloomFilterPainter = std::make_shared<BloomFilterPainter>(MaxBloomMipLevel, BloomMipStopSize);
	bloomFilterPainter->init(&info);
	toneMapPainter = std::make_shared<ToneMapPainter>(pingpongFbo[0]);
	toneMapPainter->init(&info);
	fxaaPainter = std::make_shared<FxaaPainter>(std::make_shared<GLQWidgetFrameBufferObject>(this));
	fxaaPainter->init(&info);

	doPrimAdd();

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(repaint()));
	timer->start(10);

	//init paint info
	info.lineWidth = 1.5f;
	info.pointSize = 8.0f;
	info.prims = &prims_;

	initAllShaders();
}

void Scene::resizeGL(int w, int h)
{
	GLFUNC->glViewport(0, 0, w, h);
	camera->initialize(w, h);

	lightSMPainter->resize(w, h);
	mtrPainter->resize(w, h);
	directLightPainter->resize(w,h);
	directLightFilterPainter->resize(w, h);
	hizFilterPainter->resize(w, h);
	ssrPainter->resize(w, h);
	ssrFilterPainter->resize(w, h);
	ssdoPainter->resize(w, h);
	ssdoFilterPainter->resize(w, h);
	bloomFilterPainter->resize(w, h);
	toneMapPainter->resize(w, h);
	fxaaPainter->resize(w, h);

	info.width = w;
	info.height = h;
}

void Scene::paintGL()
{
	if (!Singleton<GlobalInfo>::getSingleton()->shaderReady)
		return;
	doPrimAdd();

	/******* Prepare Paint Info ***********/
	info.projMat = camera->getProjMatrix();
	info.viewMat = camera->getViewMatrix();
	if (wfmode_) info.fillmode = FILL_WIREFRAME;
	else info.fillmode = FILL;

	/******* Prepare Shadow Map ***********/
	if (bNeedInitLight) {
		info.lights = lights_;
		lightSMPainter->paint(&info);
		bNeedInitLight = false;
	}

	/******* Prepare Deferred Shading ***********/
	mtrPainter->paint(&info);

	/******* Deferred Shading ***********/
	// direct light
	directLightPainter->paint(&info);

	// filter direct light
	directLightFilterPainter->paint(&info);

	// SSDO
	ssdoPainter->paint(&info);

	// filter SSDO
	ssdoFilterPainter->paint(&info);

	hizFilterPainter->paint(&info);

	// SSR
	ssrPainter->paint(&info);

	// SSR Filter
	ssrFilterPainter->paint(&info);

	// filter Bloom Mipmaps
	bloomFilterPainter->paint(&info);

	//Forward Shading For Line, Point etc.
	//#TODO0
	//for (auto& prim : prims_) {
	//	prim.second->paint(&info, forwardPainter.get());
	//}

	//坐标轴
	//#TODO0
	//GLFUNC->glClear(GL_DEPTH_BUFFER_BIT);
	//axis.updateViewMat(camera->getViewMatrixForAxis());
	//axis.updateProjMat(camera->getProjMatrixForAxis());
	//axis.paint();

	//toneMap
	toneMapPainter->paint(&info);

	fxaaPainter->paint(&info);

	debugOpenGL();
}

void Scene::mouseMoveEvent(QMouseEvent *ev)
{
	camera->mouseMoveEvent(ev);
	updateScene(CAMERA);
}

//滚轮控制缩放
void Scene::wheelEvent(QWheelEvent *ev) {
	camera->wheelEvent(ev);
	updateScene(CAMERA);
}

void Scene::mousePressEvent(QMouseEvent *ev)
{
	camera->mousePressEvent(ev);
	updateScene(CAMERA);
}

void Scene::keyPressEvent(QKeyEvent* ev)
{
	camera->keyPressEvent(ev);
	updateScene(CAMERA);
}

void Scene::keyReleaseEvent(QKeyEvent* ev)
{
	camera->keyReleaseEvent(ev);
	updateScene(CAMERA);
}

void Scene::doPrimAdd()
{
	while (!primsToAdd.empty()) {
		const auto& prim = primsToAdd.back();
		if (prims_.find(prim->id()) == prims_.end()) {
			prims_[prim->id()] = prim;
			prim->initAll();
		}
		primsToAdd.pop_back();
	}
}

void Scene::updateScene(UpdateReason reason)
{
	update();
	repaint();
	if (reason == PRIMITIVE) {
		prepareLight();
		sceneBound = Bound3f();
		for (const auto& prim : prims_) {
			sceneBound.unionBd(prim.second->getBound());
		}
		for (const auto& prim : primsToAdd) {
			sceneBound.unionBd(prim->getBound());
		}
		info.sceneBnd = sceneBound;
		Float len = (sceneBound.pMax() - sceneBound.pMin()).length();
		for (auto& light : lights_) {
			if (std::dynamic_pointer_cast<PointLight>(light)) {
				std::dynamic_pointer_cast<PointLight>(light)->setShadowOffset(len / 1000);
			}
		}

	}
	emit updated(reason);
}

void Scene::prepareLight()
{
	lights_.clear();
	for (const auto& prim : prims_) {
		if (prim.second->getMaterial()->hasEmission()) {
			lights_.push_back(std::make_shared<PointLight>(prim.second->getMaterial()->emission(), prim.second, 2048, 2048));
		}
	}
	for (const auto& prim : primsToAdd) {
		if (prim->getMaterial()->hasEmission()) {
			lights_.push_back(std::make_shared<PointLight>(prim->getMaterial()->emission(), prim, 2048, 2048));
		}
	}
	bNeedInitLight = true;
	//#TODO1 适应环境贴图光源
}

std::shared_ptr<mcl::RTScene> Scene::createRTScene()
{
	std::vector<std::shared_ptr<RTPrimitive>> rtprims;
	std::vector<std::shared_ptr<RTLight>> rtlights;
	for (const auto&prim : prims_) {
		auto pprim = prim.second->createRTPrimitive();
		if (pprim) {
			rtprims.emplace_back(pprim);
			if (pprim->getLight()) {
				rtlights.emplace_back(pprim->getSharedLight());
			}
		}
	}
	if (rtinfo.skyboxpath != "") {
		auto skytex = std::make_shared<PixelMapTexture>(rtinfo.skyboxpath);
		auto skylight = std::make_shared<RTSkyBox>(skytex,Color3f(1.00, 1.00, 1.00),Transform::rotate(Vector3f(1,0,0),-90)); //使y轴朝上
		rtlights.push_back(skylight);
	}
	return std::make_shared<RTScene>(rtprims, rtlights, camera->createRTCamera());
}

void Scene::delPrimitive(int id)
{
	auto it = prims_.find(id);
	if (it != prims_.end())
		prims_.erase(it);
	for (auto it = primsToAdd.begin(); it != primsToAdd.end();) {
		// #PERF5 此处如果PrimsToAdd较多，会出现性能问题
		if ((*it)->id() == id) it = primsToAdd.erase(it); 
		else it++;
	}
	updateScene(PRIMITIVE);
}

void Scene::debugOpenGL()
{
	const QList<QOpenGLDebugMessage> messages = logger->loggedMessages();
	for (const QOpenGLDebugMessage &message : messages)
		qDebug() << message;
}

int Scene::wireFrameMode(bool wfmode)
{
	wfmode_ = wfmode;
	updateScene(PAINTING_INFO);
	return 0;
}

int Scene::setCameraLookAt(const Point3f& pos, const Vector3f& up, const Vector3f& center)
{
	auto pcamera = std::dynamic_pointer_cast<ViewCamera>(camera);
	if (!pcamera) {
		EXEPTR->printMessage("Invalid camera type!", Console::Partial);
		return -1;
	}
	pcamera->setLookAt(QVector3D(pos), QVector3D(up), QVector3D(center));
	return 0;
}

int Scene::printCameraLookAt()
{
	std::stringstream sio;
	std::string info;
	QVector3D pos, up, center;
	auto pcamera = std::dynamic_pointer_cast<ViewCamera>(camera);
	if (!pcamera) {
		EXEPTR->printMessage("Invalid camera type!", Console::Partial);
		return -1;
	}
	pcamera->getLookAt(pos, up, center);
	sio << "Position: " << Point3f(pos) << std::endl << "Up: " << Vector3f(up) << std::endl << "Front: " << Vector3f(center) << std::endl;
	EXEPTR->printMessage(QString::fromStdString(sio.str()), Console::Partial);
	return 0;
}

int Scene::setCameraFov(Float fov)
{
	auto pcamera = std::dynamic_pointer_cast<PerspectiveCamera>(camera);
	if (!pcamera) {
		EXEPTR->printMessage("Invalid camera type!", Console::Partial);
		return -1;
	}
	pcamera->setFov(fov);
	return 0;
}

const std::map<int, std::shared_ptr<mcl::Primitive>>& Scene::getPrimitives()
{
	if (!primsToAdd.empty()) {
		QOpenGLWidget::repaint();
	}
	return prims_;
}

void mcl::Scene::addPrimitive(std::shared_ptr<Primitive> prim)
{
	primsToAdd.push_back(prim);
	updateScene(PRIMITIVE);
}
}
