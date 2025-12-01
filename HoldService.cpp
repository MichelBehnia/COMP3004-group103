#include "HoldService.h"
#include "DatabaseManager.h"
#include <algorithm>

HoldService::HoldService(LibraryService* libService, LoanService* loanService)
    : libraryService(libService), loanService(loanService)
{
}

// Places a hold on a checked-out item for the patron and adds them to the items queue
ActionResult HoldService::placeHold(Patron* patron, const QUuid& itemId) {
    if (!patron) {
        return {false, "No patron logged in."};
    }

    Item* item = libraryService->findItemById(itemId);
    if (!item) {
        return {false, "Item not found."};
    }

    if (item->status != ItemStatus::CheckedOut) {
        return {false, "You can only place holds on checked-out items."};
    }

    if (loanService->patronHasLoan(*patron, itemId)) {
        return {false, "You cannot place a hold on an item you are currently borrowing."};
    }

    if (patronHasHold(*patron, itemId)) {
        return {false, "You already have a hold on this item."};
    }

    item->holdQueue.push_back(patron->name);
    patron->activeHolds.push_back(itemId);

    DatabaseManager::instance().saveHold(patron->name, itemId.toString(), item->holdQueue.size() - 1);
    DatabaseManager::instance().updateItem(item);

    int position = item->holdQueue.size();
    QString msg = QString("Hold placed successfully. You are #%1 in the queue.").arg(position);

    return {true, msg};
}

// Removes a patron's hold from an item and updates the queue positions for remaining patrons
ActionResult HoldService::cancelHold(Patron* patron, const QUuid& itemId) {
    if (!patron) {
        return {false, "No patron logged in."};
    }

    Item* item = libraryService->findItemById(itemId);
    if (!item) {
        return {false, "Item not found."};
    }

    auto holdIt = std::find(patron->activeHolds.begin(), patron->activeHolds.end(), itemId);
    if (holdIt == patron->activeHolds.end()) {
        return {false, "You do not have a hold on this item."};
    }

    patron->activeHolds.erase(holdIt);

    auto qIt = std::find(item->holdQueue.begin(), item->holdQueue.end(), patron->name);
    if (qIt != item->holdQueue.end()) {
        item->holdQueue.erase(qIt);
    }

    DatabaseManager::instance().deleteHold(patron->name, itemId.toString());
    DatabaseManager::instance().updateHoldPositions(itemId.toString(), item->holdQueue);

    return {true, "Hold canceled successfully."};
}

// Checks if a patron already has an active hold on the specified item
bool HoldService::patronHasHold(const Patron& patron, const QUuid& itemId) const {
    return std::any_of(patron.activeHolds.begin(), patron.activeHolds.end(),
                       [&](const QUuid& x) { return x == itemId; });
}

// Returns the patrons position in the hold queue for an item (1-indexed), or -1 if not in queue
int HoldService::getQueuePosition(const Patron& patron, const Item* item) const {
    if (!item) return -1;

    for (int i = 0; i < item->holdQueue.size(); ++i) {
        if (item->holdQueue[i] == patron.name) {
            return i + 1;
        }
    }
    return -1;
}
