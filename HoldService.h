#ifndef HOLDSERVICE_H
#define HOLDSERVICE_H

#include "User.h"
#include "Item.h"
#include "LibraryService.h"
#include "LoanService.h"
#include <QUuid>

class HoldService {
public:
    HoldService(LibraryService* libService, LoanService* loanService);

    // Hold operations
    ActionResult placeHold(Patron* patron, const QUuid& itemId);
    ActionResult cancelHold(Patron* patron, const QUuid& itemId);

    // Query helpers
    bool patronHasHold(const Patron& patron, const QUuid& itemId) const;
    int getQueuePosition(const Patron& patron, const Item* item) const;

private:
    LibraryService* libraryService;
    LoanService* loanService;
};

#endif // HOLDSERVICE_H
