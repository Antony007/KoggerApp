#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <connection.h>
#include <console.h>
#include <DevQProperty.h>
#include <QUrl>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <waterfall.h>
#include <DevHub.h>
#include <logger.h>
#include <QThread>
#include <3Plot.h>

#include <sceneobjectslistcontroller.h>
#include <bottomtrackparamscontroller.h>
#include <mpcfilterparamscontroller.h>
#include <npdfilterparamscontroller.h>
#include <pointsetparamscontroller.h>
#include <surfaceparamscontroller.h>
#include <toolbar3dcontroller.h>
#include <scenecontroller.h>
#include <sceneobjectslistmodel.h>
#include <tool3dworker.h>
#include <activeobjectprovider.h>
#include <bottomtrackprovider.h>

//#define FLASHER

#ifdef FLASHER
#include "flasher.h"
#endif

using Scene3DController    = std::shared_ptr <SceneController>;

class Core : public QObject
{
    Q_OBJECT

public:
    explicit Core();

    Q_PROPERTY(ConsoleListModel* consoleList READ consoleList CONSTANT)

    Q_PROPERTY(bool logging WRITE setLogging)

    ConsoleListModel* consoleList() {
        return m_console->listModel();
    }

    Console *console() {
        return m_console;
    }

    PlotCash* plot() {
        return m_plot;
    }

    void consoleInfo(QString msg) {
        console()->put(QtMsgType::QtInfoMsg, msg);
    }

    void consoleWarning(QString msg) {
        console()->put(QtMsgType::QtWarningMsg, msg);
    }

    void consoleProto(FrameParser &parser, bool is_in = true);

    void setEngine(QQmlApplicationEngine *engine);

private:

    //! Метод создания контроллеров
    void createControllers();
    //! Метод создания моделей
    void createModels();
\
    // View controllers
    Scene3DController                             mpSceneController;
    std::shared_ptr <Toolbar3dController>         mpToolbar3dController;
    std::shared_ptr <SceneObjectsListController>  mpSceneObjectsListController;
    std::shared_ptr <BottomTrackParamsController> mpBottomTrackParamsController;
    std::shared_ptr <SurfaceParamsController>     mpSurfaceParamsController;
    std::shared_ptr <MPCFilterParamsController>   mpMPCFilterParamsController;
    std::shared_ptr <NPDFilterParamsController>   mpNPDFilterParamsController;

    // App models
    std::shared_ptr <ActiveObjectProvider>     mpActiveObjectProvider;
    std::shared_ptr <Tool3dWorker>             mpTool3dWorker;
    std::shared_ptr <SceneObjectsListModel>    mpSceneObjectsListModel;
    std::shared_ptr <BottomTrackProvider>      mpBottomTrackProvider;
    std::shared_ptr <PointSetParamsController> mpPointSetParamsController;

public slots:
    QList<QSerialPortInfo> availableSerial();
    QStringList availableSerialName();
    bool openConnectionAsSerial(const QString &name, int baudrate, bool mode);
    bool devsConnection();
    bool openConnectionAsFile(const QString &name);
    bool openConnectionAsIP(const QString &address, const int port, bool is_tcp);
    bool isOpenConnection();
    bool closeConnection();

    bool openProxy(const QString &address, const int port, bool is_tcp);
    bool closeProxy();

    bool connectionBaudrate(int baudrate);

    bool upgradeFW(const QString &name, QObject* dev);
    void upgradeChanged(int progress_status);

    void setLogging(bool is_logging);
    bool isLogging();



    bool exportPlotAsCVS(QString file_path);

#ifdef FLASHER
    bool factoryFlash(const QString &name, int sn, QString pn, QObject* dev);
#endif

    void setPlotStartLevel(int level) {
        m_plot->setStartLevel(level);
    }

    void setPlotStopLevel(int level) {
        m_plot->setStopLevel(level);
    }

    void setTimelinePosition(double position) {
        m_plot->setTimelinePosition(position);
    }

    void setChartVis(bool visible) {
        m_plot->setChartVis(visible);
    }

    void setOscVis(bool visible) {
        m_plot->setOscVis(visible);
    }

    void setDistVis(bool visible) {
        m_plot->setDistVis(visible);
    }

    Device* dev() { return &_devs; }

    void UILoad(QObject *object, const QUrl &url);

signals:
    void connectionChanged(bool duplex = false);

public:
    Console *m_console;
    Connection *m_connection;
    PlotCash* m_plot;
    WaterFall* m_waterFall = NULL;
    FboInSGRenderer* _render = NULL;

    Device _devs;
    Logger _logger;
    QThread connectionThread;
    QQmlApplicationEngine *m_engine = nullptr;

#ifdef FLASHER
    Flasher flasher;
#endif

private slots:
    void closing();
#ifdef FLASHER
    void updateDeviceID(QByteArray uid);
    void flasherConnectionChanged(Flasher::BootState connection_status);
    bool reconnectForFlash();
#endif

protected:
#ifdef FLASHER
    QByteArray boot_data;
    QByteArray fw_data;

    QTcpSocket *_socket = new QTcpSocket();
    bool getFW(void* uid);
    QString _pn;
    enum  {
        FactoryIdle,
        FactoryTest,
        FactoryProduct
    } _factoryState = FactoryIdle;

    QByteArray _flashUID;
#endif

    bool _isLogging;

    int backupBaudrate = 115200;
    void restoreBaudrate();
    void setUpgradeBaudrate();
};

#endif // CORE_H
