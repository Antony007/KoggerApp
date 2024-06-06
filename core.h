#pragma once

#include <QObject>
#include <QUrl>
#include <QQmlApplicationEngine>
#include <QStandardItemModel>
#include <QQmlContext>
#include <QThread>

#ifdef FLASHER
#include "flasher.h"
#endif
#include "waterfall.h"
#include "logger.h"
#include "console.h"
#include "ConverterXTF.h"
#include <graphicsscene3dview.h>
#include <bottomtrackcontrolmenucontroller.h>
#include <surfacecontrolmenucontroller.h>
#include <pointgroupcontrolmenucontroller.h>
#include <polygongroupcontrolmenucontroller.h>
#include <mpcfiltercontrolmenucontroller.h>
#include <npdfiltercontrolmenucontroller.h>
#include <scene3dtoolbarcontroller.h>
#include <scene3dcontrolmenucontroller.h>
#include <DeviceManagerWrapper.h>
#include <LinkManagerWrapper.h>
#include <FileReader.h>


class Core : public QObject
{
    Q_OBJECT

public:
    Core();
    ~Core();

    Q_PROPERTY(bool isFactoryMode READ isFactoryMode CONSTANT)
    Q_PROPERTY(ConsoleListModel* consoleList READ consoleList CONSTANT)
    Q_PROPERTY(bool logging WRITE setLogging)
    Q_PROPERTY(int fileReaderProgress READ getFileReaderProgress NOTIFY fileReaderProgressChanged)

    void setEngine(QQmlApplicationEngine *engine);
    Console* getConsolePtr();
    Dataset* getDatasetPtr();
    DeviceManagerWrapper* getDeviceManagerWrapperPtr() const;
    LinkManagerWrapper* getLinkManagerWrapperPtr() const;
    void stopLinkManagerTimer() const;
    void consoleInfo(QString msg);
    void consoleWarning(QString msg);
    void consoleProto(FrameParser& parser, bool isIn = true);
#ifdef FLASHER
    void getFlasherPtr() const;
#endif

public slots:
    bool openLogFile(const QString& name, bool isAppend = false);
    bool closeLogFile();
    bool openXTF(QByteArray data);    
    bool openCSV(QString name, int separatorType, int row = -1, int colTime = -1, bool isUtcTime = true, int colLat = -1, int colLon = -1, int colAltitude = -1, int colNorth = -1, int colEast = -1, int colUp = -1);
    bool openProxy(const QString& address, const int port, bool isTcp);
    bool closeProxy();
    bool upgradeFW(const QString& name, QObject* dev);
    void upgradeChanged(int progressStatus);
    void setLogging(bool isLogging);
    bool getIsLogging();
    bool exportComplexToCSV(QString filePath);
    bool exportUSBLToCSV(QString filePath);
    bool exportPlotAsCVS(QString filePath, int channel, float decimation = 0);
    bool exportPlotAsXTF(QString filePath);
    void setPlotStartLevel(int level);
    void setPlotStopLevel(int level);
    void setTimelinePosition(double position);
    void UILoad(QObject* object, const QUrl& url);
    // fileReader
    void startFileReader(const QString& filePath);
    void stopFileReader();
    void receiveFileReaderProgress(int progress);
    int getFileReaderProgress();
#ifdef FLASHER
    bool simpleFlash(const QString &name);
    bool factoryFlash(const QString &name, int sn, QString pn, QObject* dev);
#endif

signals:
    void connectionChanged(bool duplex = false);
    // fileReader
    void sendStopFileReader();
    void fileReaderProgressChanged();

private slots:
#ifdef FLASHER
    void updateDeviceID(QByteArray uid);
    void flasherConnectionChanged(Flasher::BootState connection_status);
    bool reconnectForFlash();
#endif

private:
    /*methods*/
    ConsoleListModel* consoleList();
    void createControllers();
    void createDeviceManagerConnections();
    void createLinkManagerConnections();
    void removeLinkManagerConnections();
    bool isOpenedFile() const;
    bool isFactoryMode() const;

    /*data*/
    std::shared_ptr<BottomTrackControlMenuController> bottomTrackControlMenuController_;
    std::shared_ptr<MpcFilterControlMenuController> mpcFilterControlMenuController_;
    std::shared_ptr<NpdFilterControlMenuController> npdFilterControlMenuController_;
    std::shared_ptr<SurfaceControlMenuController> surfaceControlMenuController_;
    std::shared_ptr<PointGroupControlMenuController> pointGroupControlMenuController_;
    std::shared_ptr<PolygonGroupControlMenuController> polygonGroupControlMenuController_;
    std::shared_ptr<Scene3DControlMenuController> scene3dControlMenuController_;
    std::shared_ptr<Scene3dToolBarController> scene3dToolBarController_;
    std::unique_ptr<DeviceManagerWrapper> deviceManagerWrapperPtr_;
    std::unique_ptr<LinkManagerWrapper> linkManagerWrapperPtr_;
    QQmlApplicationEngine* qmlAppEnginePtr_ = nullptr;
    Dataset* datasetPtr_;
    Console* consolePtr_;
    GraphicsScene3dView* scene3dViewPtr_ = nullptr;
    ConverterXTF converterXtf_;
    Logger logger_;
    QList<qPlot2D*> plot2dList_;
    QList<QMetaObject::Connection> linkManagerWrapperConnections_;
    QString openedfilePath_;
    bool isLogging_;
    // fileReader
    std::unique_ptr<FileReader> fileReader_;
    std::unique_ptr<QThread> fileReaderThread_;
    QList<QMetaObject::Connection> fileReaderConnections_;
    int fileReaderProgress_ = 0;
#ifdef FLASHER
    Flasher flasher;
    QByteArray boot_data;
    QByteArray fw_data;
    QTcpSocket *_socket = new QTcpSocket();
    bool getFW(void* uid);
    QString _pn;
    enum  {
        FactoryIdle,
        FactoryTest,
        FactoryProduct,
        FactorySimple
    } _factoryState = FactoryIdle;
    QByteArray _flashUID;
#endif
};
