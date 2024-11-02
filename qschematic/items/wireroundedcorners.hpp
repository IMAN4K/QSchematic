#pragma once

#include <qschematic-export.h>

#include "wire.hpp"

namespace QSchematic::Items
{

    class QSCHEMATIC_EXPORT WireRoundedCorners :
        public Wire
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(WireRoundedCorners)

    public:
        WireRoundedCorners(int type = Item::WireRoundedCornersType, QGraphicsItem* parent = nullptr);
        ~WireRoundedCorners() override = default;

        gpds::container to_container() const override;
        void from_container(const gpds::container& container) override;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    private:
        enum QuarterCircleSegment {
            None,
            TopLeft,
            TopRight,
            BottomLeft,
            BottomRight
        };
    };

}
