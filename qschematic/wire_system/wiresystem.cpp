#include "wiresystem.h"
#include "items/node.h"
#include <QVector2D>

using namespace wire_system;
using namespace QSchematic;

wire_manager::wire_manager()
{
}

void wire_manager::addWireNet(const std::shared_ptr<net> wireNet)
{
    // Sanity check
    if (!wireNet) {
        return;
    }

    wireNet->set_manager(this);

    // Keep track of stuff
    _nets.append(wireNet);
}

/**
 * Returns a list of all the nets
 */
QList<std::shared_ptr<net>> wire_manager::nets() const
{
    return _nets;
}

/**
 * Returns a list of all the wires
 */
QList<std::shared_ptr<wire>> wire_manager::wires() const
{
    QList<std::shared_ptr<wire>> list;

    for (const auto& wireNet : _nets) {
        for (const auto& wire : wireNet->wires()) {
            list.append(wire);
        }
    }

    return list;
}

void wire_manager::generateJunctions()
{
    for (const auto& wire: wires()) {
        for (auto& otherWire: wires()) {
            if (wire == otherWire) {
                continue;
            }
            if (wire->point_is_on_wire(otherWire->points().first().toPointF())) {
                connectWire(wire.get(), otherWire.get());
                otherWire->set_point_is_junction(0, true);
            }
            if (wire->point_is_on_wire(otherWire->points().last().toPointF())) {
                connectWire(wire.get(), otherWire.get());
                otherWire->set_point_is_junction(otherWire->points().count() - 1, true);
            }
        }
    }
}

/**
 * Connect a wire to another wire while taking care of merging the nets.
 * @param wire The wire to connect to
 * @param rawWire The wire to connect
 */
void wire_manager::connectWire(wire* wire, wire_system::wire* rawWire)
{
    if (not wire->connect_wire(rawWire)) {
        return;
    }
    std::shared_ptr<wire_system::net> net = wire->net();
    std::shared_ptr<wire_system::net> otherNet = rawWire->net();
    if (mergeNets(net, otherNet)) {
        removeWireNet(otherNet);
    }
}

/**
 * Merges two wirenets into one
 * \param net The net into which the other one will be merged
 * \param otherNet The net to merge into the other one
 * \return Whether the two nets where merged successfully or not
 */
bool wire_manager::mergeNets(std::shared_ptr<net>& net, std::shared_ptr<wire_system::net>& otherNet)
{
    // Ignore if it's the same net
    if (net == otherNet) {
        return false;
    }
    for (auto& wire: otherNet->wires()) {
        net->addWire(wire);
        otherNet->removeWire(wire);
    }
    return true;
}

void wire_manager::removeWireNet(std::shared_ptr<net> net)
{
    _nets.removeAll(net);
}

void wire_manager::clear()
{
    _nets.clear();
}

bool wire_manager::removeWire(const std::shared_ptr<wire> wire)
{
    // Detach from all connectors
    detach_wire_from_all(wire);

    // Disconnect from connected wires
    for (const auto& otherWire: wiresConnectedTo(wire)) {
        if (otherWire != wire) {
            disconnectWire(otherWire, wire.get());
            // Update the junction on the other wire
            for (int index = 0; index < otherWire->points_count(); index++) {
                const auto point = otherWire->points().at(index);
                if (not point.is_junction()) {
                    continue;
                }
                if (wire->point_is_on_wire(point.toPointF())) {
                    otherWire->set_point_is_junction(index, false);
                }
            }
        }
    }

    // Remove the wire from the list
    QList<std::shared_ptr<net>> netsToDelete;
    for (auto& net : _nets) {
        if (net->contains(wire)) {
            net->removeWire(wire);
        }

        if (net->wires().count() < 1) {
            netsToDelete.append(net);
        }
    }

    // Delete the net if this was the nets last wire
    for (auto& net : netsToDelete) {
        removeWireNet(net);
    }

    return true;
}


/**
 * Generates a list of all the wires connected to a certain wire including the
 * wire itself.
 */
QVector<std::shared_ptr<wire>> wire_manager::wiresConnectedTo(const std::shared_ptr<wire>& wire) const
{
    QVector<std::shared_ptr<wire_system::wire>> connectedWires;

    // Add the wire itself to the list
    connectedWires.push_back(wire);

    QVector<std::shared_ptr<wire_system::wire>> newList;
    do {
        newList.clear();
        // Go through all the wires in the net
        for (const auto& otherWire: wire->net()->wires()) {
            // Ignore if the wire is already in the list
            if (connectedWires.contains(otherWire)) {
                continue;
            }

            // If they are connected to one of the wire in the list add them to the new list
            for (const auto& wire2 : connectedWires) {
                if (wire2->connected_wires().contains(otherWire.get())) {
                    newList << otherWire;
                    break;
                }
                if (otherWire->connected_wires().contains(wire2.get())) {
                    newList << otherWire;
                    break;
                }
            }
        }

        connectedWires << newList;
    } while (not newList.isEmpty());

    return connectedWires;
}

/**
 * Disconnects the a wire from another and takes care of updating the wirenets.
 * \param wire The wire that the other is attached to
 * \param otherWire The wire that is being disconnected
 */
void wire_manager::disconnectWire(const std::shared_ptr<wire_system::wire>& wire, wire_system::wire* otherWire)
{
    wire->disconnectWire(otherWire);
    auto net = otherWire->net();
    // Create a list of wires that will stay in the old net
    QVector<std::shared_ptr<wire_system::wire>> oldWires = wiresConnectedTo(wire);
    // If there are wires that are not in the list create a new net
    if (net->wires().count() != oldWires.count()) {
        // Create new net and add the wire
        auto newNet = std::make_shared<WireNet>();
        addWireNet(std::static_pointer_cast<wire_system::net>(newNet));
        for (auto wireToMove: net->wires()) {
            if (oldWires.contains(wireToMove)) {
                continue;
            }
            newNet->addWire(wireToMove);
            net->removeWire(wireToMove);
        }
    }
}

bool wire_manager::addWire(const std::shared_ptr<wire>& wire)
{
    // Sanity check
    if (!wire) {
        return false;
    }

    // No point of the new wire lies on an existing line segment - create a new wire net
    auto newNet = std::make_shared<WireNet>();
    newNet->addWire(wire);
    addWireNet(std::static_pointer_cast<wire_system::net>(newNet));

    return true;
}

void wire_manager::wirePointMovedByUser(wire& rawWire, int index)
{
    point point = rawWire.points().at(index);

    emit wirePointMoved(rawWire, index);

    // Detach wires
    if (index == 0 or index == rawWire.points_count() - 1){
        if (point.is_junction()) {
            for (const auto& wire: wires()) {
                // Skip current wire
                if (wire.get() == &rawWire) {
                    continue;
                }
                // If is connected
                if (wire->connected_wires().contains(&rawWire)) {
                    bool shouldDisconnect = true;
                    // Keep the wires connected if there is another junction
                    for (const auto& jIndex : rawWire.junctions()) {
                        const auto& junction = rawWire.points().at(jIndex);
                        // Ignore the point that moved
                        if (jIndex == index) {
                            continue;
                        }
                        // If the point is on the line stay connected
                        if (wire->point_is_on_wire(junction.toPointF())) {
                            shouldDisconnect = false;
                            break;
                        }
                    }
                    if (shouldDisconnect) {
                        disconnectWire(wire, &rawWire);
                    }
                    rawWire.set_point_is_junction(index, false);
                }
            }
        }
    }

    // Attach point to wire if needed
    if (index == 0 or index == rawWire.points().count() - 1) {
        for (const auto& wire: wires()) {
            // Skip current wire
            if (wire.get() == &rawWire) {
                continue;
            }
            if (wire->point_is_on_wire(rawWire.points().at(index).toPointF())) {
                if (not rawWire.connected_wires().contains(wire.get())) {
                    rawWire.set_point_is_junction(index, true);
                    connectWire(wire.get(), &rawWire);
                }
            }
        }
    }
}

void wire_manager::attach_wire_to_connector(const std::shared_ptr<wire>& wire, int index, const std::shared_ptr<Connector>& connector)
{
    if (not wire or not connector) {
        return;
    }

    // TODO: Check if it make sense for the index to be -1 or is this an error?
    if (index < -1 or wire->points().count() < index) {
        return;
    }

    // Ignore if there is already one attached
    if (_connections.contains(connector)) {
        return;
    }

    _connections.insert(connector, { wire, index });

    // Move the wire when the connector or its parent node moves
    connect(connector.get(), &Connector::movedInScene, this, [=, connector = connector.get()] { connectorMoved(connector->sharedPtr<Connector>()); });
}

/**
 * Connects a wire to a connector and finds out with end should be connected.
 * \remark If the connector is not on one of the ends, it does nothing
 */
void wire_manager::attach_wire_to_connector(const std::shared_ptr<wire>& wire, const std::shared_ptr<Connector>& connector)
{
    // Check if it's the first point
    if (wire->points().first().toPoint() == connector->scenePos().toPoint()) {
        attach_wire_to_connector(wire, 0, connector);
    }

    // Check if it's the last point
    else if (wire->points().last().toPoint() == connector->scenePos().toPoint()) {
        attach_wire_to_connector(wire, wire->points().count() - 1, connector);
    }
}

void wire_manager::point_inserted(const wire* wire, int index)
{
    for (const auto& connector : _connections.keys()) {
        // Skip if it's not the connected to the wire
        auto wirePoint = _connections.value(connector);
        if (_connections.value(connector).first.get() != wire) {
            continue;
        }
        // Do nothing if the connected point is the first
        if (wirePoint.second == 0) {
            continue;
        }
        // Inserted point comes before the connected point or the last point is connected
        else if (wirePoint.second >= index or wirePoint.second == wire->points_count() - 2) {
            wirePoint.second++;
        }
        // Update the connection
        _connections.insert(connector, wirePoint);
    }
}

void wire_manager::point_removed(const wire* wire, int index)
{
    for (const auto& connector : _connections.keys()) {
        // Skip if it's not the connected to the wire
        auto wirePoint = _connections.value(connector);
        if (_connections.value(connector).first.get() != wire) {
            continue;
        }
        if (wirePoint.second >= index) {
            wirePoint.second--;
        }
        // Update the connection
        _connections.insert(connector, wirePoint);
    }
}

QList<std::shared_ptr<Connector>> wire_manager::connectors_attached_to_wire(const std::shared_ptr<wire>& wire)
{
    QList<std::shared_ptr<Connector>> connectors;
    for (const auto& connector : _connections.keys()) {
        if (_connections.value(connector).first == wire) {
            connectors.append(connector);
        }
    }
    return connectors;
}

void wire_manager::detachWire(const std::shared_ptr<Connector>& connector)
{
    const auto& wire = _connections.value(connector).first;
    _connections.remove(connector);
}

std::shared_ptr<wire> wire_manager::wireWithExtremityAt(const QPointF& point)
{
    for (const auto& wire : wires()) {
        for (const auto& point : wire->points()) {
            if (point.toPoint() == point) {
                return wire;
            }
        }
    }
    return nullptr;
}

void wire_manager::detach_wire_from_all(const std::shared_ptr<wire>& wire)
{
    for (const auto& connector : _connections.keys()) {
        // Skip if it's not the connected to the wire
        auto wirePoint = _connections.value(connector);
        if (_connections.value(connector).first != wire) {
            continue;
        }

        _connections.remove(connector);
    }
}

std::shared_ptr<wire> wire_manager::attached_wire(const std::shared_ptr<Connector>& connector)
{
    if (not _connections.contains(connector)) {
        return nullptr;
    }
    return _connections.value(connector).first;
}

int wire_manager::attachedWirepoint(const std::shared_ptr<Connector>& connector)
{
    if (not _connections.contains(connector)) {
        return -1;
    }
    return _connections.value(connector).second;
}

void wire_manager::connectorMoved(const std::shared_ptr<Connector>& connector)
{
    if (not _connections.contains(connector)) {
        return;
    }
    const auto wirePoint = _connections.value(connector);

    if (wirePoint.second < -1 or wirePoint.first->points_count() <= wirePoint.second) {
        return;
    }

    QPointF oldPos = wirePoint.first->points().at(wirePoint.second).toPointF();
    QVector2D moveBy = QVector2D(connector->scenePos() - oldPos);
    if (not moveBy.isNull()) {
        wirePoint.first->move_point_by(wirePoint.second, moveBy);
    }
}

/**
 * Returns whether the wire's point is attached to a connector
 */
bool wire_manager::wire_point_is_attached(wire_system::wire* wire, int index)
{
    for (const auto& wire_point : _connections.values()) {
        if (wire_point.first.get() != wire) {
            continue;
        }
        if (wire_point.second == index) {
            return true;
        }
    }
    return false;
}

void wire_manager::set_settings(const Settings& settings)
{
    _settings = settings;
}

Settings wire_manager::settings() const
{
    return _settings;
}
