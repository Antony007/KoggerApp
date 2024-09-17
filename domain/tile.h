#pragma once

#include <QUuid>
#include <QImage>
#include <QVector>
#include <QVector3D>
#include <QVector2D>
#include <QOpenGLFunctions>
#include "sceneobject.h"


class GlobalMesh;
class Tile {
public:
    /*methods*/
    Tile();
    void initTile(QVector3D origin, int sidePixelSize, int heightMatrixRatio, float resolution, QImage::Format imageFormat = QImage::Format_Indexed8);

    void setSomeInt(int val);
    void setTextureId(GLuint val);
    void setIsUpdate(bool val);
    QUuid                                    getUuid() const;
    QVector3D                                getOrigin() const;
    bool                                     getIsInited() const;
    int                                      getSomeInt() const;
    GLuint                                   getTextureId() const;
    int                                      getIsUpdate() const;
    QImage&                                  getImageRef();
    QVector<QVector3D>&                      getHeightVerticesRef();
    const QVector<QVector2D>&                getTextureVerticesRef() const;
    const QVector<QVector3D>&                getHeightVerticesRef() const;
    const QVector<int>&                      getHeightIndicesRef() const;
    const SceneObject::RenderImplementation& getGridRenderImplRef() const;

private:
    /*data*/
    QUuid id_;
    QVector3D origin_;
    QImage image_;
    QVector<QVector3D> heightVertices_;
    QVector<int> heightIndices_;
    QVector<QVector2D> textureVertices_;
    SceneObject::RenderImplementation gridRenderImpl_;
    GLuint textureId_;
    bool isUpdate_;
    bool isInited_;
    int someInt_; // debug
};
