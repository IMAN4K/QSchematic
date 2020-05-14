#pragma once

#include "wire_system/connectable.h"

struct connector : public wire_system::connectable
{
    QPointF position() const override
    {
        return pos;
    }

    QPointF pos;
};