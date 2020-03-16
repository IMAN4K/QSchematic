#pragma once

#include <QObject>
#include <QList>
#include <QMap>
#include <memory>
#include "../settings.h"

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
    void add_net(const std::shared_ptr<net> wireNet);
    QList<std::shared_ptr<net>> nets() const;
    QList<std::shared_ptr<wire>> wires() const;
    void generate_junctions();
    void connect_wire(wire* wire, wire_system::wire* rawWire);
    void remove_net(std::shared_ptr<net> net);
    void clear();
    bool remove_wire(const std::shared_ptr<wire> wire);
    QVector<std::shared_ptr<wire>> wires_connected_to(const std::shared_ptr<wire>& wire) const;
    void disconnect_wire(const std::shared_ptr<wire_system::wire>& wire, wire_system::wire* otherWire);
    bool add_wire(const std::shared_ptr<wire>& wire);
    void attach_wire_to_connector(wire* wire, int index, const std::shared_ptr<Connector>& connector);
    void attach_wire_to_connector(wire* wire, const std::shared_ptr<Connector>& connector);
    wire* attached_wire(const std::shared_ptr<Connector>& connector);
    int attached_point(const std::shared_ptr<Connector>& connector);
    void detach_wire(const std::shared_ptr<Connector>& connector);
    std::shared_ptr<wire> wire_with_extremity_at(const QPointF& point);
    void point_inserted(const wire* wire, int index);
    bool point_is_attached(wire_system::wire* wire, int index);
    void set_settings(const Settings& settings);
    Settings settings() const;
    void point_removed(const wire* wire, int index);
    void point_moved_by_user(wire& rawWire, int index);
    void set_net_factory(std::function<std::shared_ptr<net>()> func);

signals:
    void wire_point_moved(wire& wire, int index);

private:
    static bool merge_nets(std::shared_ptr<wire_system::net>& net, std::shared_ptr<wire_system::net>& otherNet);

    void net_highlight_changed(bool highlighted);
    QList<std::shared_ptr<net>> nets(const std::shared_ptr<net> wireNet) const;
    void detach_wire(const std::shared_ptr<wire>& wire);
    void detach_wire_from_all(const wire* wire);
    QList<std::shared_ptr<Connector>> connectors_attached_to_wire(const wire* wire);
    void connector_moved(const std::shared_ptr<Connector>& connector);
    std::shared_ptr<net> create_net();

    QList<std::shared_ptr<net>> m_nets;
    Settings m_settings;
    QMap<std::shared_ptr<Connector>, QPair<wire*, int>> m_connections;
    std::optional<std::function<std::shared_ptr<net>()>> m_net_factory;
};

}
