#include "Scene.h"
#include <QOpenGLDebugLogger>
#include "Primitive.h"
#include "primitives/PPolygon.h"
#include "PaintInformation.h"
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
	mtrPainter = std::make_shared<DeferredMtrPaintVisitor>();
	deferredDirLightPainter = std::make_shared<DeferredDirectLightPaintVisitor>();
	deferredSsdoPaintVisitor = std::make_shared<DeferredSsdoPaintVisitor>();
	forwardPainter = std::make_shared<ForwardPaintVisitor>();
	toneMapPainter = std::make_shared<ToneMapPaintVisitor>();

	//相机初始化
	camera->initialize(width(), height());

	//坐标轴初始化
	axis.initialize();
	axis.updateViewMat(camera->getViewMatrixForAxis());
	axis.updateProjMat(camera->getProjMatrixForAxis());

	billboard = PTriMesh::createBillBoard();
	billboard->initAll();
	fbo1 = std::make_shared<GLColorFrameBufferObject>();
	fbo2 = std::make_shared<GLColorFrameBufferObject>();
	msfbo = std::make_shared<GLMultiSampleFrameBufferObject>(sampleRate);
	mtrfbo = std::make_shared<GLMtrFrameBufferObject>(sampleRate);
	doPrimAdd();

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(repaint()));
	timer->start(10);

	initAllShaders();
}

void Scene::resizeGL(int w, int h)
{
	GLFUNC->glViewport(0, 0, w, h);
	camera->initialize(w, h);
	fbo1->resize(h, w);
	fbo2->resize(h, w);
	msfbo->resize(h, w);
	mtrfbo->resize(h, w);
}

void Scene::paintGL()
{
	if (!Singleton<GlobalInfo>::getSingleton()->shaderReady)
		return;
	doPrimAdd();

	//Prepare shadow map
	if (bNeedInitLight) {
		for (int i = 0; i < lights_.size(); i++) {
			std::shared_ptr<PointLight> ptlight = std::dynamic_pointer_cast<PointLight>(lights_[i]);
			if (ptlight) {
				std::shared_ptr<CubeShadowMapPaintVisitor> smpainter = std::make_shared<CubeShadowMapPaintVisitor>(ptlight);
				ptlight->initFbo();
				ptlight->getFbo()->bind();
				GLFUNC->glViewport(0, 0, ptlight->shadowMapSize().x(), ptlight->shadowMapSize().y());
				GLFUNC->glEnable(GL_DEPTH_TEST);
				GLFUNC->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
				GLFUNC->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				PaintInfomation info;
				for (auto& prim : prims_) {
					prim.second->paint(&info, smpainter.get());
				}
			}
		}
		GLFUNC->glClearColor(0.7f, 0.7f, 0.8f, 1.0f);
		GLFUNC->glViewport(0, 0, this->width(), this->height());
		bNeedInitLight = false;
	}
	mtrfbo->bind();
	GLFUNC->glEnable(GL_DEPTH_TEST);
	GLFUNC->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	debugOpenGL();
	PaintInfomation info;
	info.projMat = camera->getProjMatrix();
	info.viewMat = camera->getViewMatrix();
	info.lights = lights_;
	
	if(wfmode_) info.fillmode = FILL_WIREFRAME;
	else info.fillmode = FILL;

	info.lineWidth = 1.5f;
	info.pointSize = 8.0f;
	info.width = width();
	info.height = height();

	for (auto& prim : prims_) {
		prim.second->paint(&info, mtrPainter.get());
	}

	//Deferred Shading
	info.mtrTexIdx = mtrfbo->transferedTextureId();
	//direct light
	fbo1->bind();
	GLFUNC->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	billboard->paint(&info, deferredDirLightPainter.get());

	info.mtrTexIdx[0] = fbo1->textureId();
	fbo2->bind();
	GLFUNC->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	billboard->paint(&info, deferredSsdoPaintVisitor.get());
	

	//Forward Shading
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

	//Gamma 校正
	GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
	GLFUNC->glActiveTexture(GL_TEXTURE0);
	GLFUNC->glBindTexture(GL_TEXTURE_2D, fbo2->textureId());
	billboard->paint(&info, toneMapPainter.get());
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
