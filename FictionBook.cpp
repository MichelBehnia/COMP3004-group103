#include "FictionBook.h"

FictionBook::FictionBook(const QString& title,
                         const QString& author,
                         int year,
                         const QString& format,
                         ItemCondition condition,
                         const QString& isbnNum)
    : Item(title, author, year, format, condition), isbn(isbnNum)
{
}

QString FictionBook::typeName() const {
    return "Fiction";
}
