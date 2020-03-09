#include "wire.h"
using namespace wire_system;

int wire::points_count() const
{
    return _points.count();
}

QVector<int> wire::junctions() const
{
    if (points_count() < 2) {
        return {};
    }
    QVector<int> indexes;
    if (_points.first().isJunction()) {
        indexes.append(0);
    }
    if (_points.last().isJunction()) {
        indexes.append(points_count() - 1);
    }
    return indexes;
}

QList<Wire*> wire::connected_wires()
{
    return _connectedWires;
}
