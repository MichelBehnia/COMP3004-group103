#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>
#include <QVector>
#include "Item.h"
#include "User.h"

class DatabaseManager {
public:
    static DatabaseManager& instance();

    bool initialize(const QString& dbPath = "hinlibs.sqlite3");
    void close();

    QVector<Item*> loadAllItems();
    bool saveItem(Item* item);
    bool updateItem(Item* item);
    bool deleteItem(const QString& itemId);
    Item* loadItemById(const QString& itemId);

    QVector<Patron> loadAllPatrons();
    QVector<Librarian> loadAllLibrarians();
    QVector<SystemAdmin> loadAllSystemAdmins();
    bool updatePatron(const Patron& patron);

    bool saveLoan(const QString& patronName, const QString& itemId, const QDate& dueDate);
    bool deleteLoan(const QString& patronName, const QString& itemId);

    bool saveHold(const QString& patronName, const QString& itemId, int position);
    bool deleteHold(const QString& patronName, const QString& itemId);
    QVector<QString> loadHoldQueueForItem(const QString& itemId);
    bool updateHoldPositions(const QString& itemId, const QVector<QString>& queue);

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool createTables();
    bool populateDefaultData();

    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
