#pragma once

#include <QList>
#include <QVector2D>
#include <memory>
#include "point.h"

namespace wire_system
{
    class wire_manager;
    class net;
    class line;

    class wire
    {

    public:
        wire();
        void set_manager(wire_manager* manager);
        [[nodiscard]] QVector<point> points() const;
        [[nodiscard]] int points_count() const;
        [[nodiscard]] QVector<int> junctions() const;
        [[nodiscard]] QList<wire*> connected_wires();
        [[nodiscard]] QList<line> line_segments() const;
        virtual void move_point_to(int index, const QPointF& moveTo);
        void set_point_is_junction(int index, bool isJunction);
        virtual void prepend_point(const QPointF& point);
        virtual void append_point(const QPointF& point);
        virtual void insert_point(int index, const QPointF& point);
        void move_point_by(int index, const QVector2D& moveBy);
        [[nodiscard]] bool point_is_on_wire(const QPointF& point) const;
        void move(const QVector2D& movedBy);
        void simplify();
        [[nodiscard]] bool connect_wire(wire* wire);
        void setNet(const std::shared_ptr<wire_system::net>& net);
        [[nodiscard]] std::shared_ptr<wire_system::net> net();
        void disconnectWire(wire* wire);
        virtual void add_segment(int index);

    protected:
        void move_junctions_to_new_segment(const line& oldSegment, const line& newSegment);
        void move_line_segment_by(int index, const QVector2D& moveBy);
        wire_manager* manager();

        QVector<point> _points;

    private:
        void remove_duplicate_points();
        void remove_obsolete_points();
        virtual void about_to_change();
        virtual void has_changed();

        QList<wire*> _connectedWires;
        std::shared_ptr<wire_system::net> _net;
        wire_manager* m_manager;
    };
}