#pragma once

#include "item.h"
#include "wirepoint.h"
#include "line.h"

class QVector2D;

namespace QSchematic {

    class Line;

    /**
     * IMPORTANT NOTE: The points coordinates are ABSOLUTE and in SCHEMATIC COORDINATES.
     */
    class Wire : public Item
    {
        Q_OBJECT

    public:
        enum WireType {
            Normal,
            Bus
        };
        Q_ENUM(WireType)

        Wire(QGraphicsItem* parent = nullptr);
        virtual ~Wire() override = default;
        virtual void update() override;

        virtual QRectF boundingRect() const override;
        virtual QPainterPath shape() const override;

        void prependPoint(const QPoint& point);
        void appendPoint(const QPoint& point);
        void insertPoint(int index, const QPoint& point);        // Index of new point
        void removeFirstPoint();
        void removeLastPoint();
        void removePoint(const QPoint& point);
        int removeDuplicatePoints();
        int removeObsoletePoints();
        void movePointBy(size_t index, const QVector2D& moveBy);
        void movePointTo(size_t index, const QPoint& moveTo);
        void moveLineSegmentBy(size_t index, const QVector2D& moveBy);
        void setPointIsJunction(size_t index, bool isJunction);
        bool pointIsOnWire(const QPoint& point) const;

        QVector<QPoint> points() const;
        QList<QSchematic::Line> lineSegments() const;

    signals:
        void pointMoved(Wire& wire, WirePoint& point);

    protected:
        virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
        virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        Q_DISABLE_COPY(Wire)

        QVector<WirePoint> sceneWirePointsRelative() const;
        QVector<QPoint> scenePointsRelative() const;
        QVector<QPoint> scenePointsAbsolute() const;
        void calculateBoundingRect();

        Wire::WireType _type;
        QVector<WirePoint> _points; // Store grid coordinates, not scene coordinates, but RELATIVE to this object
        QRectF _rect;
        int _pointToMoveIndex;
        int _lineSegmentToMoveIndex;
        QPoint _prevMousePos;   // Store grid coordinates, not scene coordinates
    };

}