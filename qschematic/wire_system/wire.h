#pragma once

#include <QList>
#include <QVector2D>
#include "point.h"

using namespace QSchematic; // TODO: Needs to be removed

namespace wire_system
{
    class wire_manager;

    class wire
    {

    public:
        wire();
        void set_manager(wire_manager* manager);
        QVector<point> points() const;
        int points_count() const;
        QVector<int> junctions() const;
        QList<wire*> connected_wires();
        QList<Line> line_segments() const;
        virtual void move_point_to(int index, const QPointF& moveTo);
        void set_point_is_junction(int index, bool isJunction);
        virtual void prepend_point(const QPointF& point);
        virtual void append_point(const QPointF& point);
        virtual void insert_point(int index, const QPointF& point);
        void move_point_by(int index, const QVector2D& moveBy);
        bool point_is_on_wire(const QPointF& point) const;
        void move(const QVector2D& movedBy);
        void simplify();
        bool connect_wire(wire* wire);

    protected: // TODO: All these members should be private
        void move_junctions_to_new_segment(const Line& oldSegment, const Line& newSegment);
        virtual void about_to_change();
        virtual void has_changed();
        void move_line_segment_by(int index, const QVector2D& moveBy);
        virtual void add_segment(int index);

        wire_manager* m_manager;
        QVector<point> _points;
        QList<wire*> _connectedWires;

    private:
        void remove_duplicate_points();
        void remove_obsolete_points();
    };
}