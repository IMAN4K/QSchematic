#pragma once

#include <QList>
#include <memory>

namespace QSchematic
{
    class Wire;
}

namespace wire_system
{
    class net
    {
    public:
        void set_name(const std::string& name);
        virtual void set_name(const QString& name);
        QString name() const;
        QList<std::shared_ptr<QSchematic::Wire>> wires() const;

    protected:
        QList<std::weak_ptr<QSchematic::Wire>> _wires;
        QString _name;
    };
}