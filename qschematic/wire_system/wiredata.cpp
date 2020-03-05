#include <items/line.h>
#include <utils.h>
#include "wiredata.h"
#include <QLineF>

using namespace QSchematic;

QVector<WirePoint> WireData::wirePoints() const
{
    return _points;
}

QVector<QPointF> WireData::points() const
{
    QVector<QPointF> points;

    for (const WirePoint& point : _points) {
        points << point.toPointF();
    }

    return points;
}

int WireData::pointCount() const
{
    return _points.count();
}

void WireData::appendPoint(const WirePoint& point)
{
    _points.append(point);
}

void WireData::prependPoint(const WirePoint& point)
{
    _points.prepend(point);
}

void WireData::insertPoint(int index, const WirePoint& point)
{
    _points.insert(index, point);
}

const WirePoint& WireData::first() const
{
    return _points.first();
}

WirePoint WireData::last() const
{
    return _points.last();
}

WirePoint WireData::pointAt(int index) const
{
    return _points.at(index);
}

void WireData::replacePoint(int index, const WirePoint& point)
{
    _points[index] = point;
}

void WireData::removeFirstPoint()
{
    _points.removeFirst();
}

void WireData::removeLastPoint()
{
    _points.removeLast();
}

void WireData::removePoint(int index)
{
    _points.remove(index);
}

void WireData::removeObsoletePoints()
{
    // Don't do anything if there are not at least three line segments
    if (pointCount() < 3) {
        return;
    }

    // Compile a list of obsolete points
    auto it = _points.begin()+2;
    while (it != _points.end()) {
        QPointF p1 = (*(it - 2)).toPointF();
        QPointF p2 = (*(it - 1)).toPointF();
        QPointF p3 = (*it).toPointF();

        // Check if p2 is on the line created by p1 and p3
        if (Utils::pointIsOnLine(QLineF(p1, p2), p3)) {
            emit pointRemoved(_points.indexOf(*(it-1)));
            it = _points.erase(it-1);
        }
        it++;
    }
}
