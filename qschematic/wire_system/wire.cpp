#include <items/line.h>
#include "wire.h"
#include "items/wire.h"
#include <QVector2D>
#include <QLine>

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

void wire::move_junctions_to_new_segment(const Line& oldSegment, const Line& newSegment)
{
    // Do nothing if the segment was just resized
    if (qFuzzyCompare(oldSegment.toLineF().angle(), newSegment.toLineF().angle())) {
        return;
    }

    // Move connected junctions
    for (const auto& wire: _connectedWires) {
        for (const auto& jIndex: wire->junctions()) {
            WirePoint point = wire->points().at(jIndex);
            // Check if the point is on the old segment
            if (oldSegment.containsPoint(point.toPoint(), 5)) {
                Line junctionSeg;
                // Find out if one of the segments is horizontal or vertical
                if (jIndex < wire->points().count() - 1) {
                    Line seg = wire->line_segments().at(jIndex);
                    if (seg.isHorizontal() or seg.isVertical()) {
                        junctionSeg = seg;
                    }
                }
                if (jIndex > 0) {
                    Line seg = wire->line_segments().at(jIndex - 1);
                    if (seg.isHorizontal() or seg.isVertical()) {
                        junctionSeg = seg;
                    }
                }
                // Only move in the direction of the segment if it is hor. or vert.
                if (!junctionSeg.isNull()) {
                    QPointF intersection;
                    auto type = junctionSeg.toLineF().intersect(newSegment.toLineF(), &intersection);
                    if (type != QLineF::NoIntersection) {
                        wire->movePointBy(jIndex, QVector2D(intersection - point.toPointF()));
                    }
                }
                    // Move the point along the segment so that it stays at the same proportional distance from the two points
                else {
                    QPointF d =  point.toPointF() - oldSegment.p1();
                    qreal ratio = QVector2D(d).length() / oldSegment.lenght();
                    QPointF pos = newSegment.toLineF().pointAt(ratio);
                    wire->movePointBy(jIndex, QVector2D(pos - point.toPointF()));
                }
            }
        }
    }
}
