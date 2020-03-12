#include "net.h"

#include <QString>
#include "../items/wire.h"

using namespace wire_system;

void net::set_name(const std::string& name)
{
    set_name(QString::fromStdString(name));
}

void net::set_name(const QString& name)
{
    _name = name;
}

QString net::name() const
{
    return _name;
}

QList<std::shared_ptr<Wire>> net::wires() const
{
    QList<std::shared_ptr<Wire>> list;

    for (const auto& wire: _wires) {
        list.append(wire.lock());
    }

    return list;
}
