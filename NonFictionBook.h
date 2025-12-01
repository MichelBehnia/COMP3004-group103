#ifndef NONFICTIONBOOK_H
#define NONFICTIONBOOK_H

#include "Item.h"

class NonFictionBook : public Item {
public:
    QString isbn;
    QString deweyClass;

    NonFictionBook(const QString& title,
                   const QString& author,
                   int year,
                   const QString& format,
                   ItemCondition condition,
                   const QString& isbnNum,
                   const QString& deweyClass);

    QString typeName() const override;
};

#endif // NONFICTIONBOOK_H
