#ifndef LOANSERVICE_H
#define LOANSERVICE_H

#include "User.h"
#include "Item.h"
#include "LibraryService.h"
#include <QUuid>
#include <QDate>

class LoanService {
public:
    explicit LoanService(LibraryService* libService);

    // Loan operations
    ActionResult borrowItem(Patron* patron, const QUuid& itemId);
    ActionResult returnItem(Patron* patron, const QUuid& itemId);

    // Query helpers
    bool patronHasLoan(const Patron& patron, const QUuid& itemId) const;
    bool canBorrow(const Patron* patron, const Item* item) const;

private:
    LibraryService* libraryService;
};

#endif // LOANSERVICE_H
