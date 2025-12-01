#ifndef FICTIONBOOK_H
#define FICTIONBOOK_H

#include "Item.h"

class FictionBook : public Item {
public:
    QString isbn;

    FictionBook(const QString& title,
                const QString& author,
                int year,
                const QString& format,
                ItemCondition condition,
                const QString& isbnNum);

    QString typeName() const override;
};

#endif // FICTIONBOOK_H
