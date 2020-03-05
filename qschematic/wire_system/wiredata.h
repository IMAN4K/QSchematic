#pragma once

#include <QList>
#include <QObject>
#include <items/wirepoint.h>

namespace QSchematic
{
    class WireData : public QObject
    {
        Q_OBJECT

    public:
        QVector<WirePoint> wirePoints() const;
        QVector<QPointF> points() const;
        int pointCount() const;
        void appendPoint(const WirePoint& point);
        void prependPoint(const WirePoint& point);
        void insertPoint(int index, const WirePoint& point);
        const WirePoint& first() const;
        WirePoint last() const;
        WirePoint pointAt(int index) const;
        void replacePoint(int index, const WirePoint& point);
        void removeFirstPoint();
        void removeLastPoint();
        void removePoint(int index);
        void removeObsoletePoints();

    signals:
        void pointRemoved(int index);

        // TODO: All these members should be private
    protected:
        QVector<WirePoint> _points;
    };
}