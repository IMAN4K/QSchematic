#pragma once

#include <QList>
#include <memory>
#include "items/wire.h"
#include "items/wirenet.h"

namespace QSchematic
{
    class Connector;
}

using namespace QSchematic;

namespace wire_system
{

class wire_manager : public QObject
{
    Q_OBJECT
public:
    wire_manager();
    void addWireNet(const std::shared_ptr<WireNet> wireNet);
    QList<std::shared_ptr<WireNet>> nets() const;
    QList<std::shared_ptr<Wire>> wires() const;
    void generateJunctions();
    void connectWire(const std::shared_ptr<Wire>& wire, std::shared_ptr<Wire>& rawWire);
    void removeWireNet(std::shared_ptr<WireNet> net);
    void clear();
    bool removeWire(const std::shared_ptr<Wire> wire);
    QVector<std::shared_ptr<Wire>> wiresConnectedTo(const std::shared_ptr<Wire>& wire) const;
    void disconnectWire(const std::shared_ptr<Wire>& wire, const std::shared_ptr<Wire>& otherWire);
    bool addWire(const std::shared_ptr<Wire>& wire);
    void attachWireToConnector(const std::shared_ptr<Wire>& wire, int index, const std::shared_ptr<Connector>& connector);
    void attachWireToConnector(const std::shared_ptr<Wire>& wire, const std::shared_ptr<Connector>& connector);
    std::shared_ptr<Wire> attachedWire(const std::shared_ptr<Connector>& connector);
    int attachedWirepoint(const std::shared_ptr<Connector>& connector);
    void detachWire(const std::shared_ptr<Connector>& connector);
    std::shared_ptr<Wire> wireWithExtremityAt(const QPointF& point);
    void point_inserted(const wire* wire, int index);

signals:
    void wirePointMoved(Wire& wire, int index);

private:
    static bool mergeNets(std::shared_ptr<WireNet>& net, std::shared_ptr<WireNet>& otherNet);

    QList<std::shared_ptr<WireNet>> _nets;
    void wirePointMovedByUser(Wire& rawWire, int index);
    void wireNetHighlightChanged(bool highlighted);
    QList<std::shared_ptr<WireNet>> nets(const std::shared_ptr<WireNet> wireNet) const;
    QMap<std::shared_ptr<Connector>, QPair<std::shared_ptr<Wire>, int>> _connections;
    void pointRemoved(const std::shared_ptr<Wire>& wire, int index);
    void detachWire(const std::shared_ptr<Wire>& wire);
    void detachWireFromAll(const std::shared_ptr<Wire>& wire);
    QList<std::shared_ptr<Connector>> connectorsAttachedToWire(const std::shared_ptr<Wire>& wire);
    void connectorMoved(const std::shared_ptr<Connector>& connector);
};

}
