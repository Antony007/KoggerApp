#pragma once

#include "sceneobject.h"
#include <QVector>
#include "plotcash.h"


class SideScanView : public SceneObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SideScanView)

public:
    class SideScanViewRenderImplementation : public SceneObject::RenderImplementation
    {
    public:
        SideScanViewRenderImplementation();
        virtual void render(QOpenGLFunctions* ctx, const QMatrix4x4& mvp, const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>>& shaderProgramMap) const override final;
    private:
        friend class SideScanView;

        QVector<int> evenIndices_;
        QVector<int> oddIndices_;
    };

    explicit SideScanView(QObject* parent = nullptr);
    virtual ~SideScanView();

    void updateData();

    void clear();
    void setDatasetPtr(Dataset* datasetPtr);

private:
    /*data*/
    Dataset* datasetPtr_ = nullptr;
};
