#ifndef SONARDRIVER_H
#define SONARDRIVER_H

#include <QObject>
#include <ProtoBinnary.h>
#include <IDBinnary.h>
#include <QHash>
#include <QVector>
#include "QTimer"

using namespace Parsers;

class DevDriver : public QObject
{
    Q_OBJECT
public:
    explicit DevDriver(QObject *parent = nullptr);

    typedef enum {
        DatasetOff = 0,
        DatasetCh1 = 1,
        DatasetCh2 = 2,
        DatasetRequest = 255
    } DatasetChannel;

    enum UpgradeStatus {
        failUpgrade = -1,
        successUpgrade = 101
    };

    int distMax();
    void setDistMax(int dist);

    int distDeadZone();
    void setDistDeadZone(int dead_zone);

    int distConfidence();
    void setConfidence(int confidence);

    int chartSamples();
    void setChartSamples(int samples);

    int chartResolution();
    void setChartResolution(int resol);

    int chartOffset();
    void setChartOffset(int offset);

    int dspSmoothFactor();
    void setDspSmoothFactor(int dsp_smooth);

    int datasetTimestamp();
    void setDatasetTimestamp(int ch_param);
    int datasetDist();
    void setDatasetDist(int ch_param);
    int datasetChart();
    void setDatasetChart(int ch_param);
    int datasetTemp();
    void setDatasetTemp(int ch_param);
    int datasetEuler();
    void setDatasetEuler(int ch_param);
    int datasetSDDBT();
    void setDatasetSDDBT(int ch_param);
    int datasetSDDBT_P2();
    void setDatasetSDDBT_P2(int ch_param);

    int ch1Period();
    void setCh1Period(int period);

    int ch2Period();
    void setCh2Period(int period);

    void sendUpdateFW(QByteArray update_data);
    bool isUpdatingFw() { return m_bootloader; }
    int upgradeFWStatus() {return m_upgrade_status; }

    void sendFactoryFW(QByteArray update_data);

    int transFreq();
    void setTransFreq(int freq);

    int transPulse();
    void setTransPulse(int pulse);

    int transBoost();
    void setTransBoost(int boost);

    int soundSpeed();
    void setSoundSpeed(int speed);

    float yaw();
    float pitch();
    float roll();

    void setBusAddress(int addr);
    int getBusAddress();

    void setDevAddress(int addr);
    int getDevAddress();

    void setBaudrate(int baudrate);
    int getBaudrate();

    void setDevDefAddress(int addr);
    int getDevDefAddress();

    int dopplerVeloX();
    int dopplerVeloY();
    int dopplerVeloZ();
    int dopplerDist();


    QString devName() { return m_devName; }
    uint32_t devSerialNumber();
    QString devPN();

    QString fwVersion() { return m_fwVer; }

    BoardVersion boardVersion() {
        return idVersion->boardVersion();
    }

    bool isSonar() {
        BoardVersion ver = boardVersion();
        return ver == BoardBase || ver == BoardNBase || ver == BoardEnhanced || ver == BoardChirp || ver == BoardNEnhanced || ver == BoardSideEnhanced || ver == BoardEcho20;
    }

    bool isRecorder() {
        BoardVersion ver = boardVersion();
        return ver == BoardRecorderMini;
    }

    bool isDoppler() {
        BoardVersion ver = boardVersion();
        return ver == BoardDVL;
    }

    bool isChartSupport() { return m_state.duplex && isSonar(); }
    bool isDistSupport() { return m_state.duplex && isSonar(); }
    bool isDSPSupport() { return m_state.duplex && isSonar(); }
    bool isTransducerSupport() { return m_state.duplex && isSonar(); }
    bool isDatasetSupport() { return m_state.duplex && isSonar(); }
    bool isSoundSpeedSupport() { return m_state.duplex && isSonar(); }
    bool isAddressSupport() { return m_state.duplex; }
    bool isUpgradeSupport() { return m_state.duplex; }

signals:
    void binFrameOut(ProtoBinOut &proto_out);

    void chartComplete(int16_t channel, QVector<int16_t> data, int resolution, int offset);
    void iqComplete(QByteArray data, uint8_t type);
    void attitudeComplete(float yaw, float pitch, float roll);
    void distComplete(int dist);
    void positionComplete(uint32_t date, uint32_t time, double lat, double lon);
    void chartSetupChanged();
    void dspSetupChanged();
    void distSetupChanged();
    void datasetChanged();
    void transChanged();
    void soundChanged();
    void UARTChanged();
    void upgradeProgressChanged(int progress_status);
    void upgradeChanged();
    void deviceVersionChanged();
    void deviceIDChanged(QByteArray uid);
    void onReboot();

    void dopplerVeloComplete();
    void dopplerBeamComplete(IDBinDVL::BeamSolution *beams, uint16_t cnt);
    void dvlSolutionComplete(IDBinDVL::DVLSolution dvlSolution);

public slots:
    void protoComplete(FrameParser &proto);
    void startConnection(bool duplex);
    void stopConnection();
    void restartState();

    void requestDist();
    void requestChart();

    void requestStreamList();
    void requestStream(int stream_id);

    void setConsoleOut(bool is_console);

    void flashSettings();
    void resetSettings();
    void reboot();
    void process();

protected:
    typedef void (DevDriver::* ParseCallback)(Type type, Version ver, Resp resp);

    FrameParser* m_proto;

    IDBinTimestamp* idTimestamp = NULL;
    IDBinDist* idDist = NULL;
    IDBinChart* idChart = NULL;
    IDBinAttitude* idAtt = NULL;
    IDBinTemp* idTemp = NULL;

    IDBinDataset* idDataset = NULL;
    IDBinDistSetup* idDistSetup = NULL;
    IDBinChartSetup* idChartSetup = NULL;
    IDBinDSPSetup* idDSPSetup = NULL;
    IDBinTransc* idTransc = NULL;
    IDBinSoundSpeed* idSoundSpeed = NULL;
    IDBinUART* idUART = NULL;

    IDBinVersion* idVersion = NULL;
    IDBinMark* idMark = NULL;
    IDBinFlash* idFlash = NULL;
    IDBinBoot* idBoot = NULL;
    IDBinUpdate* idUpdate = NULL;

    IDBinNav* idNav = NULL;
    IDBinDVL* idDVL = NULL;

    QHash<ID, IDBin*> hashIDParsing;
    QHash<ID, ParseCallback> hashIDCallback;
    QHash<ID, IDBin*> hashIDSetup;

    typedef enum {
        ConfNone = 0,
        ConfRequest,
        ConfRx
    } ConfStatus;

    typedef enum {
        UptimeNone,
        UptimeRequest,
        UptimeFix
    } UptimeStatus;

    struct {
        bool connect = false;
        bool duplex = false;
        bool heartbeat = false;
        bool mark = false;

        ConfStatus conf = ConfNone;
        UptimeStatus uptime = UptimeNone;

    } m_state;

    uint8_t _lastAddres = 0;

    QTimer m_processTimer;

    bool m_bootloader = false;
    bool m_bootloaderLagacyMode = true;
    int m_upgrade_status = 0;
    int64_t _lastUpgradeAnswerTime = 0;
    int64_t _timeoutUpgradeAnswerTime = 0;
    bool m_isConsole = false;

    int m_busAddress = 0;
    int m_devAddress = 0;
    int m_devDefAddress = 0;

    QString m_devName = "...";
    QString m_fwVer = "";

    void regID(IDBin* id_bin, ParseCallback method, bool is_setup = false);
    void requestSetup();

    void fwUpgradeProcess();

protected slots:
    void receivedTimestamp(Type type, Version ver, Resp resp);
    void receivedDist(Type type, Version ver, Resp resp);
    void receivedChart(Type type, Version ver, Resp resp);
    void receivedAtt(Type type, Version ver, Resp resp);
    void receivedTemp(Type type, Version ver, Resp resp);

    void receivedDataset(Type type, Version ver, Resp resp);
    void receivedDistSetup(Type type, Version ver, Resp resp);
    void receivedChartSetup(Type type, Version ver, Resp resp);
    void receivedDSPSetup(Type type, Version ver, Resp resp);
    void receivedTransc(Type type, Version ver, Resp resp);
    void receivedSoundSpeed(Type type, Version ver, Resp resp);
    void receivedUART(Type type, Version ver, Resp resp);

    void receivedVersion(Type type, Version ver, Resp resp);
    void receivedMark(Type type, Version ver, Resp resp);
    void receivedFlash(Type type, Version ver, Resp resp);
    void receivedBoot(Type type, Version ver, Resp resp);
    void receivedUpdate(Type type, Version ver, Resp resp);

    void receivedNav(Type type, Version ver, Resp resp);
    void receivedDVL(Type type, Version ver, Resp resp);

};

#endif // SONARDRIVER_H
