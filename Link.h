#ifndef LINK_H
#define LINK_H

#include <ProtoBinnary.h>

#include <QObject>
#include <QIODevice>
#include <QByteArray>
#include <QQueue>
#include <QMutex>
#include <QThread>
#include <QUuid>


using namespace Parsers;

typedef enum {
    LinkNone,
    LinkSerial,
    LinkIPUDP,
    LinkIPTCP,
} LinkType;

typedef enum {
    kManual = 0,
    kAuto,
    kAutoOnce
} ControlType;

class Link : public QObject {
    Q_OBJECT
public:
    Link();

    void createAsSerial(const QString &portName, int baudrate, bool parity);
    void openAsSerial();

    void openAsUDP(const QString &address, const int port_in,  const int port_out);

    bool isOpen();
    void close();
    bool parse();

    FrameParser* frameParser() { return &_frame; }
    QIODevice* device() { return _dev; }


    /*multi link*/
    QUuid getUuid() const;
    bool getConnectionStatus() const;
    ControlType getControlType() const;
    /*Serial*/
    QString getPortName() const;
    int getBaudrate() const;
    bool getParity() const;
    /*UDP/TCP*/
    LinkType getLinkType() const;
    QString getAddress() const;
    int getSourcePort() const;
    int getDestinationPort() const;
    /*other*/
    bool isPinned() const;
    bool isHided() const;
    bool isNotAvailable() const;
    /**/


public slots:
    bool writeFrame(FrameParser* frame);
    bool write(QByteArray data);

private slots:
    void toContext(const QByteArray data);
    void readyRead();
    void aboutToClose();

private:
    QMutex _mutex;
    FrameParser _frame;

    QIODevice* _dev = nullptr;

    QByteArray _context;
    QByteArray _buffer;

    LinkType _type = LinkNone;


    /*multi link*/
    QUuid uuid_;

    ControlType controlType_;
    /*Serial*/
    QString portName_;
    int baudrate_;
    bool parity_;
    /*UDP/TCP*/
    LinkType linkType_;
    QString address_;
    int srcPort_;
    int dstPort_;
    /*others*/
    bool isPinned_;
    bool isHided_;
    bool isNotAvailable_;
    /**/

    void setType(LinkType type) { _type = type; }
    void setDev(QIODevice* dev);
    void deleteDev();

signals:;
    void readyParse(Link* link);
    void changeState();


    void connectionStatusChanged(Link* link, bool state);
    void deleted();
    void frameReady();
    void dataReady();

};


#endif // LINK_H
