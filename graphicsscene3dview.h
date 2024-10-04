#ifndef GRAPHICSSCENE3DVIEW_H
#define GRAPHICSSCENE3DVIEW_H

#include <QQuickFramebufferObject>
#include <QtMath>
#include "coordinateaxes.h"
#include "planegrid.h"
#include "raycaster.h"
#include "surface.h"
#include "side_scan_view.h"
#include "boattrack.h"
#include "bottomtrack.h"
#include "polygongroup.h"
#include "pointgroup.h"
#include "vertexeditingdecorator.h"
#include "ray.h"
#include "navigation_arrow.h"


class Dataset;
class GraphicsScene3dRenderer;
class GraphicsScene3dView : public QQuickFramebufferObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(GraphicsScene3dView)

public:
    //Camera
    class Camera
    {
    public:
        Camera();
        Camera(qreal pitch,
               qreal yaw,
               qreal distToFocusPoint,
               qreal fov,
               qreal sensivity);

        qreal distToFocusPoint() const;
        qreal fov() const;
        qreal pitch() const;
        qreal yaw() const;
        QMatrix4x4 viewMatrix() const;

        //TODO! Process this method later
        //void rotate(qreal yaw, qreal pitch);
        void rotate(const QVector2D& lastMouse, const QVector2D& mousePos);
        void rotate(const QPointF& prevCenter, const QPointF& currCenter, qreal angleDelta, qreal widgetHeight);
        //void move(const QVector2D& startPos, const QVector2D& endPos);
        void move(const QVector2D &lastMouse, const QVector2D &mousePos);
        void moveZAxis(float z);
        void zoom(qreal delta);
        void commitMovement();
        void focusOnObject(std::weak_ptr<SceneObject> object);
        void focusOnPosition(const QVector3D& pos);
        void setDistance(qreal distance);
        void setIsometricView();
        void setMapView();
        void reset();

    private:
        void updateViewMatrix(QVector3D* lookAt = nullptr);
        void checkRotateAngle();
    private:
        friend class GraphicsScene3dView;
        friend class GraphicsScene3dRenderer;

        QVector3D m_eye = {0.0f, 0.0f, 0.0f};
        QVector3D m_up = {0.0f, 1.0f, 0.0f};
        QVector3D m_lookAt = {0.0f, 0.0f, 0.0f};
        QVector3D m_relativeOrbitPos = {0.0f, 0.0f, 0.0f};

        QMatrix4x4 m_view;

        std::weak_ptr<SceneObject> m_focusedObject;
        QVector3D m_offset;
        QVector3D m_deltaOffset;
        QVector3D m_focusPoint;

        qreal m_pitch = 0.f;
        qreal m_yaw = 0.f;
        qreal m_fov = 45.f;
        float m_distToFocusPoint = 50.f;
        qreal m_sensivity = 4.f;

        QVector2D m_rotAngle;
    };

    //Renderer
    class InFboRenderer : public QQuickFramebufferObject::Renderer
    {
    public:
        InFboRenderer();
        virtual ~InFboRenderer();

    protected:
        virtual void render() override;
        virtual void synchronize(QQuickFramebufferObject * fbo) override;
        virtual QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

    private:
        friend class GraphicsScene3dView;
        void processColorTableTexture(GraphicsScene3dView* viewPtr) const;
        void processTileTexture(GraphicsScene3dView* viewPtr) const;
        std::unique_ptr <GraphicsScene3dRenderer> m_renderer;
    };

    enum ActiveMode{
        Idle                                = 0, // not used
        BottomTrackVertexSelectionMode      = 1,
        BottomTrackVertexComboSelectionMode = 2,
        PolygonCreationMode                 = 3,
        MarkCreationMode                    = 4,
        PolygonEditingMode                  = 5,
        BottomTrackSyncPointCreationMode    = 6,
        ShoreCreationMode                   = 7,
        MeasuringRouteCreationMode          = 8
    };

    /**
     * @brief Constructor
     */
    GraphicsScene3dView();

    /**
     * @brief Destructor
     */
    virtual ~GraphicsScene3dView();

    /**
     * @brief Creates renderer
     * @return renderer
     */
    Renderer *createRenderer() const override;
    std::shared_ptr<BoatTrack> boatTrack() const;
    std::shared_ptr<BottomTrack> bottomTrack() const;
    std::shared_ptr<Surface> surface() const;
    std::shared_ptr<SideScanView> getSideScanViewPtr() const;
    std::shared_ptr<PointGroup> pointGroup() const;
    std::shared_ptr<PolygonGroup> polygonGroup() const;
    std::weak_ptr <Camera> camera() const;
    float verticalScale() const;
    bool sceneBoundingBoxVisible() const;
    Dataset* dataset() const;
    void setNavigationArrowState(bool state);
    void clear();
    QVector3D calculateIntersectionPoint(const QVector3D &rayOrigin, const QVector3D &rayDirection, float planeZ);
    void setCalcStateSideScanView(bool state);
    void interpolateDatasetEpochs();

    Q_INVOKABLE void switchToBottomTrackVertexComboSelectionMode(qreal x, qreal y);
    Q_INVOKABLE void mousePressTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, Qt::Key keyboardKey = Qt::Key::Key_unknown);
    Q_INVOKABLE void mouseMoveTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, Qt::Key keyboardKey = Qt::Key::Key_unknown);
    Q_INVOKABLE void mouseReleaseTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, Qt::Key keyboardKey = Qt::Key::Key_unknown);
    Q_INVOKABLE void mouseWheelTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, QPointF angleDelta, Qt::Key keyboardKey = Qt::Key::Key_unknown);
    Q_INVOKABLE void pinchTrigger(const QPointF& prevCenter, const QPointF& currCenter, qreal scaleDelta, qreal angleDelta);
    Q_INVOKABLE void keyPressTrigger(Qt::Key key);
    Q_INVOKABLE void bottomTrackActionEvent(BottomTrack::ActionEvent actionEvent);

public Q_SLOTS:
    void setSceneBoundingBoxVisible(bool visible);
    void fitAllInView();
    void setIsometricView();
    void setCancelZoomView();
    void setMapView();
    void setLastEpochFocusView();
    void setIdleMode();
    void setVerticalScale(float scale);
    void shiftCameraZAxis(float shift);
    void setBottomTrackVertexSelectionMode();
    void setPolygonCreationMode();
    void setPolygonEditingMode();
    void setDataset(Dataset* dataset);
    void addPoints(QVector<QVector3D>, QColor color, float width = 1);
    void setQmlEngine(QObject* engine);

private:
    void updateBounds();
    void updatePlaneGrid();
    void clearComboSelectionRect();

private:
    friend class BottomTrack;

    std::shared_ptr<Camera> m_camera;
    std::shared_ptr<Camera> m_axesThumbnailCamera;
    QPointF m_startMousePos = {0.0f, 0.0f};
    QPointF m_lastMousePos = {0.0f, 0.0f};
    std::shared_ptr<RayCaster> m_rayCaster;
    std::shared_ptr<Surface> m_surface;
    std::shared_ptr<SideScanView> sideScanView_;
    std::shared_ptr<BoatTrack> m_boatTrack;
    std::shared_ptr<BottomTrack> m_bottomTrack;
    std::shared_ptr<PolygonGroup> m_polygonGroup;
    std::shared_ptr<PointGroup> m_pointGroup;
    std::shared_ptr<CoordinateAxes> m_coordAxes;
    std::shared_ptr<PlaneGrid> m_planeGrid;
    std::shared_ptr<SceneObject> m_vertexSynchroCursour;
    std::shared_ptr<NavigationArrow> m_navigationArrow;

    QMatrix4x4 m_model;
    QMatrix4x4 m_projection;
    Cube m_bounds;
    ActiveMode m_mode = ActiveMode::BottomTrackVertexSelectionMode;
    ActiveMode lastMode_ = ActiveMode::BottomTrackVertexSelectionMode;
    QRect m_comboSelectionRect = { 0, 0, 0, 0 };
    Ray m_ray;
    float m_verticalScale = 1.0f;
    bool m_isSceneBoundingBoxVisible = true;
    Dataset* m_dataset = nullptr;
    bool navigationArrowState_;
    bool sideScanCalcState_;
#if defined (Q_OS_ANDROID)
    static constexpr double mouseThreshold_{ 15.0 };
#else
    static constexpr double mouseThreshold_{ 10.0 };
#endif
    bool wasMoved_;
    Qt::MouseButtons wasMovedMouseButton_;
    QObject* engine_ = nullptr;
    bool switchedToBottomTrackVertexComboSelectionMode_;
    int bottomTrackWindowCounter_;
};

#endif // GRAPHICSSCENE3DVIEW_H
