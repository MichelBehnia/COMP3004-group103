#include "Item.h"

Item::Item(const QString& title,
           const QString& creator,
           int year,
           const QString& format,
           ItemCondition condition)
    : itemId(QUuid::createUuid()),
      title(title),
      creator(creator),
      publicationYear(year),
      format(format),
      condition(condition),
      status(ItemStatus::Available)
{
}

Item::~Item() {
}

// Returns the item's effective status for a specific patron which shows Available if they are first in the hold queue
ItemStatus Item::getStatusForPatron(const QString& patronName) const {
    if (status == ItemStatus::OnHold) {
        if (!holdQueue.isEmpty() && holdQueue.first() == patronName) {
            return ItemStatus::Available;
        }
    }
    return status;
}
