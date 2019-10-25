#pragma once

#include <vector>
#include <memory>
#include <QList>
#include "items/item.h"
#include <QDebug>

namespace QSchematic::ItemUtils
{

template <
    template <typename... T> typename OutContainerT = std::vector,
    typename ContainerT
>
auto mapItemListToSharedPtrList(ContainerT itemList) -> OutContainerT<std::shared_ptr<Item>>
{
    OutContainerT<std::shared_ptr<Item>> out;

    if constexpr (not std::is_same_v<OutContainerT<Item>, QList<Item>>) {
        out.reserve(itemList.count());
    }

    for (auto& item : itemList) {
        if (auto qitem = dynamic_cast<Item*>(item)) { //  qgraphicsitem_cast<Item*>(item)) {
            if (auto qsitem = qitem->sharedPtr()) {
                out.push_back(qsitem);
            }
            else {
                qDebug() << endl << "item without shared-ptr: " << qitem << endl;
            }
        } else {
            qDebug() << endl << "item without QS::ancestry" << item << endl;
        }
    }

    return out;
}




template <
    typename ItemT = QSchematic::Item,
    template <typename... T> typename OutContainerT = std::vector,
    template <typename... T> typename PtrMgrT = std::shared_ptr,
    typename SrcContainerT
    >
auto mapItemListToOtherList(SrcContainerT itemList) -> OutContainerT<PtrMgrT<ItemT>>
{
    OutContainerT<PtrMgrT<ItemT>> out;

    if constexpr (not std::is_same_v<OutContainerT<PtrMgrT<ItemT>>, QList<PtrMgrT<ItemT>>>) {
        out.reserve(itemList.size());
    }

    for (auto const& item : itemList) {
        if (auto sharedPtrItemBase = std::dynamic_pointer_cast<ItemT>(item)) {
            out.push_back(sharedPtrItemBase);
        }

//        if constexpr (std::is_base_of_v<QGraphicsItem, decltype(*item)>) {
//            if (auto qsitem = qgraphicsitem_cast<ItemT*>(item)) {
//                out.push_back(qsitem->sharedPtr());
//            }
//        }
//        else {
//            if (auto sharedPtrItemBase = std::dynamic_pointer_cast<ItemT>(item)) {
//                out.push_back(sharedPtrItemBase);
//            }
//        }

    }

    return out;
}


}
