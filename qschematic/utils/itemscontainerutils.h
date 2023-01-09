#pragma once

#include "../items/item.h"

#include <QList>

#include <vector>
#include <memory>

namespace QSchematic::ItemUtils
{

    template <
        template <typename... T> typename OutContainerT = std::vector,
        typename ContainerT
    >
    [[nodiscard]]
    OutContainerT<std::shared_ptr<Items::Item>>
    mapItemListToSharedPtrList(ContainerT itemList)
    {
        OutContainerT<std::shared_ptr<Items::Item>> out;

        if constexpr (!std::is_same_v<OutContainerT<Items::Item>, QList<Items::Item>>)
            out.reserve(itemList.count());

        for (auto& item : itemList) {
            if (auto qsitem = qgraphicsitem_cast<Items::Item*>(item))
                out.push_back(qsitem->sharedPtr());
        }

        return out;
    }

}
