#pragma once

#include <qschematic-export.h>

#include "wire.hpp"

namespace QSchematic::Items
{

    class QSCHEMATIC_EXPORT SplineWire :
        public QSchematic::Items::Wire
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(SplineWire)
    public:
        SplineWire(int type = Item::SplineWireType, QGraphicsItem* parent = nullptr);
        ~SplineWire() override = default;

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
        QPainterPath path() const;
        QPainterPath shape() const override;
        QRectF boundingRect() const override;
    };

}
