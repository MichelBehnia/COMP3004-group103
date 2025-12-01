#include "LoanService.h"
#include "DatabaseManager.h"
#include <algorithm>

LoanService::LoanService(LibraryService* libService)
    : libraryService(libService)
{
}

// Processes a borrow request, validating loan limits and item availability before checking out the item
ActionResult LoanService::borrowItem(Patron* patron, const QUuid& itemId) {
    if (!patron) {
        return {false, "No patron logged in."};
    }

    Item* item = libraryService->findItemById(itemId);
    if (!item) {
        return {false, "Item not found."};
    }

    if (patronHasLoan(*patron, itemId)) {
        return {false, "You already borrowed this item."};
    }

    if (patron->activeLoans.size() >= 3) {
        return {false, "Max 3 active loans reached (D1)."};
    }

    switch (item->status) {
        case ItemStatus::Available:
            break;
        case ItemStatus::CheckedOut:
            return {false, "Item is already checked out."};
        case ItemStatus::OnHold:
            if (item->holdQueue.isEmpty() || item->holdQueue.front() != patron->name) {
                return {false, "Item is on hold for another patron."};
            }
            item->holdQueue.pop_front();
            DatabaseManager::instance().deleteHold(patron->name, itemId.toString());
            DatabaseManager::instance().updateHoldPositions(itemId.toString(), item->holdQueue);
            break;
    }

    item->status = ItemStatus::CheckedOut;
    item->dueDate = QDate::currentDate().addDays(14);
    patron->activeLoans.push_back(itemId);

    DatabaseManager::instance().updateItem(item);
    DatabaseManager::instance().saveLoan(patron->name, itemId.toString(), item->dueDate);

    auto holdIt = std::find(patron->activeHolds.begin(), patron->activeHolds.end(), itemId);
    if (holdIt != patron->activeHolds.end()) {
        patron->activeHolds.erase(holdIt);
    }

    return {true, "Borrowed successfully."};
}

// Processes a return, updating item status to OnHold if others are waiting or Available otherwise
ActionResult LoanService::returnItem(Patron* patron, const QUuid& itemId) {
    if (!patron) {
        return {false, "No patron logged in."};
    }

    Item* item = libraryService->findItemById(itemId);
    if (!item) {
        return {false, "Item not found."};
    }

    auto itPos = std::find(patron->activeLoans.begin(), patron->activeLoans.end(), itemId);
    if (item->status != ItemStatus::CheckedOut || itPos == patron->activeLoans.end()) {
        return {false, "You don't have this item on loan."};
    }

    patron->activeLoans.erase(itPos);
    DatabaseManager::instance().deleteLoan(patron->name, itemId.toString());

    if (!item->holdQueue.isEmpty()) {
        item->status = ItemStatus::OnHold;
    } else {
        item->status = ItemStatus::Available;
    }

    item->dueDate = QDate();
    DatabaseManager::instance().updateItem(item);

    return {true, "Returned successfully."};
}

// Checks if the patron currently has the specified item on loan
bool LoanService::patronHasLoan(const Patron& patron, const QUuid& itemId) const {
    return std::any_of(patron.activeLoans.begin(), patron.activeLoans.end(),
                       [&](const QUuid& x) { return x == itemId; });
}

// Validates whether a patron is eligible to borrow an item based on loan limits and item status
bool LoanService::canBorrow(const Patron* patron, const Item* item) const {
    if (!patron || !item) return false;
    if (patronHasLoan(*patron, item->itemId)) return false;
    if (patron->activeLoans.size() >= 3) return false;
    if (item->status == ItemStatus::CheckedOut) return false;
    if (item->status == ItemStatus::OnHold &&
        (item->holdQueue.isEmpty() || item->holdQueue.front() != patron->name)) {
        return false;
    }
    return true;
}
