#ifndef LIBRARYSERVICE_H
#define LIBRARYSERVICE_H

#include "Item.h"
#include "FictionBook.h"
#include "NonFictionBook.h"
#include "Magazine.h"
#include "Movie.h"
#include "VideoGame.h"
#include <QVector>
#include <QUuid>

class LibraryService {
public:
    LibraryService();
    ~LibraryService();

    Item* findItemById(const QUuid& id);
    const Item* findItemById(const QUuid& id) const;
    QVector<Item*> getAllItems() const;
    QVector<Item*> getItemsByType(const QString& typeName) const;

    void addItem(Item* item);
    bool removeItem(const QUuid& id);
    void reloadCatalogue();

private:
    QVector<Item*> catalogue;
};

#endif // LIBRARYSERVICE_H
