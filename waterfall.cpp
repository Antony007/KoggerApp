#include "waterfall.h"
#include <QPixmap>
#include <QPainter>
#include <QSGSimpleTextureNode>
#include <QQuickWindow>

#include <QMutex>
#include <core.h>
#include <epochevent.h>

qPlot2D::qPlot2D(QQuickItem* parent)
    : QQuickPaintedItem(parent)
    , m_updateTimer(new QTimer(this))
{
//    setRenderTarget(QQuickPaintedItem::FramebufferObject);
//    connect(m_updateTimer, &QTimer::timeout, this, [&] { update(); });
    m_updateTimer->start(30);
    setFlag(ItemHasContents);
    setAcceptedMouseButtons(Qt::AllButtons);
//    setFillColor(QColor(255, 255, 255));

    _isHorizontal = false;
}

#include <time.h>

void qPlot2D::paint(QPainter *painter) {
    clock_t start = clock();

    if(m_plot != nullptr && painter != nullptr) {
        Plot2D::getImage((int)width(), (int)height(), painter, _isHorizontal);
    }

    clock_t end = clock();
    int cpu_time_used = ((end-start));
    Q_UNUSED(cpu_time_used);

//    qInfo("r time %d", cpu_time_used);
}

//QSGNode *qPlot2D::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
//{
//    QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode *>(oldNode);
//    if (!node) {
//        node = new QSGSimpleTextureNode();

//    }

//    QSGTexture *texture = window()->createTextureFromImage(getImage((int)width(), (int)height()));
////    node->markDirty(QSGNode::DirtyForceUpdate);
//    node->setTexture(texture);
//    node->setRect(boundingRect());
//    return node;
//}

//QSGNode *qPlot2D::updatePaintNode( QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *updatePaintNodeData) {

//    auto node = dynamic_cast<QSGSimpleTextureNode *>(oldNode);

//    if (!node) {
//        node = new QSGSimpleTextureNode();
//    }


//    QQuickWindow* w = window();

//    QSGTexture *texture = window()->createTextureFromImag;

//    node->setOwnsTexture(true);
//    node->setRect(boundingRect());
//    node->markDirty(QSGNode::DirtyForceUpdate);
//    node->setTexture(texture);
//    return node;
//}

void qPlot2D::setPlot(Dataset *dataset) {
    if(dataset == nullptr) { return; }
    m_plot = dataset;
    setDataset(dataset);
    connect(dataset, &Dataset::dataUpdate, this, &qPlot2D::dataUpdate);
//    connect(m_plot, &Dataset::updatedImage, this, [&] { updater(); });
}

void qPlot2D::plotUpdate() {
    static QMutex mutex;
    if(!mutex.tryLock()) {
//        qInfo("HHHHHHHHHHHHHHHHHHHHHHHHHH==================HHHHHHHHHHHHHHHHHHHHHHHHHH");
        return;
    }
    emit timelinePositionChanged();

    update();

    mutex.unlock();
}

bool qPlot2D::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);

    if (event->type() == EpochSelected3d) {
        auto epochEvent = static_cast<EpochEvent*>(event);
        //qDebug() << QString("[Plot 2d]: catched event from 3d view (epoch index is %1)").arg(epochEvent->epochIndex());
        setAimEpochEventState(true);
        setTimelinePositionByEpoch(epochEvent->epochIndex());
    }
    return false;
}

void qPlot2D::sendSyncEvent(int epoch_index) {
    //qDebug() << "qPlot2D::sendSyncEvent: epoch_index: " << epoch_index;
    _cursor.selectEpochIndx = -1;
    auto epochEvent = new EpochEvent(EpochSelected2d, _dataset->fromIndex(epoch_index), epoch_index, _cursor.channel1);
    QCoreApplication::postEvent(this, epochEvent);
}

void qPlot2D::horScrollEvent(int delta) {
    _cursor.selectEpochIndx = -1;

    if(_isHorizontal) {
        scrollPosition(-delta);
    } else {
        scrollPosition(delta);
    }
}

void qPlot2D::verZoomEvent(int delta) {
//    if(m_plot != nullptr) {
//        m_plot->verZoom(delta);
//    }

    zoomDistance(delta);
}

void qPlot2D::verScrollEvent(int delta) {
    scrollDistance(delta);
}

void qPlot2D::plotMouseTool(int mode) {
    setMouseTool((MouseTool)mode);
}


void qPlot2D::doDistProcessing(int preset, int window_size, float vertical_gap, float range_min, float range_max, float gain_slope, float threshold, float offsetx, float offsety, float offsetz) {
    if (_dataset != nullptr) {
        if (auto btpPtr =_dataset->getBottomTrackParamPtr(); btpPtr) {
            btpPtr->preset = static_cast<BottomTrackPreset>(preset);
            btpPtr->gainSlope = gain_slope;
            btpPtr->threshold = threshold;
            btpPtr->windowSize = window_size;
            btpPtr->verticalGap = vertical_gap;
            btpPtr->minDistance = range_min;
            btpPtr->maxDistance = range_max;
            btpPtr->indexFrom = 0;
            btpPtr->indexTo = _dataset->size();
            btpPtr->offset.x = offsetx;
            btpPtr->offset.y = offsety;
            btpPtr->offset.z = offsetz;

            _dataset->bottomTrackProcessing(_cursor.channel1, _cursor.channel2);
        }
    }
    plotUpdate();
}

void qPlot2D::refreshDistParams(int preset, int windowSize, float verticalGap, float rangeMin, float rangeMax, float gainSlope, float threshold, float offsetX, float offsetY, float offsetZ)
{
    auto btPRefreshFunc = [this, preset, windowSize, verticalGap, rangeMin, rangeMax, gainSlope, threshold, offsetX, offsetY, offsetZ]() {
        if (_dataset != nullptr) {
            if (auto btpPtr =_dataset->getBottomTrackParamPtr(); btpPtr) {
                btpPtr->preset = static_cast<BottomTrackPreset>(preset);
                btpPtr->gainSlope = gainSlope;
                btpPtr->threshold = threshold;
                btpPtr->windowSize = windowSize;
                btpPtr->verticalGap = verticalGap;
                btpPtr->minDistance = rangeMin;
                btpPtr->maxDistance = rangeMax;
                btpPtr->indexFrom = 0;
                btpPtr->indexTo = _dataset->size();
                btpPtr->offset.x = offsetX;
                btpPtr->offset.y = offsetY;
                btpPtr->offset.z = offsetZ;
                qDebug() << "qPlot2D::refreshDistParams!";

            }
        }
    };

    if (_dataset == nullptr) {
        pendingBtpLambda_ = btPRefreshFunc;
    } else {
        btPRefreshFunc();
    }
}

void qPlot2D::plotMousePosition(int x, int y) {
    setAimEpochEventState(false);
    if(_isHorizontal) {
        setMousePosition(x, y);
    } else {
        if(x >=0 && y >= 0) {
            setMousePosition(height() - y, x);
        } else {
            setMousePosition(-1, -1);
        }

    }
}

void qPlot2D::timerUpdater() {
    if(m_needUpdate) {
        m_needUpdate = false;
       update();
    }
}

void qPlot2D::updater() {
    m_needUpdate = true;
}
