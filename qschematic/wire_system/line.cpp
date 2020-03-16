#include <QtGlobal>
#include <QLineF>
#include <QVector2D>
#include "utils.h"
#include "line.h"

using namespace wire_system;

line::line(int x1, int y1, int x2, int y2) :
    _p1(QPointF(x1, y1)),
    _p2(QPointF(x2, y2))
{
}

line::line(qreal x1, qreal y1, qreal x2, qreal y2) :
    _p1(QPointF(x1, y1)),
    _p2(QPointF(x2, y2))
{
}

line::line(const QPoint& p1, const QPoint& p2) :
    _p1(p1),
    _p2(p2)
{
}

line::line(const QPointF& p1, const QPointF& p2) :
    _p1(p1),
    _p2(p2)
{
}

QPointF line::p1() const
{
    return _p1;
}

QPointF line::p2() const
{
    return _p2;
}

bool line::is_null() const
{
    return qFuzzyCompare(_p1.x(), _p2.x()) && qFuzzyCompare(_p1.y(), _p2.y());
}

bool line::is_horizontal() const
{
    return qFuzzyCompare(_p1.y(), _p2.y());
}

bool line::is_vertical() const
{
    return qFuzzyCompare(_p1.x(), _p2.x());
}

qreal line::lenght() const
{
    return ::QLineF(_p1, _p2).length();
}

QPointF line::mid_point() const
{
    return (_p1 + _p2) / 2;
}

bool line::contains_point(const QPointF& point, qreal tolerance) const
{
    return contains_point(QLineF(_p1, _p2), point, tolerance);
}

QPointF line::point_on_line_closest_to(const QPointF& point)
{
    return QSchematic::Utils::pointOnLineClosestToPoint(_p1, _p2, point);
}

QLineF line::toLineF() const
{
    return QLineF(_p1, _p2);
}

bool line::contains_point(const QLineF& line, const QPointF& point, qreal tolerance)
{
    const qreal MIN_LENGTH = 0.01;
    tolerance = qMax(tolerance, MIN_LENGTH);

    if (line.isNull()) {
        QPointF linePoint = line.p1();
        if (QVector2D(linePoint).distanceToPoint(QVector2D(point)) <= tolerance) {
            return true;
        }
    } else {
        // Find perpendicular line
        QLineF normal = line.normalVector();
        // Move line to point
        QPointF offset = point - normal.p1();
        normal.translate(offset);
        // Set length to double the tolerance
        normal.setLength(2 * tolerance);
        // Move line so that the center lays on the point
        QVector2D unit(normal.unitVector().dx(), normal.unitVector().dy());
        offset = (unit * -tolerance).toPointF();
        normal.translate(offset);
        // Make the line longer by 2 * tolerance
        QLineF lineAdjusted = line;
        lineAdjusted.setLength(line.length() + 2 * tolerance);

        // Check if the lines are intersecting
        if (lineAdjusted.intersect(normal, nullptr) == QLineF::BoundedIntersection) {
            return true;
        }
    }

    return false;
}
