#ifndef ITEM_H
#define ITEM_H

#include <QString>
#include <QUuid>
#include <QDate>
#include <QVector>

enum class ItemCondition {
    New,
    Standard,
    Worn
};

enum class ItemStatus {
    Available,
    CheckedOut,
    OnHold
};

class Item {
public:
    QUuid itemId;
    QString title;
    QString creator;
    int publicationYear;
    QString format;
    ItemCondition condition;
    ItemStatus status;
    QDate dueDate;
    QVector<QString> holdQueue;

    explicit Item(const QString& title,
                  const QString& creator,
                  int year,
                  const QString& format,
                  ItemCondition condition);

    virtual ~Item();
    virtual QString typeName() const = 0;

    ItemStatus getStatusForPatron(const QString& patronName) const;
};

#endif // ITEM_H
