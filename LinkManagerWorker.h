#pragma once

#include <memory>

#include <QObject>
#include <QTimer>
#include <QList>
#include <QUuid>
#include <QSerialPortInfo>

#include "Link.h"
#include "LinkListModel.h"


class LinkManagerWorker : public QObject
{
    Q_OBJECT
public:
    /*methods*/
    explicit LinkManagerWorker(QList<Link*>* hashPtr, LinkListModel* modelPtr, QObject *parent = nullptr);

private:
    /*methods*/
    QList<QSerialPortInfo> getCurrentSerialList() const;
    Link* createSerialPort(const QSerialPortInfo& serialInfo) const;
    void addNewLinks(const QList<QSerialPortInfo> &currSerialList);
    void deleteMissingLinks(const QList<QSerialPortInfo> &currSerialList);
    void update();

    /*data*/
    QList<Link*>* list_;
    LinkListModel* model_;
    std::unique_ptr<QTimer> timer_;
    static const int timerInterval_ = 200; // msecs
    QMutex mutex_;

signals:
    void dataUpdated();

public slots:
    void onExpiredTimer();
    void stateChanged(Link* linkPtr, bool state);

};

