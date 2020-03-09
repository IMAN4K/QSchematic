#include <items/line.h>
#include "wire.h"

using namespace wire_system;

QVector<WirePoint> wire::points() const
{
    return _points;
}

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

QList<Line> wire::line_segments() const
{
    // A line segment requires at least two points... duuuh
    if (points_count() < 2) {
        return QList<Line>();
    }

    QList<Line> ret;
    for (int i = 0; i < points_count() - 1; i++) {
        ret.append(Line(_points.at(i).toPointF(), _points.at(i+1).toPointF()));
    }

    return ret;
}
