#pragma once

#include <memory>

#include <QObject>
#include <QTimer>
#include <QList>
#include <QUuid>
#include <QSerialPortInfo>

#include "Link.h"


class LinkManager : public QObject
{
    Q_OBJECT
public:
    /*methods*/
    explicit LinkManager(QObject *parent = nullptr);
    ~LinkManager();

private:
    /*methods*/
    QList<QSerialPortInfo> getCurrentSerialList() const;
    Link* createSerialPort(const QSerialPortInfo& serialInfo) const;
    void addNewLinks(const QList<QSerialPortInfo> &currSerialList);
    void deleteMissingLinks(const QList<QSerialPortInfo> &currSerialList);
    void openAutoConnections();
    void update();
    Link *getLinkPtr(QUuid uuid);
    void doEmitAppendModifyModel(Link* linkPtr);
    void exportPinnedLinksToXML();
    Link* createNewLink() const;
    void printLinkDebugInfo(Link* link) const;

    /*data*/
    QList<Link*> list_;
    std::unique_ptr<QTimer> timer_;
    static const int timerInterval_ = 500; // msecs
    QMutex mutex_;

signals:
    void appendModifyModel(QUuid uuid, bool connectionStatus, ControlType controlType, QString portName, int baudrate, bool parity,
                       LinkType linkType, QString address, int sourcePort, int destinationPort, bool isPinned, bool isHided, bool isNotAvailable);
    void deleteModel(QUuid uuid);

    void frameReady(QUuid uuid, Link* link, FrameParser frame); //
    void linkClosed(QUuid uuid, Link* link);
    void linkOpened(QUuid uuid, Link* link);
    void linkDeleted(QUuid uuid, Link* link);

public slots:
    void onLinkConnectionStatusChanged(QUuid uuid);

    void createTimer();
    void stopTimer();
    void onExpiredTimer();

    void openAsSerial(QUuid uuid);

    void openAsUdp(QUuid uuid, QString address, int sourcePort, int destinationPort);
    void openAsTcp(QUuid uuid, QString address, int sourcePort, int destinationPort);

    void closeLink(QUuid uuid);
    void closeFLink(QUuid uuid);
    void deleteLink(QUuid uuid);


    void updateBaudrate(QUuid uuid, int baudrate);
    void updateAddress(QUuid uuid, const QString& address);
    void updateSourcePort(QUuid uuid,int sourcePort);
    void updateDestinationPort(QUuid uuid,int destinationPort);
    void updatePinnedState(QUuid uuid, bool state);
    void updateControlType(QUuid uuid, ControlType controlType);

    void frameInput(Link* link, FrameParser frame); //

    void createAsUdp(QString address, int sourcePort, int destinationPort);
    void createAsTcp(QString address, int sourcePort, int destinationPort);

    void importPinnedLinksFromXML();

    void openFLinks();
};
