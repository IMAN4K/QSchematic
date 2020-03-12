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
        QString name() const;
        QList<std::shared_ptr<wire>> wires() const;
        virtual bool addWire(const std::shared_ptr<wire>& wire);
        virtual bool removeWire(const std::shared_ptr<wire> wire);
        bool contains(const std::shared_ptr<wire>& wire) const;
        void set_manager(wire_system::wire_manager* manager);

    protected:
        QList<std::weak_ptr<wire>> _wires;
        QString _name;
        wire_manager* m_manager;
    };
}