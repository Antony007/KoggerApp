#include "image_view_control_menu_controller.h"
#include "graphicsscene3dview.h"
#include "image_view.h"



ImageViewControlMenuController::ImageViewControlMenuController(QObject *parent) :
    QmlComponentController(parent),
    m_graphicsSceneView(nullptr)
{ }

void ImageViewControlMenuController::setGraphicsSceneView(GraphicsScene3dView *sceneView)
{
    m_graphicsSceneView = sceneView;
}

void ImageViewControlMenuController::onVisibilityChanged(bool state)
{
    if (m_graphicsSceneView) {
        m_graphicsSceneView->getImageViewPtr()->setVisible(state);
    }
}

void ImageViewControlMenuController::onUseFilterChanged(bool state)
{
    if (m_graphicsSceneView) {
        m_graphicsSceneView->getImageViewPtr()->setUseLinearFilter(state);
    }
}

void ImageViewControlMenuController::onUpdateClicked(const QString& imagePath, int x1, int y1, int x2, int y2, int z)
{
    if (m_graphicsSceneView) {

        QVector3D lt {static_cast<float>(x1), static_cast<float>(y1), static_cast<float>(z)};
        QVector3D rb {static_cast<float>(x2), static_cast<float>(y2), static_cast<float>(z)};

        m_graphicsSceneView->getImageViewPtr()->updateTexture(imagePath, lt, rb);;
    }
}

ImageView* ImageViewControlMenuController::getImageViewPtr() const
{
    if (m_graphicsSceneView) {
        return m_graphicsSceneView->getImageViewPtr().get();
    }
    return nullptr;
}

void ImageViewControlMenuController::findComponent()
{
    m_component = m_engine->findChild<QObject*>("imageViewControlMenu");
}
