#pragma once

#include <qschematic-export.h>

#include "item.hpp"
#include "../wire_system/connectable.hpp"

namespace QSchematic::Items
{

    class Label;
    class Wire;

    class QSCHEMATIC_EXPORT Connector :
        public Item,
        public wire_system::connectable
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(Connector)

    public:
        enum SnapPolicy {
            Anywhere,
            NodeSizerect,
            NodeSizerectOutline
        };
        Q_ENUM(SnapPolicy)

        Connector(int type = Item::ConnectorType, const QPoint& gridPos = QPoint(), const QString& text = QString(), QGraphicsItem* parent = nullptr);
        ~Connector() override;

        gpds::container to_container() const override;
        void from_container(const gpds::container& container) override;
        std::shared_ptr<Item> deepCopy() const override;

        void setSnapPolicy(SnapPolicy policy);
        SnapPolicy snapPolicy() const;
        void setText(const QString& text);
        QString text() const;
        void setForceTextDirection(bool enabled);
        bool forceTextDirection() const;
        void setForcedTextDirection(TextDirection direction);
        TextDirection textDirection() const;
        void update() override;

        /**
         * Checks whether a wire is connected to this connector.
         *
         * @return Whether a wire is connected to this connector.
         */
        bool
        hasConnection() const;

        QPointF connectionPoint() const;
        std::shared_ptr<Label> label() const;
        void alignLabel();
        QRectF boundingRect() const override;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

        // Connectable
        QPointF position() const override;

    protected:
        void copyAttributes(Connector& dest) const;
        QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

    private:
        void calculateSymbolRect();
        void calculateTextDirection();
        void disconnect_all_wires();
        void notify_wire_manager();

        SnapPolicy _snapPolicy;
        QRectF _symbolRect;
        bool _forceTextDirection;
        TextDirection _textDirection;
        std::shared_ptr<Label> _label;
    };

}
