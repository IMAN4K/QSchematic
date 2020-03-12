#include <items/line.h>
#include "wire.h"
#include "wiresystem.h"
#include <QVector2D>
#include <QLineF>
#include <utils.h>

using namespace wire_system;

wire::wire() : m_manager(nullptr)
{
}

void wire::set_manager(wire_system::wire_manager* manager)
{
    m_manager = manager;
}

QVector<point> wire::points() const
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
    if (_points.first().is_junction()) {
        indexes.append(0);
    }
    if (_points.last().is_junction()) {
        indexes.append(points_count() - 1);
    }
    return indexes;
}

QList<wire*> wire::connected_wires()
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
            point point = wire->points().at(jIndex);
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
                        wire->move_point_by(jIndex, QVector2D(intersection - point.toPointF()));
                    }
                }
                    // Move the point along the segment so that it stays at the same proportional distance from the two points
                else {
                    QPointF d = point.toPointF() - oldSegment.p1();
                    qreal ratio = QVector2D(d).length() / oldSegment.lenght();
                    QPointF pos = newSegment.toLineF().pointAt(ratio);
                    wire->move_point_by(jIndex, QVector2D(pos - point.toPointF()));
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
            point point = wire->points().at(jIndex);
            if ((_points[index]).toPoint() == point.toPoint()) {
                wire->move_point_by(jIndex, QVector2D(moveTo - _points[index].toPointF()));
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

    point wirepoint = moveTo;
    wirepoint.set_is_junction(_points[index].is_junction());
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

    _points[index].set_is_junction(isJunction);

    has_changed(); // TODO: Make sure this correctly redraws the wire
}

void wire::prepend_point(const QPointF& point)
{
    about_to_change();
    _points.prepend(wire_system::point(point));
    has_changed();

    // Update junction
    if (points_count() >= 2) {
        set_point_is_junction(0, _points.at(1).is_junction());
        set_point_is_junction(1, false);
    }

    m_manager->point_inserted(this, 0);
}

void wire::append_point(const QPointF& point)
{
    about_to_change();
    _points.append(wire_system::point(point));
    has_changed();

    // Update junction
    if (points_count() > 2) {
        set_point_is_junction(points_count() - 1, _points.at(points_count() - 2).is_junction());
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
            point point = wire->points().at(jIndex);
            Line segment = line_segments().at(index);
            if (segment.containsPoint(point.toPointF())) {
                // Don't move it if it is on one of the points
                if (segment.p1().toPoint() == point.toPoint() or segment.p2().toPoint() == point.toPoint()) {
                    continue;
                }
                wire->move_point_by(jIndex, moveBy);
            }
        }
    }

    // If this is the first or last segment we might need to add a new segment
    if (index == 0 or index == line_segments().count() - 1) {
        // Get the correct point
        point point;
        if (index == 0) {
            point = points().first();
        } else {
            point = points().last();
        }

        int pointIndex = (index == 0) ? 0 : points_count() - 1;

        // Check if the segment is connected to a node
        bool isConnected = m_manager->wire_point_is_attached(this, pointIndex);

        // Check if it's connected to a wire
        if (not isConnected and point.is_junction()) {
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

void wire::insert_point(int index, const QPointF& point)
{
    // Boundary check
    if (index < 0 || index >= points_count()) {
        return;
    }

    Line segment = line_segments().at(index - 1);
    // If the point is not on the segment, move the junctions
    if (not segment.containsPoint(point)) {
        // Find the closest point on the segment
        QPointF closestPoint = Utils::pointOnLineClosestToPoint(segment.p1(), segment.p2(), point);
        // Create two line that split the segment at the closest point
        Line seg1(segment.p1(), closestPoint);
        Line seg2(closestPoint, segment.p2());
        // Calculate what will be the new segments
        Line seg1new(segment.p1(), point);
        Line seg2new(point, segment.p2());
        // Move the junction on both lines
        move_junctions_to_new_segment(seg1, seg1new);
        move_junctions_to_new_segment(seg2, seg2new);
    }

    about_to_change();
    _points.insert(index, wire_system::point(m_manager->settings().snapToGrid(point)));
    has_changed();

    m_manager->point_inserted(this, index);
}

void wire::move_point_by(int index, const QVector2D& moveBy)
{
    if (index < 0 or index > points_count() - 1) {
        return;
    }

    // If there are only two points (one line segment) and we are supposed to preserve
    // straight angles, we need to insert two additional points if we are not moving in
    // the direction of the line.
    if (points_count() == 2 && m_manager->settings().preserveStraightAngles) {
        const Line line = line_segments().first();

        bool moveVertically = line.isHorizontal() and not qFuzzyIsNull(moveBy.y());
        bool moveHorizontally = line.isVertical() and not qFuzzyIsNull(moveBy.x());
        // Only do this if we're not moving in the direction of the line. Because in that case
        // this is unnecessary as we're just moving one of the two points.
        if (not line.isNull() and (moveVertically or moveHorizontally)) {
            qreal lineLength = line.lenght();
            QPointF p;

            // The line is horizontal
            if (line.isHorizontal()) {
                QPointF leftPoint = line.p1();
                if (line.p2().x() < line.p1().x()) {
                    leftPoint = line.p2();
                }

                p.rx() = leftPoint.x() + static_cast<int>(lineLength/2);
                p.ry() = leftPoint.y();

                // The line is vertical
            } else {
                QPointF upperPoint = line.p1();
                if (line.p2().y() < line.p1().y()) {
                    upperPoint = line.p2();
                }

                p.rx() = upperPoint.x();
                p.ry() = upperPoint.y() + static_cast<int>(lineLength/2);
            }

            // Insert twice as these two points will form the new additional vertical or
            // horizontal line segment that is required to preserver straight angles.
            insert_point(1, p);
            insert_point(1, p);

            // Account for inserted points
            if (index == 1) {
                index += 2;
            }
        }
    }

    // Move the points
    QPointF currPoint = points().at(index).toPointF();
    // Preserve straight angles (if supposed to)
    if (m_manager->settings().preserveStraightAngles) {

        // Move previous point
        if (index >= 1) {
            QPointF prevPoint = points().at(index-1).toPointF();
            Line line(prevPoint, currPoint);

            // Make sure that two wire points never collide
            if (points_count() > 3 and index >= 2 and Line(currPoint+moveBy.toPointF(), prevPoint).lenght() <= 2) {
                move_line_segment_by(index - 2, moveBy);
            }

            // Move junctions before the points are moved
            if (not line.isNull() and (line.isHorizontal() or line.isVertical())) {
                // Move connected junctions
                for (const auto& wire: _connectedWires) {
                    for (const auto& jIndex: wire->junctions()) {
                        const auto& point = wire->points().at(jIndex);
                        if (line.containsPoint(point.toPointF())) {
                            // Don't move it if it is on one of the points
                            if (line.p1().toPoint() == point.toPoint() or line.p2().toPoint() == point.toPoint()) {
                                continue;
                            }
                            if (line.isHorizontal()) {
                                wire->move_point_by(jIndex, QVector2D(0, moveBy.y()));
                            } else {
                                wire->move_point_by(jIndex, QVector2D(moveBy.x(), 0));
                            }
                        }
                    }
                }
                // The line is horizontal
                if (line.isHorizontal()) {
                    move_point_to(index - 1, points().at(index - 1) + QPointF(0, moveBy.toPointF().y()));
                }
                    // The line is vertical
                else if (line.isVertical()) {
                    move_point_to(index - 1, points().at(index - 1) + QPointF(moveBy.toPointF().x(), 0));
                }
            }
        }

        // Move next point
        if (index < points_count()-1) {
            QPointF nextPoint = points().at(index+1).toPointF();
            Line line(currPoint, nextPoint);

            // Make sure that two wire points never collide
            if (points_count() > 3 and Line(currPoint+moveBy.toPointF(), nextPoint).lenght() <= 2) {
                move_line_segment_by(index + 1, moveBy);
            }

            // Move junctions before the points are moved
            if (not line.isNull() and (line.isHorizontal() or line.isVertical())) {
                // Move connected junctions
                for (const auto& wire: _connectedWires) {
                    for (const auto& jIndex: wire->junctions()) {
                        const auto& point = wire->points().at(jIndex);
                        if (line.containsPoint(point.toPointF())) {
                            // Don't move it if it is on one of the points
                            if (line.p1().toPoint() == point.toPoint() or line.p2().toPoint() == point.toPoint()) {
                                continue;
                            }
                            if (line.isHorizontal()) {
                                wire->move_point_by(jIndex, QVector2D(0, moveBy.y()));
                            } else {
                                wire->move_point_by(jIndex, QVector2D(moveBy.x(), 0));
                            }
                        }
                    }
                }
                // The line is horizontal
                if (line.isHorizontal()) {
                    move_point_to(index + 1, points().at(index + 1) + QPointF(0, moveBy.toPointF().y()));
                }
                    // The line is vertical
                else if (line.isVertical()) {
                    move_point_to(index + 1, points().at(index + 1) + QPointF(moveBy.toPointF().x(), 0));
                }
            }
        }
    }

    // Move the actual point itself
    move_point_to(index, currPoint + moveBy.toPointF());
}

bool wire::point_is_on_wire(const QPointF& point) const
{
    for (const Line& lineSegment : line_segments()) {
        if (lineSegment.containsPoint(point, 0)) {
            return true;
        }
    }

    return false;
}

void wire::move(const QVector2D& movedBy)
{
    // Ignore if it shouldn't move
    if (movedBy.isNull()) {
        return;
    }

    // Move junctions
    for (const auto& index : junctions()) {
        const auto& junction = points().at(index);
        for (const auto& wire : m_manager->wires()) {
            if (not wire->connected_wires().contains(this)) {
                continue;
            }
            if (wire->point_is_on_wire(junction.toPointF()) and not movedBy.isNull()) {
                move_point_by(index, -movedBy);
            }
        }
    }

    // Move junction on the wire
    for (const auto& wire : connected_wires()) {
        for (const auto& index : wire->junctions()) {
            const auto& point = wire->points().at(index);
            if (point_is_on_wire(point.toPointF())) {
                wire->move_point_by(index, movedBy);
            }
        }
    }

    // Move the points
    for (int index = 0; index < points_count(); index++) {
        move_point_to(index, _points[index].toPointF() + movedBy.toPointF());
    }
}