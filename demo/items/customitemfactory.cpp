#include "../qschematic/items/item.h"
#include "../qschematic/items/itemfactory.h"
#include "itemtypes.h"
#include "operation.h"
#include "operationconnector.h"
#include "operationdemo1.h"
#include "customitemfactory.h"
#include "fancywire.h"
#include "flowstart.h"
#include "flowend.h"

using namespace QSchematic;

std::shared_ptr<QSchematic::Item> CustomItemFactory::fromContainer(const Gpds::Container& container)
{
    // Extract the type
    QSchematic::Item::ItemType type = QSchematic::ItemFactory::extractType(container);

    // Create the item
    switch (static_cast<ItemType>(type)) {
    case ItemType::OperationType:
        return QSchematic::mk_sh<Operation>();

    case ItemType::OperationConnectorType:
        return QSchematic::mk_sh<OperationConnector>();

    case ItemType::OperationDemo1Type:
        return QSchematic::mk_sh<OperationDemo1>();

    case ItemType::FancyWireType:
        return QSchematic::mk_sh<FancyWire>();

    case ItemType::FlowStartType:
        return QSchematic::mk_sh<FlowStart>();

    case ItemType::FlowEndType:
        return QSchematic::mk_sh<FlowEnd>();
    }

    return {};
}
