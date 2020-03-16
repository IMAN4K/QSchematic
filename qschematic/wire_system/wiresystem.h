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

class net;
class wire;

class wire_manager : public QObject
{
    Q_OBJECT
public:
    wire_manager();
    void addWireNet(const std::shared_ptr<net> wireNet);
    QList<std::shared_ptr<net>> nets() const;
    QList<std::shared_ptr<wire>> wires() const;
    void generateJunctions();
    void connectWire(wire* wire, wire_system::wire* rawWire);
    void removeWireNet(std::shared_ptr<net> net);
    void clear();
    bool removeWire(const std::shared_ptr<wire> wire);
    QVector<std::shared_ptr<wire>> wiresConnectedTo(const std::shared_ptr<wire>& wire) const;
    void disconnectWire(const std::shared_ptr<wire_system::wire>& wire, wire_system::wire* otherWire);
    bool addWire(const std::shared_ptr<wire>& wire);
    void attach_wire_to_connector(wire* wire, int index, const std::shared_ptr<Connector>& connector);
    void attach_wire_to_connector(wire* wire, const std::shared_ptr<Connector>& connector);
    wire* attached_wire(const std::shared_ptr<Connector>& connector);
    int attachedWirepoint(const std::shared_ptr<Connector>& connector);
    void detachWire(const std::shared_ptr<Connector>& connector);
    std::shared_ptr<wire> wireWithExtremityAt(const QPointF& point);
    void point_inserted(const wire* wire, int index);
    bool wire_point_is_attached(wire_system::wire* wire, int index);
    void set_settings(const Settings& settings);
    Settings settings() const;
    void point_removed(const wire* wire, int index);
    void wirePointMovedByUser(wire& rawWire, int index);

signals:
    void wirePointMoved(wire& wire, int index);

private:
    static bool mergeNets(std::shared_ptr<wire_system::net>& net, std::shared_ptr<wire_system::net>& otherNet);

    QList<std::shared_ptr<net>> _nets;
    void wireNetHighlightChanged(bool highlighted);
    QList<std::shared_ptr<net>> nets(const std::shared_ptr<net> wireNet) const;
    void detachWire(const std::shared_ptr<wire>& wire);
    void detach_wire_from_all(const wire* wire);
    QList<std::shared_ptr<Connector>> connectors_attached_to_wire(const wire* wire);
    void connectorMoved(const std::shared_ptr<Connector>& connector);

    Settings _settings;
    QMap<std::shared_ptr<Connector>, QPair<wire*, int>> _connections;
};

}
