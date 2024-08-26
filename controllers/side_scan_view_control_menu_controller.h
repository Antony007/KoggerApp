#pragma once

#include "qmlcomponentcontroller.h"
#include "side_scan_view.h"


class SideScanView;
class GraphicsScene3dView;
class SideScanViewControlMenuController : public QmlComponentController
{
    Q_OBJECT
    Q_PROPERTY(SideScanView* sideScanView READ getSideScanViewPtr CONSTANT)

public:
    explicit SideScanViewControlMenuController(QObject *parent = nullptr);
    void setGraphicsSceneView(GraphicsScene3dView* sceneView);

    Q_INVOKABLE void onSideScanViewVisibilityCheckBoxCheckedChanged(bool checked);

    Q_INVOKABLE void onUpdateSideScanViewButtonClicked(const QString& imagePath, const QString& heightMatrixPath);
    Q_INVOKABLE void onClearSideScanViewButtonClicked();
    Q_INVOKABLE void onScaleSideScanViewSpinBoxValueChanged(int scaleFactor);

Q_SIGNALS:

protected:
    virtual void findComponent() override;

private:
    SideScanView* getSideScanViewPtr() const;

    GraphicsScene3dView* m_graphicsSceneView = nullptr;
};
