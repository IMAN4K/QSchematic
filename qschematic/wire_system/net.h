#pragma once

#include <QList>
#include <memory>

namespace QSchematic
{
    class Wire;
}

namespace wire_system
{
    class wire;
    class wire_manager;

    class net : public std::enable_shared_from_this<net>
    {
    public:
        void set_name(const std::string& name);
        virtual void set_name(const QString& name);
        [[nodiscard]] QString name() const;
        [[nodiscard]] QList<std::shared_ptr<wire>> wires() const;
        [[nodiscard]] virtual bool addWire(const std::shared_ptr<wire>& wire);
        [[nodiscard]] virtual bool removeWire(const std::shared_ptr<wire> wire);
        [[nodiscard]] bool contains(const std::shared_ptr<wire>& wire) const;
        void set_manager(wire_system::wire_manager* manager);

    protected:
        QList<std::weak_ptr<wire>> m_wires;
        QString m_name;
        wire_manager* m_manager;
    };
}