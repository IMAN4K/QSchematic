#pragma once

#include <QList>
#include "items/wirepoint.h"

namespace QSchematic {
    class Wire;
}

using namespace QSchematic; // TODO: Needs to be removed

namespace wire_system
{
    class wire
    {

    public:
        QVector<WirePoint> points() const;
        int points_count() const;
        QVector<int> junctions() const;
        QList<Wire*> connected_wires();
        QList<Line> line_segments() const;
        virtual void move_point_to(int index, const QPointF& moveTo);

    protected: // TODO: All these members should be private
        void move_junctions_to_new_segment(const Line& oldSegment, const Line& newSegment);

        QVector<WirePoint> _points;
        QList<Wire*> _connectedWires; // TODO: Should be QList<wire_system::wire>
    };
}