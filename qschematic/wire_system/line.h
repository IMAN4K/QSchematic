#pragma once

#include <QPointF>

class QLineF;

namespace wire_system {

    class line
    {
    public:
        line() = default;
        line(int x1, int y1, int x2, int y2);
        line(qreal x1, qreal y1, qreal x2, qreal y2);
        line(const QPoint& p1, const QPoint& p2);
        line(const QPointF& p1, const QPointF& p2);

        QPointF p1() const;
        QPointF p2() const;
        bool is_null() const;
        bool is_horizontal() const;
        bool is_vertical() const;
        qreal lenght() const;
        QPointF mid_point() const;
        bool contains_point(const QPointF& point, qreal tolerance = 0) const;
        QPointF point_on_line_closest_to(const QPointF& point);
        QLineF toLineF() const;

        static bool contains_point(const QLineF& line, const QPointF& point, qreal tolerance = 0);

    private:
        QPointF _p1;
        QPointF _p2;
    };

}
