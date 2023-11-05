#include "surface.h"
#include <boundarydetector.h>
#include <Triangle.h>
#include <drawutils.h>

Surface::Surface(QObject* parent)
: SceneObject(new SurfaceRenderImplementation, parent)
, m_contour(std::make_shared <Contour>())
, m_grid(std::make_shared <SurfaceGrid>())
{
    QObject::connect(m_grid.get(), &SurfaceGrid::changed, [this](){
        RENDER_IMPL(Surface)->m_gridRenderImpl = *m_grid->m_renderImpl;
        Q_EMIT changed();
    });

    QObject::connect(m_contour.get(), &Contour::changed, [this](){
        RENDER_IMPL(Surface)->m_contourRenderImpl = *m_contour->m_renderImpl;
        Q_EMIT changed();
    });
}

Surface::~Surface()
{}

SceneObject::SceneObjectType Surface::type() const
{
    return SceneObjectType::Surface;
}

Contour *Surface::contour() const
{
    return m_contour.get();
}

SurfaceGrid *Surface::grid() const
{
    return m_grid.get();
}

void Surface::setData(const QVector<QVector3D>& data, int primitiveType)
{
    blockSignals(true);
    SceneObject::setData(data, primitiveType);
    blockSignals(false);

    updateGrid();
    updateContour();

    Q_EMIT changed();
}

void Surface::clearData()
{
    SceneObject::clearData();

    m_grid->clearData();
    m_contour->clearData();
}

void Surface::updateGrid()
{
    m_grid->clearData();

    switch(RENDER_IMPL(Surface)->primitiveType()){
    case GL_TRIANGLES:
        makeTriangleGrid();
        break;
    case GL_QUADS:
        makeQuadGrid();
        break;
    default:
        break;
    }
}

void Surface::makeTriangleGrid()
{
    auto impl = RENDER_IMPL(Surface);

    if (impl->cdata().size() < 3)
        return;

    QVector <QVector3D> grid;

    for (int i = 0; i < impl->cdata().size()-3; i+=3){
        QVector3D A = impl->cdata()[i];
        QVector3D B = impl->cdata()[i+1];
        QVector3D C = impl->cdata()[i+2];

        A.setZ(A.z() + 0.03);
        B.setZ(B.z() + 0.03);
        C.setZ(C.z() + 0.03);

        grid.append({A, B,
                     B, C,
                     A, C});
    }

    m_grid->setData(grid, GL_LINES);
    impl->m_gridRenderImpl = *m_grid->m_renderImpl;
}

void Surface::makeQuadGrid()
{
    m_grid->clearData();

    auto impl = RENDER_IMPL(Surface);

    if (impl->cdata().size() < 4)
        return;

    QVector <QVector3D> grid;

    for (int i = 0; i < impl->cdata().size()-4; i+=4){
        QVector3D A = impl->cdata()[i];
        QVector3D B = impl->cdata()[i+1];
        QVector3D C = impl->cdata()[i+2];
        QVector3D D = impl->cdata()[i+3];

        A.setZ(A.z() + 0.03);
        B.setZ(B.z() + 0.03);
        C.setZ(C.z() + 0.03);
        D.setZ(D.z() + 0.03);

        grid.append({A, B,
                     B, C,
                     C, D,
                     A, D});
    }

    m_grid->setData(grid, GL_LINES);
    impl->m_gridRenderImpl = *m_grid->m_renderImpl;
}

void Surface::makeContourFromTriangles()
{
    auto impl = RENDER_IMPL(Surface);
    auto surfaceData = impl->cdata();

    BoundaryDetector <float> boundaryDetector;

    std::vector <::Triangle <float>> temp;

    for(int i = 0; i < surfaceData.size()-3; i+=3){
        temp.push_back(::Triangle <float>(
                            Point3D <float>(surfaceData[i].x(),   surfaceData[i].y(),   surfaceData[i].z()),
                            Point3D <float>(surfaceData[i+1].x(), surfaceData[i+1].y(), surfaceData[i+1].z()),
                            Point3D <float>(surfaceData[i+2].x(), surfaceData[i+2].y(), surfaceData[i+2].z())
                        ));
    }

    auto boundary = boundaryDetector.detect(temp);

    QVector<QVector3D> contour;

    for(const auto& segment : boundary){
        contour.append({segment.p1().toQVector3D(),
                          segment.p2().toQVector3D()
                    });
    };

    m_contour->setData(contour, GL_LINES);
    impl->m_contourRenderImpl = *m_contour->m_renderImpl;
}

void Surface::makeContourFromQuads()
{
    auto impl = RENDER_IMPL(Surface);
    auto surfaceData = impl->cdata();

    BoundaryDetector <float> boundaryDetector;

    std::vector <::Quad <float>> temp;

    for(int i = 0; i < surfaceData.size()-4; i+=4){
        temp.push_back(::Quad <float>(
                            Point3D <float>(surfaceData[i].x(),   surfaceData[i].y(),   surfaceData[i].z()),
                            Point3D <float>(surfaceData[i+1].x(), surfaceData[i+1].y(), surfaceData[i+1].z()),
                            Point3D <float>(surfaceData[i+2].x(), surfaceData[i+2].y(), surfaceData[i+2].z()),
                            Point3D <float>(surfaceData[i+3].x(), surfaceData[i+3].y(), surfaceData[i+3].z())
                        ));
    }

    auto boundary = boundaryDetector.detect(temp);

    QVector<QVector3D> contour;

    for(const auto& segment : boundary){
        contour.append({segment.p1().toQVector3D(),
                          segment.p2().toQVector3D()
                         });
    };

    m_contour->setData(contour, GL_LINES);
    impl->m_contourRenderImpl = *m_contour->m_renderImpl;
}

void Surface::updateContour()
{
    m_contour->clearData();

    switch(RENDER_IMPL(Surface)->primitiveType()){
    case GL_TRIANGLES:
        makeContourFromTriangles();
        break;
    case GL_QUADS:
        makeContourFromQuads();
        break;
    default:
        break;
    }
}

void Surface::SurfaceRenderImplementation::render(QOpenGLFunctions *ctx, const QMatrix4x4 &mvp, const QMap<QString, std::shared_ptr<QOpenGLShaderProgram> > &shaderProgramMap) const
{
    m_gridRenderImpl.render(ctx, mvp, shaderProgramMap);
    m_contourRenderImpl.render(ctx, mvp, shaderProgramMap);

    if(!m_isVisible)
        return;

    if(!shaderProgramMap.contains("height"))
        return;

    auto shaderProgram = shaderProgramMap["height"];

    if (!shaderProgram->bind()){
        qCritical() << "Error binding shader program.";
        return;
    }

    int posLoc    = shaderProgram->attributeLocation("position");
    int maxYLoc   = shaderProgram->uniformLocation("max_y");
    int minYLoc   = shaderProgram->uniformLocation("min_y");
    int matrixLoc = shaderProgram->uniformLocation("matrix");
    int selectedPrimitiveFirstIndex = shaderProgram->uniformLocation("selectedPrimitiveFirstIndex");
    int selectedPrimitiveLastIndex = shaderProgram->uniformLocation("selectedPrimitiveLastIndex");

    shaderProgram->setUniformValue(maxYLoc, m_bounds.maximumY());
    shaderProgram->setUniformValue(minYLoc, m_bounds.minimumY());
    shaderProgram->setUniformValue(selectedPrimitiveFirstIndex, m_selectedIndices.first);
    shaderProgram->setUniformValue(selectedPrimitiveLastIndex, m_selectedIndices.second);
    shaderProgram->setUniformValue(matrixLoc, mvp);
    shaderProgram->enableAttributeArray(posLoc);
    shaderProgram->setAttributeArray(posLoc, m_data.constData());

    ctx->glDrawArrays(m_primitiveType, 0, m_data.size());

    shaderProgram->disableAttributeArray(posLoc);
    shaderProgram->release();
}
