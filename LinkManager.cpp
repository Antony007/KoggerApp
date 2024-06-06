#include "LinkManager.h"

#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>


LinkManager::LinkManager(QObject *parent) :
    QObject(parent)
{
    qRegisterMetaType<ControlType>("ControlType");
    qRegisterMetaType<LinkType>("LinkType");
    qRegisterMetaType<FrameParser>("FrameParser");
}

LinkManager::~LinkManager()
{

}

QList<QSerialPortInfo> LinkManager::getCurrentSerialList() const
{
    return QSerialPortInfo::availablePorts();
}

Link* LinkManager::createSerialPort(const QSerialPortInfo &serialInfo) const
{
    Link* newLinkPtr = nullptr;

    if (serialInfo.isNull())
        return newLinkPtr;

    newLinkPtr = createNewLink();

    newLinkPtr->createAsSerial(serialInfo.portName(), 921600, false);   

    return newLinkPtr;
}

void LinkManager::addNewLinks(const QList<QSerialPortInfo> &currSerialList)
{
    for (const auto& itmI : currSerialList) {
        bool isBeen{ false };

        for (auto& itmJ : list_) {
            if (itmJ->getLinkType() != LinkType::LinkSerial)
                continue;

            if (itmI.portName() == itmJ->getPortName()) {
                isBeen = true;
                break;
            }
        }

        if (!isBeen) {
            auto link = createSerialPort(itmI);
            qDebug() << "link created: " << link->getUuid();

            // list
            list_.append(link);
            // model
            doEmitAppendModifyModel(link);
        }
    }
}

void LinkManager::deleteMissingLinks(const QList<QSerialPortInfo> &currSerialList)
{
    for (int i = 0; i < list_.size(); ++i) {
        Link* link = list_.at(i);

        if (link->getLinkType() != LinkType::LinkSerial)
            continue;

        bool isBeen{ false };
        for (const auto& itm : currSerialList) {
            if (itm.portName() == link->getPortName()) {
                isBeen = true;
                break;
            }
        }

        if (link->getIsPinned()) {
            if (!isBeen && !link->getIsNotAvailable()) {
                if (link->isOpen())
                    link->close();
                link->setIsNotAvailable(true);
                doEmitAppendModifyModel(link);
            }
            else if (isBeen && link->getIsNotAvailable()) {
                link->setIsNotAvailable(false);
                doEmitAppendModifyModel(link);
            }
        }
        else if (!isBeen) {
            deleteLink(link->getUuid());
        }
    }
}

void LinkManager::openAutoConnections()
{
    for (int i = 0; i < list_.size(); ++i) {
        Link* link = list_.at(i);

        if (!link->getConnectionStatus()) {
            if (link->getControlType() == ControlType::kAuto &&
                !link->getIsNotAvailable() &&
                !link->getIsForceStopped()) {
                switch (link->getLinkType()) {
                case LinkType::LinkNone:    { break; }
                case LinkType::LinkSerial:  { link->openAsSerial(); break; }
                case LinkType::LinkIPUDP:   { link->openAsUdp(); break; }
                case LinkType::LinkIPTCP:   { link->openAsTcp(); break; }
                default:                    { break; }
                }
            }
        }
    }
}

void LinkManager::update()
{
    auto currSerialList{ getCurrentSerialList() };

    addNewLinks(currSerialList);
    deleteMissingLinks(currSerialList);

    openAutoConnections();
}

Link *LinkManager::getLinkPtr(QUuid uuid)
{
    Link* retVal{ nullptr };

    for (auto& itm : list_) {
        if (itm->getUuid() == uuid) {
            retVal = itm;
            break;
        }
    }

    return retVal;
}

void LinkManager::doEmitAppendModifyModel(Link* linkPtr)
{
    emit appendModifyModel(linkPtr->getUuid(),
                           linkPtr->getConnectionStatus(),
                           linkPtr->getControlType(),
                           linkPtr->getPortName(),
                           linkPtr->getBaudrate(),
                           linkPtr->getParity(),
                           linkPtr->getLinkType(),
                           linkPtr->getAddress(),
                           linkPtr->getSourcePort(),
                           linkPtr->getDestinationPort(),
                           linkPtr->getIsPinned(),
                           linkPtr->getIsHided(),
                           linkPtr->getIsNotAvailable());
}

void LinkManager::exportPinnedLinksToXML()
{
    TimerController(timer_.get());

    qDebug() << "LinkManager::exportPinnedLinksToXML";
    QString filePath{"pinned_links.xml"};
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("pinned_links");

    for (auto& itm : list_) {
        if (itm->getIsPinned()) {
            xmlWriter.writeStartElement("link");
            xmlWriter.writeTextElement("uuid", itm->getUuid().toString());
            xmlWriter.writeTextElement("control_type", QString::number(static_cast<int>(itm->getControlType())));
            xmlWriter.writeTextElement("port_name", itm->getPortName());
            xmlWriter.writeTextElement("baudrate", QString::number(itm->getBaudrate()));
            xmlWriter.writeTextElement("parity", QVariant(static_cast<bool>(itm->getParity())).toString());
            xmlWriter.writeTextElement("link_type", QString::number(static_cast<int>(itm->getLinkType())));
            xmlWriter.writeTextElement("address", itm->getAddress());
            xmlWriter.writeTextElement("source_port", QString::number(itm->getSourcePort()));
            xmlWriter.writeTextElement("destination_port", QString::number(itm->getDestinationPort()));
            xmlWriter.writeTextElement("is_pinned", QVariant(static_cast<bool>(itm->getIsPinned())).toString());
            xmlWriter.writeTextElement("is_hided", QVariant(static_cast<bool>(itm->getIsHided())).toString());
            xmlWriter.writeTextElement("is_not_available", QVariant(static_cast<bool>(itm->getIsNotAvailable())).toString());
            xmlWriter.writeTextElement("connection_status", QVariant(static_cast<bool>(itm->getConnectionStatus())).toString());
            xmlWriter.writeEndElement();
        }
    }

    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
    file.close();
}

Link *LinkManager::createNewLink() const
{
    Link* retVal = new Link();

    QObject::connect(retVal, &Link::connectionStatusChanged, this, &LinkManager::onLinkConnectionStatusChanged);
    QObject::connect(retVal, &Link::frameReady, this, &LinkManager::frameReady);
    QObject::connect(retVal, &Link::closed, this, &LinkManager::linkClosed);
    QObject::connect(retVal, &Link::opened, this, &LinkManager::linkOpened);

    // connect(this, &LinkManagerWorker::frameInput, newLink, &Link::writeFrame);

    return retVal;
}

void LinkManager::printLinkDebugInfo(Link* link) const
{
    if (!link)
        qDebug() << "\tlink is nullptr";
    else {
        qDebug() << QString("uuid: %1; controlType: %2; portName: %3; baudrate: %4; parity: %5; linkType: %6; address: %7; sourcePort: %8; destinationPort: %9; isPinned: %10; isHided: %11; isNotAvailable: %12; connectionStatus: %13")
                        .arg(link->getUuid().toString()).arg(link->getControlType()).arg(link->getPortName()).arg(link->getBaudrate()).arg(link->getParity()).arg(link->getLinkType()).arg(link->getAddress()).arg(link->getSourcePort())
                        .arg(link->getDestinationPort()).arg(link->getIsPinned()).arg(link->getIsHided()).arg(link->getIsNotAvailable()).arg(link->getConnectionStatus());
    }
}

void LinkManager::importPinnedLinksFromXML()
{
    qDebug() << "LinkManager::importPinnedLinksFromXML";
    QString filePath{"pinned_links.xml"};
    QFile file(filePath);

    auto createAndStartTimerFunc = [this]() {
        qDebug() << "LinkManager::importPinnedLinksFromXML: start update timer";
        createTimer();
        timer_->start();
    };

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        createAndStartTimerFunc();
        return;
    }

    QXmlStreamReader xmlReader(&file);

    while (!xmlReader.atEnd() && !xmlReader.hasError()) {
        const QXmlStreamReader::TokenType token = xmlReader.readNext();

        if (token == QXmlStreamReader::StartElement) {
            if (xmlReader.name() == "link") {
                qDebug() << "import link: ";

                Link* link = createNewLink();

                while (!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "link")) {
                    if (xmlReader.tokenType() == QXmlStreamReader::StartElement) {
                        if (xmlReader.name().toString() == "uuid") {
                            link->setUuid(QUuid(xmlReader.readElementText()));
                            qDebug() << "\tuuid: " << link->getUuid().toString();
                        }
                        else if (xmlReader.name().toString() == "connection_status") {
                            // TODO
                            link->setConnectionStatus(xmlReader.readElementText().trimmed().toUpper() == "TRUE" ? true : false);
                            qDebug() << "\tconnection_status: " << link->getConnectionStatus();
                        }
                        else if (xmlReader.name().toString() == "control_type") {
                            link->setControlType(static_cast<ControlType>(xmlReader.readElementText().toInt()));
                            qDebug() << "\tcontrol_type: " << link->getControlType();
                        }
                        else if (xmlReader.name().toString() == "port_name") {
                            link->setPortName(xmlReader.readElementText());
                            qDebug() << "\tport_name: " << link->getPortName();
                        }
                        else if (xmlReader.name().toString() == "baudrate") {
                            link->setBaudrate(xmlReader.readElementText().toInt());
                            qDebug() << "\tbaudrate: " << link->getBaudrate();
                        }
                        else if (xmlReader.name().toString() == "parity") {
                            link->setParity(xmlReader.readElementText().trimmed().toUpper() == "TRUE" ? true : false);
                            qDebug() << "\tparity: " << link->getParity();
                        }
                        else if (xmlReader.name().toString() == "link_type") {
                            link->setLinkType(static_cast<LinkType>(xmlReader.readElementText().toInt()));
                            qDebug() << "\tlink_type: " << link->getLinkType();
                        }
                        else if (xmlReader.name().toString() == "address") {
                            link->setAddress(xmlReader.readElementText());
                            qDebug() << "\taddress: " << link->getAddress();
                        }
                        else if (xmlReader.name().toString() == "source_port") {
                            link->setSourcePort(xmlReader.readElementText().toInt());
                            qDebug() << "\tsource_port: " << link->getSourcePort();
                        }
                        else if (xmlReader.name().toString() == "destination_port") {
                            link->setDestinationPort(xmlReader.readElementText().toInt());
                            qDebug() << "\tdestination_port: " << link->getDestinationPort();
                        }
                        else if (xmlReader.name().toString() == "is_pinned") {
                            link->setIsPinned(xmlReader.readElementText().trimmed().toUpper() == "TRUE" ? true : false);
                            qDebug() << "\tis_pinned: " << link->getIsPinned();
                        }
                        else if (xmlReader.name().toString() == "is_hided") {
                            link->setIsHided(xmlReader.readElementText().trimmed().toUpper() == "TRUE" ? true : false);
                            qDebug() << "\tis_hided: " << link->getIsHided();
                        }
                        else if (xmlReader.name().toString() == "is_not_available") {
                            link->setIsNotAvailable(xmlReader.readElementText().trimmed().toUpper() == "TRUE" ? true : false);
                            qDebug() << "\tis_not_available: " << link->getIsNotAvailable();
                        }
                    }
                    xmlReader.readNext();
                }

                list_.append(link);
                qDebug() << "added link from xml:";
                printLinkDebugInfo(link);
                doEmitAppendModifyModel(link);
            }
        }
    }

    if (xmlReader.hasError())
        qDebug() << "XML error:" << xmlReader.errorString();
    file.close();

    createAndStartTimerFunc();
}

void LinkManager::onLinkConnectionStatusChanged(QUuid uuid)
{
    if (const auto linkPtr = getLinkPtr(uuid); linkPtr) {
        doEmitAppendModifyModel(linkPtr);

        if (linkPtr->getIsPinned()) // or to open/close?
            exportPinnedLinksToXML();
    }
}

void LinkManager::createTimer()
{
    if (!timer_) {
        timer_ = std::make_unique<QTimer>(this);
        timer_->setInterval(timerInterval_);
        QObject::connect(timer_.get(), &QTimer::timeout, this, &LinkManager::onExpiredTimer, Qt::QueuedConnection);
    }
}

void LinkManager::stopTimer()
{
    timer_->stop();
}

void LinkManager::onExpiredTimer()
{
    update();
    timer_->start();
}

void LinkManager::openAsSerial(QUuid uuid)
{
    TimerController(timer_.get());

    if (const auto linkPtr = getLinkPtr(uuid); linkPtr) {
        linkPtr->setIsForceStopped(false);
        linkPtr->openAsSerial();
    }
}

void LinkManager::openAsUdp(QUuid uuid, QString address, int sourcePort, int destinationPort)
{
    TimerController(timer_.get());

    if (const auto linkPtr = getLinkPtr(uuid); linkPtr) {
        linkPtr->setIsForceStopped(false);
        linkPtr->updateUdpParameters(address, sourcePort, destinationPort);
        linkPtr->openAsUdp();

        doEmitAppendModifyModel(linkPtr); //
    }
}

void LinkManager::openAsTcp(QUuid uuid, QString address, int sourcePort, int destinationPort)
{
    TimerController(timer_.get());

    if (const auto linkPtr = getLinkPtr(uuid); linkPtr) {
        linkPtr->setIsForceStopped(false);
        linkPtr->updateTcpParameters(address, sourcePort, destinationPort);
        linkPtr->openAsTcp();

        doEmitAppendModifyModel(linkPtr); //
    }
}

void LinkManager::closeLink(QUuid uuid)
{
    TimerController(timer_.get());

    if (const auto linkPtr = getLinkPtr(uuid); linkPtr) {
        if (linkPtr->getControlType() == ControlType::kAuto)
            linkPtr->setIsForceStopped(true);
        linkPtr->close();

        doEmitAppendModifyModel(linkPtr); //
    }
}

void LinkManager::closeFLink(QUuid uuid)
{
    TimerController(timer_.get());

    if (const auto linkPtr = getLinkPtr(uuid); linkPtr) {
        linkPtr->setIsForceStopped(true);
        linkPtr->close();
        doEmitAppendModifyModel(linkPtr); //
    }
}

void LinkManager::deleteLink(QUuid uuid)
{
    TimerController(timer_.get());

    if (const auto linkPtr = getLinkPtr(uuid); linkPtr) {
        emit linkDeleted(linkPtr->getUuid(), linkPtr);

        emit deleteModel(linkPtr->getUuid());
        linkPtr->disconnect();
        this->disconnect(linkPtr);

        if (linkPtr->isOpen())
            linkPtr->close();

        qDebug() << "link deleted: " << linkPtr->getUuid();

        auto linkType = linkPtr->getLinkType();

        list_.removeOne(linkPtr);
        delete linkPtr;

        // manual deleting
        if (linkType == LinkType::LinkIPTCP ||
            linkType == LinkType::LinkIPUDP)
            exportPinnedLinksToXML();
    }
}

void LinkManager::updateBaudrate(QUuid uuid, int baudrate)
{
    TimerController(timer_.get());

    qDebug() << "LinkManager::updateBaudrate: " << baudrate;

    if (const auto linkPtr = getLinkPtr(uuid); linkPtr) {
        linkPtr->setBaudrate(baudrate);

        qDebug() << baudrate;
        doEmitAppendModifyModel(linkPtr); // why?

        if (linkPtr->getIsPinned())
            exportPinnedLinksToXML();
    }
}

void LinkManager::updateAddress(QUuid uuid, const QString &address)
{
    TimerController(timer_.get());

    qDebug() << "LinkManager::updateAddress: " << address;

    if (const auto linkPtr = getLinkPtr(uuid); linkPtr) {
        linkPtr->setAddress(address);

        //doEmitAppendModifyModel(linkPtr); // why not?
        if (linkPtr->getIsPinned())
            exportPinnedLinksToXML();
    }
}

void LinkManager::updateSourcePort(QUuid uuid, int sourcePort)
{
    TimerController(timer_.get());

    qDebug() << "LinkManager::updateSourcePort: " << sourcePort;

    if (const auto linkPtr = getLinkPtr(uuid); linkPtr) {
        linkPtr->setSourcePort(sourcePort);

        //doEmitAppendModifyModel(linkPtr); //
        if (linkPtr->getIsPinned())
            exportPinnedLinksToXML();
    }
}

void LinkManager::updateDestinationPort(QUuid uuid, int destinationPort)
{
    TimerController(timer_.get());

    qDebug() << "LinkManager::updateDestinationPort: " << destinationPort;
    if (const auto linkPtr = getLinkPtr(uuid); linkPtr) {
        linkPtr->setDestinationPort(destinationPort);

        //doEmitAppendModifyModel(linkPtr); //
        if (linkPtr->getIsPinned())
            exportPinnedLinksToXML();
    }
}

void LinkManager::updatePinnedState(QUuid uuid, bool state)
{
    TimerController(timer_.get());

    qDebug() << "LinkManager::updatePinnedState: " << state;

    if (auto linkPtr = getLinkPtr(uuid); linkPtr) {
        linkPtr->setIsPinned(state);

        exportPinnedLinksToXML();
    }
}

void LinkManager::updateControlType(QUuid uuid, ControlType controlType)
{
    TimerController(timer_.get());

    qDebug() << "LinkManager::updateControlType: " << controlType;

    if (auto linkPtr = getLinkPtr(uuid); linkPtr) {
        if (controlType == ControlType::kManual)
            linkPtr->setIsForceStopped(false);

        linkPtr->setControlType(controlType);

        if (linkPtr->getIsPinned())
            exportPinnedLinksToXML();
    }
}

void LinkManager::frameInput(Link *link, FrameParser frame)
{
    // TODO
    Q_UNUSED(link);
    Q_UNUSED(frame);
}

void LinkManager::createAsUdp(QString address, int sourcePort, int destinationPort)
{
    Link* newLinkPtr = createNewLink();

    newLinkPtr->createAsUdp(address, sourcePort, destinationPort);

    list_.append(newLinkPtr);

    doEmitAppendModifyModel(newLinkPtr);
}

void LinkManager::createAsTcp(QString address, int sourcePort, int destinationPort)
{
    Link* newLinkPtr = createNewLink();

    newLinkPtr->createAsTcp(address, sourcePort, destinationPort);

    list_.append(newLinkPtr);

    doEmitAppendModifyModel(newLinkPtr);
}

void LinkManager::openFLinks()
{
    for (auto& itm : list_) {
        if (itm->getIsForceStopped()) {
            itm->setIsForceStopped(false);

            switch (itm->getLinkType()) {
            case LinkType::LinkSerial: {
                itm->openAsSerial();
                break;
            }
            case LinkType::LinkIPTCP : {
                itm->openAsTcp();
                break;
            }
            case LinkType::LinkIPUDP: {
                itm->openAsUdp();
                break;
            }
            default:
                break;
            }
        }
    }
}

void LinkManager::createAndOpenAsUdpProxy(QString address, int sourcePort, int destinationPort)
{

    TimerController(timer_.get());

    Link* newLinkPtr = createNewLink();

    newLinkPtr->createAsUdp(address, sourcePort, destinationPort);

    newLinkPtr->setIsProxy(true);
    //newLinkPtr->setIsHided(true); TODO

    proxyLinkUuid_ = newLinkPtr->getUuid();

    list_.append(newLinkPtr);

    newLinkPtr->openAsUdp();

    doEmitAppendModifyModel(newLinkPtr);

    qDebug() << "LinkManager::createAndOpenAsUdpProxy end: uuid:" << proxyLinkUuid_.toString() << ", address: " << address << ", srcPort: " << sourcePort << ", dstPort: " << destinationPort;
}

void LinkManager::closeUdpProxy()
{
    qDebug() << "LinkManager::closeUdpProxy start: uuid:" << proxyLinkUuid_.toString();

    if (proxyLinkUuid_ == QUuid())
        return;

    deleteLink(proxyLinkUuid_);
    proxyLinkUuid_ = QUuid();

}

LinkManager::TimerController::TimerController(QTimer *timer) : timer_(timer) {
    if (timer_) {
        timer->stop();
    }
}

LinkManager::TimerController::~TimerController() {
    if (timer_) {
        timer_->start();
    }
}
