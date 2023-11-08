#ifndef GRAPHICSSCENE3DVIEW_H
#define GRAPHICSSCENE3DVIEW_H

#include <coordinateaxes.h>
#include <planegrid.h>
#include <raycaster.h>
#include <surface.h>
#include <bottomtrack.h>
#include <polygongroup.h>
#include <pointgroup.h>
#include <vertexeditingdecorator.h>

#include <QQuickFramebufferObject>
#include <QtMath>

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
        void rotate(qreal yaw, qreal pitch);
        void move(const QVector2D& startPos, const QVector2D& endPos);
        void zoom(qreal delta);
        void commitMovement();
        void focusOnObject(std::weak_ptr<SceneObject> object);
        void focusOnPosition(const QVector3D& pos);
        void setDistance(qreal distance);
        void setIsometricView();
        void reset();

    private:
        void updateViewMatrix(QVector3D* lookAt = nullptr);
    private:
        friend class GraphicsScene3dView;
        friend class GraphicsScene3dRenderer;

        QVector3D m_eye = {0.0f, 0.0f, 0.0f};
        QVector3D m_up = {0.0f, 1.0f, 0.0f};
        QVector3D m_lookAt = {0.0f, 0.0f, 0.0f};
        QVector3D m_relativeOrbitPos = {0.0f, 0.0f, 0.0f};
        QPointF m_startDragPos = {0.0f, 0.0f};
        bool m_useFocusPoint = true;

        QMatrix4x4 m_view;

        std::weak_ptr<SceneObject> m_focusedObject;
        QVector3D m_offset;
        QVector3D m_deltaOffset;
        QVector3D m_focusPoint;

        qreal m_pitch = 0.0f;
        qreal m_yaw = 0.0f;
        qreal m_fov = 45.0f;
        qreal m_distToFocusPoint = 25.0f;
        qreal m_sensivity = 0.5f;
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
        std::unique_ptr <GraphicsScene3dRenderer> m_renderer;
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
    std::shared_ptr <BottomTrack> bottomTrack() const;
    std::shared_ptr <Surface> surface() const;
    std::shared_ptr <PointGroup> pointGroup() const;
    std::shared_ptr <PolygonGroup> polygonGroup() const;
    std::weak_ptr <Camera> camera() const;
    void clear();
    void setBottomTrackVertexEditingModeEnabled(bool enabled);

    Q_INVOKABLE void mouseMoveTrigger(Qt::MouseButtons buttons, qreal x, qreal y);
    Q_INVOKABLE void mousePressTrigger(Qt::MouseButtons buttons, qreal x, qreal y);
    Q_INVOKABLE void mouseReleaseTrigger(Qt::MouseButtons buttons, qreal x, qreal y);
    Q_INVOKABLE void mouseWheelTrigger(Qt::MouseButtons buttons, qreal x, qreal y, QPointF angleDelta);
    Q_INVOKABLE void keyPressTrigger(Qt::Key key);

public Q_SLOTS:
    void fitAllInView();
    void setIsometricView();

private:
    void updateBounds();
    void updatePlaneGrid();

private:
    std::shared_ptr <Camera> m_camera;
    std::shared_ptr <Camera> m_axesThumbnailCamera;
    QPointF m_startMousePos = {0.0f, 0.0f};
    QPointF m_lastMousePos = {0.0f, 0.0f};
    std::shared_ptr <RayCaster> m_rayCaster;
    std::shared_ptr <Surface> m_surface;
    std::shared_ptr <BottomTrack> m_bottomTrack;
    std::shared_ptr <PolygonGroup> m_polygonGroup;
    std::shared_ptr <PointGroup> m_pointGroup;
    std::shared_ptr <CoordinateAxes> m_coordAxes;
    std::shared_ptr <PlaneGrid> m_planeGrid;
    std::shared_ptr <SceneObject> m_sceneBoundsPlane;
    std::unique_ptr <VertexEditingDecorator> m_vertexEditingDecorator;
    QMatrix4x4 m_model;
    QMatrix4x4 m_projection;
    Cube m_bounds;
    bool m_vertexEditingToolEnabled = false;
};

#endif // GRAPHICSSCENE3DVIEW_H
