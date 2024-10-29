#include "graphicsscene3dview.h"
#include <graphicsscene3drenderer.h>
#include <surface.h>
#include <plotcash.h>

#include <cmath>
#include <memory.h>
#include <math.h>

#include <QOpenGLFramebufferObject>
#include <QVector3D>


GraphicsScene3dView::GraphicsScene3dView() :
    QQuickFramebufferObject(),
    m_camera(std::make_shared<Camera>()),
    m_axesThumbnailCamera(std::make_shared<Camera>()),
    m_rayCaster(std::make_shared<RayCaster>()),
    m_surface(std::make_shared<Surface>()),
    sideScanView_(std::make_shared<SideScanView>()),
    imageView_(std::make_shared<ImageView>()),
    m_boatTrack(std::make_shared<BoatTrack>(this, this)),
    m_bottomTrack(std::make_shared<BottomTrack>(this, this)),
    m_polygonGroup(std::make_shared<PolygonGroup>()),
    m_pointGroup(std::make_shared<PointGroup>()),
    m_coordAxes(std::make_shared<CoordinateAxes>()),
    m_planeGrid(std::make_shared<PlaneGrid>()),
    m_navigationArrow(std::make_shared<NavigationArrow>()),
    usblView_(std::make_shared<UsblView>()),
    navigationArrowState_(true),
    wasMoved_(false),
    wasMovedMouseButton_(Qt::MouseButton::NoButton),
    switchedToBottomTrackVertexComboSelectionMode_(false),
    bottomTrackWindowCounter_(-1)
{
    setObjectName("GraphicsScene3dView");
    setMirrorVertically(true);
    setAcceptedMouseButtons(Qt::AllButtons);

    m_boatTrack->setColor({80,0,180});
    m_boatTrack->setWidth(6.0f);

    m_navigationArrow->setColor({ 255, 0, 0 });

    sideScanView_->setView(this);
    imageView_->setView(this);

#ifdef SEPARATE_READING
    sideScanCalcState_ = true;
#else
    sideScanCalcState_ = false;
#endif

    QObject::connect(m_surface.get(), &Surface::changed, this, &QQuickFramebufferObject::update);
    QObject::connect(sideScanView_.get(), &SideScanView::changed, this, &QQuickFramebufferObject::update);
    QObject::connect(imageView_.get(), &ImageView::changed, this, &QQuickFramebufferObject::update);
    QObject::connect(m_boatTrack.get(), &BoatTrack::changed, this, &QQuickFramebufferObject::update);
    QObject::connect(m_bottomTrack.get(), &BottomTrack::changed, this, &QQuickFramebufferObject::update);
    QObject::connect(m_polygonGroup.get(), &PolygonGroup::changed, this, &QQuickFramebufferObject::update);
    QObject::connect(m_pointGroup.get(), &PointGroup::changed, this, &QQuickFramebufferObject::update);
    QObject::connect(m_coordAxes.get(), &CoordinateAxes::changed, this, &QQuickFramebufferObject::update);
    QObject::connect(m_planeGrid.get(), &PlaneGrid::changed, this, &QQuickFramebufferObject::update);
    QObject::connect(m_navigationArrow.get(), &NavigationArrow::changed, this, &QQuickFramebufferObject::update);
    QObject::connect(usblView_.get(), &UsblView::changed, this, &QQuickFramebufferObject::update);

    QObject::connect(m_surface.get(), &Surface::boundsChanged, this, &GraphicsScene3dView::updateBounds);
    QObject::connect(sideScanView_.get(), &SideScanView::boundsChanged, this, &GraphicsScene3dView::updateBounds);
    QObject::connect(imageView_.get(), &ImageView::boundsChanged, this, &GraphicsScene3dView::updateBounds);
    QObject::connect(m_bottomTrack.get(), &BottomTrack::boundsChanged, this, &GraphicsScene3dView::updateBounds);
    QObject::connect(m_polygonGroup.get(), &PolygonGroup::boundsChanged, this, &GraphicsScene3dView::updateBounds);
    QObject::connect(m_pointGroup.get(), &PointGroup::boundsChanged, this, &GraphicsScene3dView::updateBounds);
    QObject::connect(m_coordAxes.get(), &CoordinateAxes::boundsChanged, this, &GraphicsScene3dView::updateBounds);
    QObject::connect(m_boatTrack.get(), &PlaneGrid::boundsChanged, this, &GraphicsScene3dView::updateBounds);
    QObject::connect(m_navigationArrow.get(), &NavigationArrow::boundsChanged, this, &GraphicsScene3dView::updateBounds);
    QObject::connect(usblView_.get(), &UsblView::boundsChanged, this, &GraphicsScene3dView::updateBounds);

    updatePlaneGrid();
}

GraphicsScene3dView::~GraphicsScene3dView()
{

}

QQuickFramebufferObject::Renderer *GraphicsScene3dView::createRenderer() const
{
    return new GraphicsScene3dView::InFboRenderer();
}

std::shared_ptr<BoatTrack> GraphicsScene3dView::boatTrack() const
{
    return m_boatTrack;
}

std::shared_ptr<BottomTrack> GraphicsScene3dView::bottomTrack() const
{
    return m_bottomTrack;
}

std::shared_ptr<Surface> GraphicsScene3dView::surface() const
{
    return m_surface;
}

std::shared_ptr<SideScanView> GraphicsScene3dView::getSideScanViewPtr() const
{
    return sideScanView_;
}

std::shared_ptr<ImageView> GraphicsScene3dView::getImageViewPtr() const
{
    return imageView_;
}

std::shared_ptr<PointGroup> GraphicsScene3dView::pointGroup() const
{
    return m_pointGroup;
}

std::shared_ptr<PolygonGroup> GraphicsScene3dView::polygonGroup() const
{
    return m_polygonGroup;
}

std::shared_ptr<UsblView> GraphicsScene3dView::getUsblViewPtr() const
{
    return usblView_;
}

std::weak_ptr<GraphicsScene3dView::Camera> GraphicsScene3dView::camera() const
{
    return m_camera;
}

float GraphicsScene3dView::verticalScale() const
{
    return m_verticalScale;
}

bool GraphicsScene3dView::sceneBoundingBoxVisible() const
{
    return m_isSceneBoundingBoxVisible;
}

Dataset *GraphicsScene3dView::dataset() const
{
    return m_dataset;
}

void GraphicsScene3dView::setNavigationArrowState(bool state)
{
    m_navigationArrow->setEnabled(state);
    navigationArrowState_ = state;
}

void GraphicsScene3dView::clear()
{
    m_surface->clearData();
    sideScanView_->clear();
    imageView_->clear();//
    bottomTrackWindowCounter_ = -1;
    m_boatTrack->clearData();
    m_bottomTrack->clearData();
    m_polygonGroup->clearData();
    m_pointGroup->clearData();
    m_navigationArrow->clearData();
    navigationArrowState_ = false;
    usblView_->clearData();
    m_bounds = Cube();

    //setMapView();
    updateBounds();

    QQuickFramebufferObject::update();
}

QVector3D GraphicsScene3dView::calculateIntersectionPoint(const QVector3D &rayOrigin, const QVector3D &rayDirection, float planeZ) {
    QVector3D retVal;
    if (qAbs(rayDirection.z()) < 1e-6)
        return retVal;
    const float t = (planeZ - rayOrigin.z()) / rayDirection.z();
    if (t < 0)
        return retVal;
    retVal = rayOrigin + rayDirection * t;

    return retVal;
}

void GraphicsScene3dView::setCalcStateSideScanView(bool state)
{
    sideScanCalcState_ = state;
}

void GraphicsScene3dView::interpolateDatasetEpochs(bool fromStart)
{
    if (m_dataset) {
        m_dataset->interpolateData(fromStart);
    }
}

void GraphicsScene3dView::switchToBottomTrackVertexComboSelectionMode(qreal x, qreal y)
{
    switchedToBottomTrackVertexComboSelectionMode_ = true;

    m_bottomTrack->resetVertexSelection();
    m_boatTrack->clearSelectedEpoch();
    lastMode_ = m_mode;
    m_mode = ActiveMode::BottomTrackVertexComboSelectionMode;
    m_comboSelectionRect.setTopLeft({ static_cast<int>(x), static_cast<int>(height() - y) });
    m_comboSelectionRect.setBottomRight({ static_cast<int>(x), static_cast<int>(height() - y) });

}

void GraphicsScene3dView::mousePressTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, Qt::Key keyboardKey)
{
    Q_UNUSED(keyboardKey)

    wasMoved_ = false;
    clearComboSelectionRect();

    if (engine_) { // maybe this will be removed
        if (auto selectionToolButton = engine_->findChild<QObject*>("selectionToolButton"); selectionToolButton) {
            selectionToolButton->property("checked").toBool() ? m_mode = ActiveMode::BottomTrackVertexSelectionMode : m_mode = ActiveMode::Idle;
        }
    }

    if (mouseButton == Qt::MouseButton::RightButton) {
        switchToBottomTrackVertexComboSelectionMode(x, y);
    }

    m_startMousePos = { x, y };
    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::mouseMoveTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, Qt::Key keyboardKey)
{
    // movement threshold for sync
    if (!wasMoved_) {
        double dist{ std::sqrt(std::pow(x - m_startMousePos.x(), 2) + std::pow(y - m_startMousePos.y(), 2)) };
        if (dist > mouseThreshold_) {
            wasMoved_ = true;
            if (wasMovedMouseButton_ != mouseButton)
                wasMovedMouseButton_ = mouseButton;
        }
    }

    // ray for marker
    auto toOrig = QVector3D(x, height() - y, -1.0f).unproject(m_camera->m_view * m_model, m_projection, boundingRect().toRect());
    auto toEnd = QVector3D(x, height() - y, 1.0f).unproject(m_camera->m_view * m_model, m_projection, boundingRect().toRect());
    auto toDir = (toEnd - toOrig).normalized();
    auto to = calculateIntersectionPoint(toOrig, toDir, 0);
    m_ray.setOrigin(toOrig);
    m_ray.setDirection(toDir);

    if (switchedToBottomTrackVertexComboSelectionMode_) {
        m_comboSelectionRect.setBottomRight({ static_cast<int>(x), static_cast<int>(height() - y) });
        m_bottomTrack->mouseMoveEvent(mouseButton, x, y);
    }
    else {
#ifdef Q_OS_ANDROID
        Q_UNUSED(keyboardKey);
        auto fromOrig = QVector3D(m_startMousePos.x(), height() - m_startMousePos.y(), -1.0f).unproject(m_camera->m_view * m_model, m_projection, boundingRect().toRect());
        auto fromEnd = QVector3D(m_startMousePos.x(), height() - m_startMousePos.y(), 1.0f).unproject(m_camera->m_view * m_model, m_projection, boundingRect().toRect());
        auto fromDir = (fromEnd - fromOrig).normalized();
        auto from = calculateIntersectionPoint(fromOrig, fromDir , 0);
        m_camera->move(QVector2D(from.x(), from.y()), QVector2D(to.x() ,to.y()));
#else
        if (mouseButton.testFlag(Qt::LeftButton) && (keyboardKey == Qt::Key_Control)) {
            m_camera->commitMovement();
            m_camera->rotate(QVector2D(m_lastMousePos), QVector2D(x, y));
            m_axesThumbnailCamera->rotate(QVector2D(m_lastMousePos), QVector2D(x, y));
            m_startMousePos = { x, y };
        }
        else if (mouseButton.testFlag(Qt::LeftButton)) {
            auto fromOrig = QVector3D(m_startMousePos.x(), height() - m_startMousePos.y(), -1.0f).unproject(m_camera->m_view * m_model, m_projection, boundingRect().toRect());
            auto fromEnd = QVector3D(m_startMousePos.x(), height() - m_startMousePos.y(), 1.0f).unproject(m_camera->m_view * m_model, m_projection, boundingRect().toRect());
            auto fromDir = (fromEnd - fromOrig).normalized();
            auto from = calculateIntersectionPoint(fromOrig, fromDir , 0);
            m_camera->move(QVector2D(from.x(), from.y()), QVector2D(to.x() ,to.y()));
        }
#endif
    }

    m_lastMousePos = { x, y };
    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::mouseReleaseTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, Qt::Key keyboardKey)
{
    Q_UNUSED(keyboardKey);

    clearComboSelectionRect();

    m_camera->commitMovement(); //TODO: Commit only if camera in movement state
    m_lastMousePos = { x, y };

    if (switchedToBottomTrackVertexComboSelectionMode_) {
        m_mode = lastMode_;
        m_bottomTrack->mouseReleaseEvent(mouseButton, x, y);
    }

    if (!wasMoved_ && wasMovedMouseButton_ == Qt::MouseButton::NoButton) {
        m_bottomTrack->resetVertexSelection();
        m_boatTrack->clearSelectedEpoch();
        m_bottomTrack->mousePressEvent(Qt::MouseButton::LeftButton, x, y);
        m_boatTrack->mousePressEvent(Qt::MouseButton::LeftButton, x, y);
    }

    switchedToBottomTrackVertexComboSelectionMode_ = false;
    wasMoved_ = false;
    wasMovedMouseButton_ = Qt::MouseButton::NoButton;

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::mouseWheelTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, QPointF angleDelta, Qt::Key keyboardKey)
{
    Q_UNUSED(mouseButton)
    Q_UNUSED(x)
    Q_UNUSED(y)

    if (keyboardKey == Qt::Key_Control) {
        float tempVerticalScale = m_verticalScale;
        angleDelta.y() > 0.0f ? tempVerticalScale += 0.3f : tempVerticalScale -= 0.3f;
        setVerticalScale(tempVerticalScale);
    }
    else if (keyboardKey == Qt::Key_Shift)
        angleDelta.y() > 0.0f ? shiftCameraZAxis(5) : shiftCameraZAxis(-5);
    else
        m_camera->zoom(angleDelta.y());

    updatePlaneGrid();
    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::pinchTrigger(const QPointF& prevCenter, const QPointF& currCenter, qreal scaleDelta, qreal angleDelta)
{
    m_camera->zoom(scaleDelta);

    m_camera->rotate(prevCenter, currCenter, angleDelta, height());
    m_axesThumbnailCamera->rotate(prevCenter, currCenter, angleDelta , height());

    updatePlaneGrid();
    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::keyPressTrigger(Qt::Key key)
{
    m_bottomTrack->keyPressEvent(key);

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::bottomTrackActionEvent(BottomTrack::ActionEvent actionEvent)
{
    m_bottomTrack->actionEvent(actionEvent);

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setSceneBoundingBoxVisible(bool visible)
{
    m_isSceneBoundingBoxVisible = visible;

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::fitAllInView()
{
    auto maxSize = std::max(m_bounds.width(),
                            std::max(m_bounds.height(),
                                     m_bounds.length()));

    auto d = (maxSize/2.0f)/(std::tan(m_camera->fov()/2.0f)) * 2.0f;

    if(d>0)
        m_camera->setDistance(d);

    m_camera->focusOnPosition(m_bounds.center());

    updatePlaneGrid();

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setIsometricView()
{
    m_camera->setIsometricView();
    m_axesThumbnailCamera->setIsometricView();

    fitAllInView();
    updatePlaneGrid();

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setCancelZoomView()
{
    m_verticalScale = 1.0f;

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setMapView() {
    m_camera->setMapView();
    m_axesThumbnailCamera->setMapView();

    fitAllInView();
    updatePlaneGrid();

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setLastEpochFocusView()
{
    auto* epoch = m_dataset->last();
    QVector3D currPos(epoch->getPositionGNSS().ned.n, epoch->getPositionGNSS().ned.e, 1);

    m_camera->focusOnPosition(currPos);
    updatePlaneGrid();

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setIdleMode()
{
    m_mode = Idle; 

    clearComboSelectionRect();
    m_bottomTrack->resetVertexSelection();
    m_boatTrack->clearSelectedEpoch();

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setVerticalScale(float scale)
{
    if(m_verticalScale == scale)
        return;
    else if(scale < 0.05f)
        m_verticalScale = 0.05f;
    else if(scale > 10.f)
        m_verticalScale = 10.0f;
    else
        m_verticalScale = scale;

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::shiftCameraZAxis(float shift)
{
    m_camera->moveZAxis(shift);
}

void GraphicsScene3dView::setBottomTrackVertexSelectionMode()
{
    setIdleMode();

    m_mode = BottomTrackVertexSelectionMode;

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setPolygonCreationMode()
{
    setIdleMode();

    m_mode = PolygonCreationMode;

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setPolygonEditingMode()
{
    setIdleMode();

    m_mode = PolygonEditingMode;

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setDataset(Dataset *dataset)
{
    if (m_dataset)
        QObject::disconnect(m_dataset);

    m_dataset = dataset;

    if (!m_dataset)
        return;

    m_boatTrack->setDatasetPtr(m_dataset);
    m_bottomTrack->setDatasetPtr(m_dataset);
    sideScanView_->setDatasetPtr(m_dataset);

    QObject::connect(m_dataset, &Dataset::bottomTrackUpdated,
                     this,      [this](int lEpoch, int rEpoch) -> void {
                                    clearComboSelectionRect();
                                    m_bottomTrack->isEpochsChanged(lEpoch, rEpoch);
                                    if (sideScanCalcState_) {
                                        interpolateDatasetEpochs(false);
                                    }
                                }, Qt::DirectConnection);

    QObject::connect(m_dataset, &Dataset::boatTrackUpdated,
                      this,     [this]() -> void {
                                    m_boatTrack->setData(m_dataset->boatTrack(), GL_LINE_STRIP);
                                    if (navigationArrowState_) {
                                        const Position pos = m_dataset->getLastPosition();
                                        m_navigationArrow->setPositionAndAngle(
                                            QVector3D(pos.ned.n, pos.ned.e, !isfinite(pos.ned.d) ? 0.f : pos.ned.d), m_dataset->getLastYaw() - 90.f);
                                    }

                                    if (!sideScanCalcState_ || sideScanView_->getWorkMode() != SideScanView::Mode::kRealtime) {
                                        return;
                                    }

                                    // bottom track
                                    int firstChannelId = CHANNEL_NONE;
                                    int secondChannelId = CHANNEL_FIRST;
                                    if (m_dataset) {
                                        if (auto chList = m_dataset->channelsList(); !chList.empty()) {
                                            auto it = chList.begin();
                                            firstChannelId = it.key();

                                            if (++it != chList.end()) {
                                                secondChannelId = it.key();
                                            }
                                        }
                                    }
                                    int currEpochIndx = m_dataset->endIndex();
                                    auto btP = m_dataset->getBottomTrackParamPtr();
                                    int currCount = std::floor(currEpochIndx / btP->windowSize);
                                    if (bottomTrackWindowCounter_ != currCount) {
                                        btP->indexFrom = bottomTrackWindowCounter_ * btP->windowSize;
                                        btP->indexTo = currCount  * btP->windowSize;
                                        m_dataset->bottomTrackProcessing(firstChannelId, secondChannelId);
                                        bottomTrackWindowCounter_ = currCount;;
                                    }
                                }, Qt::DirectConnection);

    QObject::connect(m_dataset, &Dataset::updatedInterpolatedData,
                     this,      [this](int indx) -> void {
                                    if (sideScanView_->getWorkMode() == SideScanView::Mode::kRealtime) {
                                        m_bottomTrack->sideScanUpdated();
                                        sideScanView_->startUpdateDataInThread(indx);
                                        if (sideScanView_->getTrackLastEpoch()) {
                                            setLastEpochFocusView();
                                        }
                                    }
                                }, Qt::DirectConnection);
}

void GraphicsScene3dView::addPoints(QVector<QVector3D> positions, QColor color, float width) {
    for(int i = 0; i < positions.size(); i++) {
        auto p = std::make_shared<PointObject>();
        p->setPosition(positions[i]);
        p->setColor(color);
        p->setWidth(width);
        pointGroup()->append(p);
    }
}

void GraphicsScene3dView::setQmlEngine(QObject* engine)
{
    engine_ = engine;
}

void GraphicsScene3dView::updateBounds()
{
    m_bounds = m_boatTrack->bounds()
                    .merge(m_surface->bounds())
                    .merge(m_bottomTrack->bounds())
                    .merge(m_boatTrack->bounds())
                    .merge(m_polygonGroup->bounds())
                    .merge(m_pointGroup->bounds())
                    .merge(sideScanView_->bounds())
                    .merge(imageView_->bounds())
                    .merge(usblView_->bounds());

    updatePlaneGrid();

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::updatePlaneGrid()
{
    m_planeGrid->setPlane(m_bounds.bottom());
    m_planeGrid->setCellSize(10);
    // if(m_camera->distToFocusPoint() < 65)
    //     m_planeGrid->setCellSize(1);
    // if(m_camera->distToFocusPoint() >= 65 && m_camera->distToFocusPoint() <= 130)
    //     m_planeGrid->setCellSize(3);
    // if(m_camera->distToFocusPoint() >= 130 && m_camera->distToFocusPoint() <= 230)
    //     m_planeGrid->setCellSize(5);
    // if(m_camera->distToFocusPoint() > 230)
    //     m_planeGrid->setCellSize(10);
}

void GraphicsScene3dView::clearComboSelectionRect()
{
    m_comboSelectionRect = { 0, 0, 0, 0 };
}

//---------------------Renderer---------------------------//
GraphicsScene3dView::InFboRenderer::InFboRenderer() :
    QQuickFramebufferObject::Renderer(),
    m_renderer(new GraphicsScene3dRenderer)
{
    m_renderer->initialize();
}

GraphicsScene3dView::InFboRenderer::~InFboRenderer()
{ }

void GraphicsScene3dView::InFboRenderer::render()
{
    m_renderer->render();
}

void GraphicsScene3dView::InFboRenderer::synchronize(QQuickFramebufferObject * fbo)
{
    auto view = qobject_cast<GraphicsScene3dView*>(fbo);

    if (!view) {
        return;
    }

    // process textures
    processColorTableTexture(view);
    processTileTexture(view);
    processImageTexture(view);

    //read from renderer
    view->m_model = m_renderer->m_model;
    view->m_projection = m_renderer->m_projection;

    // write to renderer
    m_renderer->m_coordAxesRenderImpl       = *(dynamic_cast<CoordinateAxes::CoordinateAxesRenderImplementation*>(view->m_coordAxes->m_renderImpl));
    m_renderer->m_planeGridRenderImpl       = *(dynamic_cast<PlaneGrid::PlaneGridRenderImplementation*>(view->m_planeGrid->m_renderImpl));
    m_renderer->m_boatTrackRenderImpl       = *(dynamic_cast<BoatTrack::BoatTrackRenderImplementation*>(view->m_boatTrack->m_renderImpl));
    m_renderer->m_bottomTrackRenderImpl     = *(dynamic_cast<BottomTrack::BottomTrackRenderImplementation*>(view->m_bottomTrack->m_renderImpl));
    m_renderer->m_surfaceRenderImpl         = *(dynamic_cast<Surface::SurfaceRenderImplementation*>(view->m_surface->m_renderImpl));
    m_renderer->sideScanViewRenderImpl_     = *(dynamic_cast<SideScanView::SideScanViewRenderImplementation*>(view->sideScanView_->m_renderImpl));
    m_renderer->imageViewRenderImpl_        = *(dynamic_cast<ImageView::ImageViewRenderImplementation*>(view->imageView_->m_renderImpl));
    m_renderer->m_polygonGroupRenderImpl    = *(dynamic_cast<PolygonGroup::PolygonGroupRenderImplementation*>(view->m_polygonGroup->m_renderImpl));
    m_renderer->m_pointGroupRenderImpl      = *(dynamic_cast<PointGroup::PointGroupRenderImplementation*>(view->m_pointGroup->m_renderImpl));
    m_renderer->navigationArrowRenderImpl_  = *(dynamic_cast<NavigationArrow::NavigationArrowRenderImplementation*>(view->m_navigationArrow->m_renderImpl));
    m_renderer->usblViewRenderImpl_         = *(dynamic_cast<UsblView::UsblViewRenderImplementation*>(view->usblView_->m_renderImpl));
    m_renderer->m_viewSize                  = view->size();
    m_renderer->m_camera                    = *view->m_camera;
    m_renderer->m_axesThumbnailCamera       = *view->m_axesThumbnailCamera;
    m_renderer->m_comboSelectionRect        = view->m_comboSelectionRect;
    m_renderer->m_verticalScale             = view->m_verticalScale;
    m_renderer->m_boundingBox               = view->m_bounds;
    m_renderer->m_isSceneBoundingBoxVisible = view->m_isSceneBoundingBoxVisible;
}

QOpenGLFramebufferObject *GraphicsScene3dView::InFboRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);
    return new QOpenGLFramebufferObject(size, format);
}

void GraphicsScene3dView::InFboRenderer::processColorTableTexture(GraphicsScene3dView* viewPtr) const
{
    auto sideScanPtr = viewPtr->getSideScanViewPtr();
    auto task = sideScanPtr->getColorTableTextureTask();
    if (!task.empty()) {
        GLuint colorTableTextureId = sideScanPtr->getColorTableTextureId();

#if defined(Q_OS_ANDROID)
        if (colorTableTextureId) {
            glBindTexture(GL_TEXTURE_2D, colorTableTextureId);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, task.size() / 4, 1, GL_RGBA, GL_UNSIGNED_BYTE, task.data());
        }
        else {
            glGenTextures(1, &colorTableTextureId);
            glBindTexture(GL_TEXTURE_2D, colorTableTextureId);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, task.size() / 4, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, task.data());

            sideScanPtr->setColorTableTextureId(colorTableTextureId);
        }
#else
        if (colorTableTextureId) {
            glBindTexture(GL_TEXTURE_1D, colorTableTextureId);
            glTexSubImage1D(GL_TEXTURE_1D, 0, 0, task.size() / 4, GL_RGBA, GL_UNSIGNED_BYTE, task.data());
        }
        else {
            glGenTextures(1, &colorTableTextureId);
            glBindTexture(GL_TEXTURE_1D, colorTableTextureId);

            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, task.size() / 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, task.data());

            sideScanPtr->setColorTableTextureId(colorTableTextureId);
        }
#endif
    }
}

void GraphicsScene3dView::InFboRenderer::processTileTexture(GraphicsScene3dView* viewPtr) const
{
    auto sideScanPtr = viewPtr->getSideScanViewPtr();

    auto tasks = sideScanPtr->getTileTextureTasks();

    for (auto it = tasks.begin(); it != tasks.end(); ++it) {
        const QUuid& tileId = it.key();
        const std::vector<uint8_t>& image = it.value();
        GLuint textureId = viewPtr->getSideScanViewPtr()->getTextureIdByTileId(tileId);

        if (image == std::vector<uint8_t>()) { // delete
            sideScanPtr->setTextureIdByTileId(tileId, 0);
            glDeleteTextures(1, &textureId);
            continue;
        }

        if (textureId) {
            glBindTexture(GL_TEXTURE_2D, textureId);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, viewPtr->getSideScanViewPtr()->getUseLinearFilter() ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST); // may be changed
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, viewPtr->getSideScanViewPtr()->getUseLinearFilter() ? GL_LINEAR : GL_NEAREST);

            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RED, GL_UNSIGNED_BYTE, image.data());
        }
        else {
            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_2D, textureId);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, viewPtr->getSideScanViewPtr()->getUseLinearFilter() ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, viewPtr->getSideScanViewPtr()->getUseLinearFilter() ? GL_LINEAR : GL_NEAREST);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 256, 256, 0, GL_RED, GL_UNSIGNED_BYTE, image.data());

            sideScanPtr->setTextureIdByTileId(tileId, textureId);
        }

        QOpenGLFunctions* glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->glGenerateMipmap(GL_TEXTURE_2D);
    }
}

void GraphicsScene3dView::InFboRenderer::processImageTexture(GraphicsScene3dView *viewPtr) const
{
    auto imagePtr = viewPtr->getImageViewPtr();
    auto& task = imagePtr->getTextureTasksRef();

    if (task.isNull())
        return;

    GLuint textureId = viewPtr->getImageViewPtr()->getTextureId();

    if (textureId) {
        glDeleteTextures(1, &textureId);
    }

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, viewPtr->getImageViewPtr()->getUseLinearFilter() ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, viewPtr->getImageViewPtr()->getUseLinearFilter() ? GL_LINEAR : GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    QImage glImage = task.convertToFormat(QImage::Format_RGBA8888).mirrored();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits());

    imagePtr->setTextureId(textureId);

    QOpenGLFunctions* glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->glGenerateMipmap(GL_TEXTURE_2D);

    task = QImage();
}

GraphicsScene3dView::Camera::Camera()
{
    setIsometricView();
}

GraphicsScene3dView::Camera::Camera(qreal pitch,
                                    qreal yaw,
                                    qreal distToFocusPoint,
                                    qreal fov,
                                    qreal sensivity)
    :m_pitch(std::move(pitch))
    ,m_yaw(std::move(yaw))
    ,m_fov(std::move(fov))
    ,m_distToFocusPoint(std::move(distToFocusPoint))
    ,m_sensivity(std::move(sensivity))
{
   setIsometricView();
}

qreal GraphicsScene3dView::Camera::fov() const
{
    return m_fov;
}

qreal GraphicsScene3dView::Camera::pitch() const
{
    return m_pitch;
}

qreal GraphicsScene3dView::Camera::yaw() const
{
    return m_yaw;
}

QMatrix4x4 GraphicsScene3dView::Camera::viewMatrix() const
{
    return m_view;
}

//void GraphicsScene3dView::Camera::rotate(qreal yaw, qreal pitch)
//{
//    QVector3D viewDir = (m_eye-m_lookAt).normalized();
//    QVector3D right = QVector3D::crossProduct(viewDir,m_up).normalized();
//
//    float cosAngle = QVector3D::dotProduct(viewDir,m_up);
//
//    auto sgn = [](float val){
//        return (float(0) < val) - (val < float(0));
//    };
//
//    if(cosAngle * sgn(pitch) > 0.99f)
//        pitch = 0.0f;
//
//    yaw *= m_sensivity * 65.0f;
//    pitch *= m_sensivity * 65.0f;
//
//    QMatrix4x4 rotationMatrixX;
//    rotationMatrixX.setToIdentity();
//    rotationMatrixX.rotate(yaw,m_up);
//    m_relativeOrbitPos = (rotationMatrixX * QVector4D(m_relativeOrbitPos, 1.0f)).toVector3D();
//
//    QMatrix4x4 rotationMatrixY;
//    rotationMatrixY.setToIdentity();
//    rotationMatrixY.rotate(pitch, right);
//    m_relativeOrbitPos = (rotationMatrixY * QVector4D(m_relativeOrbitPos, 1.0f)).toVector3D();
//
//    updateViewMatrix();
//}

void GraphicsScene3dView::Camera::rotate(const QVector2D& lastMouse, const QVector2D& mousePos)
{
    auto r = (lastMouse - mousePos)*0.2;
    r.setX(qDegreesToRadians(r.x()));
    r.setY(qDegreesToRadians(r.y()));

    m_rotAngle += r;

    checkRotateAngle();

    updateViewMatrix();
}

void GraphicsScene3dView::Camera::rotate(const QPointF& prevCenter, const QPointF& currCenter, qreal angleDelta, qreal widgetHeight)
{
    const qreal increaseCoeff{ 1.3 };
    const qreal angleDeltaY = (prevCenter - currCenter).y() / widgetHeight * 90.0;

    m_rotAngle.setX(m_rotAngle.x() - qDegreesToRadians(angleDelta));
    m_rotAngle.setY(m_rotAngle.y() + qDegreesToRadians(angleDeltaY * increaseCoeff));

    checkRotateAngle();
    updateViewMatrix();
}

void GraphicsScene3dView::Camera::move(const QVector2D &startPos, const QVector2D &endPos)
{
    QVector4D horizontalAxis{ -1.0f, 0.0f, 0.0f, 0.0f };
    QVector4D verticalAxis{ 0.0f, -1.0f, 0.0f, 0.0f };

    m_deltaOffset = ((horizontalAxis * (float)(endPos.x() - startPos.x()) +
                      verticalAxis * (float)(endPos.y() - startPos.y()))).toVector3D();

    auto lookAt = m_lookAt + m_deltaOffset;

    updateViewMatrix(&lookAt);
}

void GraphicsScene3dView::Camera::moveZAxis(float z)
{
    m_lookAt.setZ(m_lookAt.z() + z);
    updateViewMatrix();
}

//void GraphicsScene3dView::Camera::move(const QVector2D &lastMouse, const QVector2D &mousePos)
//{
//    QVector3D vm = QVector3D(-(qDegreesToRadians(lastMouse.x()) - qDegreesToRadians(mousePos.x())), (qDegreesToRadians(lastMouse.y()) - qDegreesToRadians(mousePos.y())), 0)*(m_fov*0.002);
//
//    m_focusPosition[0] += (vm[1]*cosf(-m_rotAngle.x())*cosf(m_rotAngle.y()) - vm[0]*sinf(-m_rotAngle.x()));
//    m_focusPosition[1] += (vm[1]*sinf(-m_rotAngle.x())*cosf(m_rotAngle.y()) + vm[0]*cosf(-m_rotAngle.x()));
//    m_focusPosition[2] += -vm[1]*sinf(m_rotAngle.y())*sinf(m_rotAngle.y());
//
//    updateViewMatrix();
//}

void GraphicsScene3dView::Camera::zoom(qreal delta)
{
#ifdef Q_OS_ANDROID
    const float increaseCoeff{ 0.95f };
    m_distToFocusPoint -= delta * m_distToFocusPoint * increaseCoeff;
#else
    m_distToFocusPoint = delta > 0.f ? m_distToFocusPoint / 1.15f : m_distToFocusPoint * 1.15f;
#endif

    const float minFocusDist = 2.0f;
    const float maxFocusDist = 10000.0f;
    if (m_distToFocusPoint < minFocusDist)
        m_distToFocusPoint = minFocusDist;
    if (m_distToFocusPoint >= maxFocusDist)
        m_distToFocusPoint = maxFocusDist;

    updateViewMatrix();
}

void GraphicsScene3dView::Camera::commitMovement()
{
    m_lookAt += m_deltaOffset;
    m_deltaOffset = QVector3D();

    updateViewMatrix();
}

void GraphicsScene3dView::Camera::focusOnObject(std::weak_ptr<SceneObject> object)
{
    Q_UNUSED(object)
}

void GraphicsScene3dView::Camera::focusOnPosition(const QVector3D &point)
{
    m_lookAt = point;

    updateViewMatrix();
}

void GraphicsScene3dView::Camera::setDistance(qreal distance)
{
    m_distToFocusPoint = distance;

    updateViewMatrix();
}

void GraphicsScene3dView::Camera::setIsometricView()
{
    reset();

    m_rotAngle.setX(qDegreesToRadians(135.0f));
    m_rotAngle.setY(qDegreesToRadians(45.0f));

    updateViewMatrix();
}

void GraphicsScene3dView::Camera::setMapView() {
    reset();

    m_rotAngle.setX(qDegreesToRadians(0.0f));
    m_rotAngle.setY(qDegreesToRadians(0.0f));

    updateViewMatrix();
}

void GraphicsScene3dView::Camera::reset()
{
    m_eye = {0.0f, 0.0f, 20.0f};
    m_lookAt = {0.0f, 0.0f, 0.0f};
    m_relativeOrbitPos = m_eye;

    m_focusedObject.lock() = nullptr;
    m_deltaOffset = {0.0f, 0.0f, 0.0f};
    m_focusPoint = {0.0f, 0.0f, 0.0f};

    m_pitch = 0.f;
    m_yaw = 0.f;
    m_fov = 45.f;
    m_distToFocusPoint = 50.f;

    updateViewMatrix();
}

void GraphicsScene3dView::Camera::updateViewMatrix(QVector3D* lookAt)
{
    auto _lookAt = lookAt ? *lookAt : m_lookAt;
    _lookAt.setZ(-_lookAt.z());

    QVector3D cf;
    cf[0] = -sinf(m_rotAngle.y())*cosf(-m_rotAngle.x())*m_distToFocusPoint;
    cf[1] = -sinf(m_rotAngle.y())*sinf(-m_rotAngle.x())*m_distToFocusPoint;
    cf[2] = -cosf(m_rotAngle.y())*m_distToFocusPoint;

    QVector3D cu;
    cu[0] = cosf(m_rotAngle.y())*cosf(-m_rotAngle.x());
    cu[1] = cosf(m_rotAngle.y())*sinf(-m_rotAngle.x());
    cu[2] = -sinf(m_rotAngle.y());

    QMatrix4x4 view;
    view.lookAt(cf + _lookAt, _lookAt, cu.normalized());
    view.scale(1.0f,1.0f,-1.0f);

    m_view = std::move(view);
}

void GraphicsScene3dView::Camera::checkRotateAngle()
{
    if (m_rotAngle[1] > M_PI_2)
        m_rotAngle[1] = M_PI_2;
    else if (m_rotAngle[1] < 0)
        m_rotAngle[1] = 0;
}


qreal GraphicsScene3dView::Camera::distToFocusPoint() const
{
    return m_distToFocusPoint;
}
