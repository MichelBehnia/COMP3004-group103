#include "NonFictionBook.h"

NonFictionBook::NonFictionBook(const QString& title,
                               const QString& author,
                               int year,
                               const QString& format,
                               ItemCondition condition,
                               const QString& isbnNum,
                               const QString& deweyClass)
    : Item(title, author, year, format, condition), isbn(isbnNum), deweyClass(deweyClass)
{
}

QString NonFictionBook::typeName() const {
    return "Non-Fiction";
}
