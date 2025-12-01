#ifndef MAGAZINE_H
#define MAGAZINE_H

#include "Item.h"

class Magazine : public Item {
public:
    int issueNumber;
    QDate publicationDate;

    Magazine(const QString& title,
             const QString& publisher,
             int year,
             const QString& format,
             ItemCondition condition,
             int issueNum,
             const QDate& pubDate);

    QString typeName() const override;
};

#endif // MAGAZINE_H
