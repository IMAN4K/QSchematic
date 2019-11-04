#pragma once

#include <memory>
#include <QGraphicsObject>
#include <gpds/serialize.h>
#include "../types.h"
#include "../settings.h"


#include <QDebug>

namespace QSchematic {

    class Scene;



    // TODO
    // - make all below `static` member of class Item
    // - make one public static get method also (to up-down cast QG-ptrs)
    // - explicit _maybe version, others will assert-throw if expectations unmet
    extern std::unordered_map<const QGraphicsItem*, std::weak_ptr<QGraphicsItem>>
        global_items_shared_ptr_registry;

    template <typename wantedT = QGraphicsItem, typename T>
    auto obtain_registry_weak_pointer(T* ptr) -> std::weak_ptr<wantedT>
    {
        auto qg_ptr = static_cast<const QGraphicsItem*>(ptr);
        if ( global_items_shared_ptr_registry.find(qg_ptr) != end(global_items_shared_ptr_registry) ) {
            auto sh_ptr = global_items_shared_ptr_registry.at(qg_ptr);
            if ( not sh_ptr.expired() ) {
                return sh_ptr;
            }
            else {
                qDebug() << "SHPTR-REG => got ptr, but it's not alive anymore!";
                return {};
            }
        }
        else {
            qDebug() << "SHPTR-REG => no matching shared-ptr found!";
            return {};
        }
    }

    template <typename wantedT = QGraphicsItem, typename T>
    auto obtain_registry_shared_pointer(T* ptr) -> std::shared_ptr<wantedT>
    {
        auto w_ptr = obtain_registry_weak_pointer(ptr);
        auto sh_ptr = w_ptr.lock();
        return sh_ptr;
    }

    template <typename InstanceT, typename ...ArgsT>
    auto mk_sh(ArgsT ...args) -> std::shared_ptr<InstanceT>
    {
        auto sh_ptr = std::make_shared<InstanceT>(args...);
        auto root_type_ptr = static_cast<QGraphicsItem*>(sh_ptr.get());
        global_items_shared_ptr_registry[root_type_ptr] = sh_ptr;
        return sh_ptr;
    }

    // TODO: get_shptr_reg_stats()
    // TODO: shptr_reg_cleanup()
    // TODO: register_shptr_reg_entry(shptr/weakptr)
    // TODO: make_shared<>() that automatically registers (preferably also TIMEPOINT/SERIAL and/or stack-point)
    // TODO: assert_shptr_dead(shptr)
    // TODO: assert_shptr_alive(shptr)



    class Item :
        public QGraphicsObject,
        public Gpds::Serialize
        //,
        //public std::enable_shared_from_this<Item>
    {
        friend class CommandItemSetVisible;

        Q_OBJECT
        Q_DISABLE_COPY(Item)

    public:
        enum ItemType {
            NodeType               = QGraphicsItem::UserType + 1,
            WireType,
            WireRoundedCornersType,
            ConnectorType,
            LabelType,
            SplineWireType,

            QSchematicItemUserType = QGraphicsItem::UserType + 100
        };
        Q_ENUM(ItemType)

        const QString JSON_ID_STRING = QStringLiteral("type_id");

        Item(int type, QGraphicsItem* parent = nullptr);

        /**
         * These funcs should be the only source for obtaining a canonical
         * shared-/weak-ptr to the item. It _must_ be allocated with make_shared
         * or shared-constructor — ,no compile time check validates that.
         * For convenience it's also possible to cast by simply explicitly
         * passing a template arg
         */
        /// @{
        template <typename RetT = Item>
        auto sharedPtr() const -> std::shared_ptr<const RetT>
        {
            auto sh_ptr = obtain_registry_shared_pointer(this);

//            if constexpr (std::is_same_v<RetT, Item>) {
//                return sh_ptr;
//            }
//            else {
                return std::dynamic_pointer_cast<const RetT>(sh_ptr);
//            }
        }

        template <typename RetT = Item>
        auto sharedPtr() -> std::shared_ptr<RetT>
        {
            auto sh_ptr = obtain_registry_shared_pointer(this);

//            if constexpr (std::is_same_v<RetT, Item>) {
//                return sh_ptr;
//            }
//            else {
                return std::dynamic_pointer_cast<RetT>(sh_ptr);
//            }
        }

        template <typename RetT = Item>
        auto weakPtr() const -> std::weak_ptr<const RetT>
        {
            auto w_ptr = obtain_registry_weak_pointer(this);
//            if constexpr (std::is_same_v<RetT, Item>) {
//                return w_ptr;
//            }
//            else {
                return std::dynamic_pointer_cast<const RetT>(w_ptr);
//            }
        }

        template <typename RetT = Item>
        auto weakPtr() -> std::weak_ptr<RetT>
        {
            auto w_ptr = obtain_registry_weak_pointer(this);
//            if constexpr (std::is_same_v<RetT, Item>) {
//                return w_ptr;
//            }
//            else {
                return std::dynamic_pointer_cast<RetT>(w_ptr);
//            }
        }
        /// @}

        virtual Gpds::Container toContainer() const override;
        virtual void fromContainer(const Gpds::Container& container) override;
        virtual std::shared_ptr<Item> deepCopy() const = 0;

        int type() const final;
        void setGridPos(const QPoint& gridPos);
        void setGridPos(int x, int y);
        void setGridPosX(int x);
        void setGridPosY(int y);
        QPoint gridPos() const;
        int gridPosX() const;
        int gridPosY() const;
        void setPos(const QPointF& pos);
        void setPos(qreal x, qreal y);
        void setPosX(qreal x);
        void setPosY(qreal y);
        QPointF pos() const;
        qreal posX() const;
        qreal posY() const;
        void setScenePos(const QPointF& point);
        void setScenePos(qreal x, qreal y);
        void setScenePosX(qreal x);
        void setScenePosY(qreal y);
        QPointF scenePos() const;
        qreal scenePosX() const;
        qreal scenePosY() const;
        void moveBy(const QVector2D& moveBy);
        void setSettings(const Settings& settings);
        const Settings& settings() const;
        void setMovable(bool enabled);
        bool isMovable() const;
        void setSnapToGrid(bool enabled);
        bool snapToGrid() const;
        void setHighlighted(bool isHighlighted);
        void setHighlightEnabled(bool enabled);
        bool highlightEnabled() const;
        QPixmap toPixmap(QPointF& hotSpot, qreal scale = 1.0);
        virtual void update();
        Scene* scene() const;

    signals:
        void moved(Item& item, const QVector2D& movedBy);
        void rotated(Item& item, const qreal rotation);
        void showPopup(const Item& item);
        void highlightChanged(const Item& item, bool isHighlighted);
        void settingsChanged();

    protected:
        Settings _settings;

        void copyAttributes(Item& dest) const;
        void addItemTypeIdToContainer(Gpds::Container& container) const;

        bool isHighlighted() const;
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

    private slots:
        void posChanged();
        void rotChanged();

    private:
        int _type;
        bool _snapToGrid;
        bool _highlightEnabled;
        bool _highlighted;
        QPointF _oldPos;
        qreal _oldRot;
    };

}
