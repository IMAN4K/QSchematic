#include <items/line.h>
#include "wire.h"
#include "wiresystem.h"
#include "items/wire.h" // TODO: This has to be removed
#include <QVector2D>
#include <QLineF>

using namespace wire_system;

wire::wire() : m_manager(nullptr)
{
}

void wire::set_manager(wire_system::wire_manager* manager)
{
    m_manager = manager;
}

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
                    QPointF d = point.toPointF() - oldSegment.p1();
                    qreal ratio = QVector2D(d).length() / oldSegment.lenght();
                    QPointF pos = newSegment.toLineF().pointAt(ratio);
                    wire->movePointBy(jIndex, QVector2D(pos - point.toPointF()));
                }
            }
        }
    }
}

void wire::move_point_to(int index, const QPointF& moveTo)
{
    if (index < 0 or index > points_count() - 1) {
        return;
    }

    // Do nothing if it already is at that position
    if (points().at(index) == moveTo) {
        return;
    }

    // Move junctions that are on the point
    for (const auto& wire: _connectedWires) {
        for (const auto& jIndex: wire->junctions()) {
            WirePoint point = wire->points().at(jIndex);
            if ((_points[index]).toPoint() == point.toPoint()) {
                wire->movePointBy(jIndex, QVector2D(moveTo - _points[index].toPointF()));
            }
        }
    }

    // Move junctions on the next segment
    if (index < points_count() - 1) {
        Line segment = line_segments().at(index);
        Line newSegment(moveTo, points().at(index+1).toPointF());
        move_junctions_to_new_segment(segment, newSegment);
    }

    // Move junctions on the previous segment
    if (index > 0) {
        Line segment = line_segments().at(index - 1);
        Line newSegment(points().at(index-1).toPointF(), moveTo);
        move_junctions_to_new_segment(segment, newSegment);
    }

    WirePoint wirepoint = moveTo;
    wirepoint.setIsJunction(_points[index].isJunction());
    _points[index] = wirepoint;
}

/**
 * Is executed when the shape of the wire is about to change. This method can be
 * overridden by subclasses to prepare for such changes.
 */
void wire::about_to_change()
{

}

/**
 * Is executed when the shape of the wire has changed. This method can be
 * overridden by subclasses to react to such changes.
 */
void wire::has_changed()
{

}

void wire::set_point_is_junction(int index, bool isJunction)
{
    if (index < 0 or index > points_count() - 1) {
        return;
    }

    _points[index].setIsJunction(isJunction);

    has_changed(); // TODO: Make sure this correctly redraws the wire
}

void wire::prepend_point(const QPointF& point)
{
    about_to_change();
    _points.prepend(WirePoint(point));
    has_changed();

    // Update junction
    if (points_count() >= 2) {
        set_point_is_junction(0, _points.at(1).isJunction());
        set_point_is_junction(1, false);
    }

    m_manager->point_inserted(this, 0);
}

void wire::append_point(const QPointF& point)
{
    about_to_change();
    _points.append(WirePoint(point));
    has_changed();

    // Update junction
    if (points_count() > 2) {
        set_point_is_junction(points_count() - 1, _points.at(points_count() - 2).isJunction());
        set_point_is_junction(points_count() - 2, false);
    }

    m_manager->point_inserted(this, points_count() - 1);
}

void wire::move_line_segment_by(int index, const QVector2D& moveBy)
{
    // Do nothing if not moving
    if (moveBy.isNull()) {
        return;
    }

    // Have points_count()-2 in here because N points form N-1 line segments
    if (index < 0 or index > points_count() - 2) {
        return;
    }

    // Move connected junctions
    for (const auto& wire: _connectedWires) {
        for (const auto& jIndex: wire->junctions()) {
            WirePoint point = wire->points().at(jIndex);
            Line segment = line_segments().at(index);
            if (segment.containsPoint(point.toPointF())) {
                // Don't move it if it is on one of the points
                if (segment.p1().toPoint() == point.toPoint() or segment.p2().toPoint() == point.toPoint()) {
                    continue;
                }
                wire->movePointBy(jIndex, moveBy);
            }
        }
    }

    // If this is the first or last segment we might need to add a new segment
    if (index == 0 or index == line_segments().count() - 1) {
        // Get the correct point
        WirePoint point;
        if (index == 0) {
            point = points().first();
        } else {
            point = points().last();
        }

        int pointIndex = (index == 0) ? 0 : points_count() - 1;

        // Check if the segment is connected to a node
        bool isConnected = m_manager->wire_point_is_attached(this, pointIndex);

        // Check if it's connected to a wire
        if (not isConnected and point.isJunction()) {
            isConnected = true; // TODO: Check why the IDE thinks this is never used
        }

        // Add segment if it is connected
        if (isConnected) {
            add_segment(index);
        }
    }

    // Move the line segment
    // Move point 1
    move_point_to(index, _points[index] + moveBy.toPointF());
    // Move point 2
    move_point_to(index + 1, _points[index + 1] + moveBy.toPointF());
}

void wire::add_segment(int index)
{
    if (index == 0) {
        // Add a point
        prepend_point(_points.first().toPointF());
        // Increment indices to account for inserted point
        index++;
    } else {
        // Add a point
        append_point(_points.last().toPointF());
    }
}
