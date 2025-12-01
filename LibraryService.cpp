#include "LibraryService.h"
#include "DatabaseManager.h"

// Initializes the library service by loading all items from the database into memory
LibraryService::LibraryService() {
    catalogue = DatabaseManager::instance().loadAllItems();
}

LibraryService::~LibraryService() {
    for (Item* item : catalogue) {
        delete item;
    }
    catalogue.clear();
}

// Searches the catalogue for an item matching the given UUID
Item* LibraryService::findItemById(const QUuid& id) {
    for (Item* it : catalogue) {
        if (it->itemId == id) return it;
    }
    return nullptr;
}

const Item* LibraryService::findItemById(const QUuid& id) const {
    for (Item* it : catalogue) {
        if (it->itemId == id) return it;
    }
    return nullptr;
}

QVector<Item*> LibraryService::getAllItems() const {
    return catalogue;
}

// Filters and returns all items of a specific type
QVector<Item*> LibraryService::getItemsByType(const QString& typeName) const {
    QVector<Item*> result;
    for (Item* item : catalogue) {
        if (item->typeName() == typeName) {
            result.append(item);
        }
    }
    return result;
}

// Adds a new item to both the catalogue and the database
void LibraryService::addItem(Item* item) {
    if (item) {
        catalogue.append(item);
        DatabaseManager::instance().saveItem(item);
    }
}

// Removes an item from the catalogue and database
bool LibraryService::removeItem(const QUuid& id) {
    for (int i = 0; i < catalogue.size(); ++i) {
        if (catalogue[i]->itemId == id) {
            DatabaseManager::instance().deleteItem(id.toString());
            delete catalogue[i];
            catalogue.remove(i);
            return true;
        }
    }
    return false;
}

// Clears and reloads the entire catalogue from the database to sync with any external changes
void LibraryService::reloadCatalogue() {
    for (Item* item : catalogue) {
        delete item;
    }
    catalogue.clear();
    catalogue = DatabaseManager::instance().loadAllItems();
}
