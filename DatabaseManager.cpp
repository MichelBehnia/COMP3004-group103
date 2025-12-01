#include "DatabaseManager.h"
#include "FictionBook.h"
#include "NonFictionBook.h"
#include "Magazine.h"
#include "Movie.h"
#include "VideoGame.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QFile>

DatabaseManager::DatabaseManager() { }

DatabaseManager::~DatabaseManager() { close(); }

// Returns the singleton instance of DatabaseManager to ensure only one database connection exists throughout the application
DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

// Opens or creates the SQLite database, creates tables if needed, and populates default data for a fresh database
bool DatabaseManager::initialize(const QString& dbPath) {
    bool isNewDatabase = !QFile::exists(dbPath);
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    if (!db.open()) return false;
    if (!createTables()) return false;
    if (isNewDatabase) {
        if (!populateDefaultData()) return false;
    }
    return true;
}

void DatabaseManager::close() {
    if (db.isOpen()) db.close();
}

// Creates all required database tables (Items, Patrons, Librarians, SystemAdmins, Loans, Holds) if they don't exist
bool DatabaseManager::createTables() {
    QSqlQuery query;
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS Items ("
        "itemId TEXT PRIMARY KEY, "
        "itemType TEXT NOT NULL, "
        "title TEXT NOT NULL, "
        "creator TEXT NOT NULL, "
        "publicationYear INTEGER NOT NULL, "
        "format TEXT NOT NULL, "
        "condition TEXT NOT NULL, "
        "status TEXT NOT NULL, "
        "dueDate TEXT, "
        "isbn TEXT, "
        "deweyClass TEXT, "
        "issueNumber INTEGER, "
        "publicationDate TEXT, "
        "genre TEXT, "
        "rating INTEGER, "
        "platform TEXT"
        ")")) return false;
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS Patrons ("
        "name TEXT PRIMARY KEY, "
        "outstandingFines REAL NOT NULL DEFAULT 0.0"
        ")")) return false;
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS Librarians ("
        "name TEXT PRIMARY KEY"
        ")")) return false;
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS SystemAdmins ("
        "name TEXT PRIMARY KEY"
        ")")) return false;
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS Loans ("
        "patronName TEXT NOT NULL, "
        "itemId TEXT NOT NULL, "
        "dueDate TEXT NOT NULL, "
        "PRIMARY KEY (patronName, itemId)"
        ")")) return false;
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS Holds ("
        "patronName TEXT NOT NULL, "
        "itemId TEXT NOT NULL, "
        "position INTEGER NOT NULL, "
        "PRIMARY KEY (patronName, itemId)"
        ")")) return false;
    return true;
}

// Pre-seed database with default data
bool DatabaseManager::populateDefaultData() {
    db.transaction();
    QSqlQuery query;
    QStringList patrons = {"michel", "eddie", "joseph", "andrey", "bob"};
    for (const QString& name : patrons) {
        query.prepare("INSERT INTO Patrons (name, outstandingFines) VALUES (:name, 0.0)");
        query.bindValue(":name", name);
        if (!query.exec()) { db.rollback(); return false; }
    }
    query.exec("INSERT INTO Librarians (name) VALUES ('lauren')");
    query.exec("INSERT INTO SystemAdmins (name) VALUES ('adam')");
    QVector<FictionBook*> fictionBooks = {
        new FictionBook("To Kill a Mockingbird", "Harper Lee", 1960, "Hardcover", ItemCondition::Standard, "978-0-06-112008-4"),
        new FictionBook("1984", "George Orwell", 1949, "Paperback", ItemCondition::Worn, "978-0-452-28423-4"),
        new FictionBook("Pride and Prejudice", "Jane Austen", 1813, "Hardcover", ItemCondition::New, "978-0-19-280238-5"),
        new FictionBook("Moby Dick", "Herman Melville", 1851, "Paperback", ItemCondition::Worn, "978-0-14-243724-7"),
        new FictionBook("The Hobbit", "J.R.R. Tolkien", 1937, "Hardcover", ItemCondition::Standard, "978-0-618-00221-3")
    };
    for (FictionBook* book : fictionBooks) { if (!saveItem(book)) { db.rollback(); qDeleteAll(fictionBooks); return false; } delete book; }
    QVector<NonFictionBook*> nonFictionBooks = {
        new NonFictionBook("Sapiens", "Yuval Noah Harari", 2011, "Paperback", ItemCondition::Standard, "978-0-06-231609-7", "909"),
        new NonFictionBook("The Selfish Gene", "Richard Dawkins", 1976, "Paperback", ItemCondition::New, "978-0-19-878860-7", "576"),
        new NonFictionBook("The Immortal Life of Henrietta Lacks", "Rebecca Skloot", 2010, "Hardcover", ItemCondition::Worn, "978-1-4000-5217-2", "616"),
        new NonFictionBook("A Short History of Nearly Everything", "Bill Bryson", 2003, "Paperback", ItemCondition::Standard, "978-0-385-66094-5", "500"),
        new NonFictionBook("Thinking, Fast and Slow", "Daniel Kahneman", 2011, "Hardcover", ItemCondition::Worn, "978-0-374-53355-7", "153.4")
    };
    for (NonFictionBook* book : nonFictionBooks) { if (!saveItem(book)) { db.rollback(); qDeleteAll(nonFictionBooks); return false; } delete book; }
    QVector<Movie*> movies = {
        new Movie("Inception", "Christopher Nolan", 2010, "Blu-ray", ItemCondition::Standard, "Sci-fi", 8),
        new Movie("The Godfather", "Francis Ford Coppola", 1972, "DVD", ItemCondition::New, "Crime", 10),
        new Movie("Forrest Gump", "Robert Zemeckis", 1994, "Blu-ray", ItemCondition::Worn, "Drama", 9)
    };
    for (Movie* movie : movies) { if (!saveItem(movie)) { db.rollback(); qDeleteAll(movies); return false; } delete movie; }
    QVector<VideoGame*> games = {
        new VideoGame("The Legend of Zelda", "Nintendo", 1986, "Cartridge", ItemCondition::Worn, "NES", "Adventure", 9),
        new VideoGame("Super Mario Bros.", "Nintendo", 1985, "Cartridge", ItemCondition::Standard, "NES", "Platformer", 10),
        new VideoGame("Halo: Combat Evolved", "Bungie", 2001, "Disc", ItemCondition::Standard, "Xbox", "Shooter", 9),
        new VideoGame("Minecraft", "Mojang", 2011, "Digital", ItemCondition::New, "PC", "Sandbox", 8)
    };
    for (VideoGame* game : games) { if (!saveItem(game)) { db.rollback(); qDeleteAll(games); return false; } delete game; }
    QVector<Magazine*> magazines = {
        new Magazine("National Geographic", "NG Society", 2023, "Print", ItemCondition::New, 5, QDate(2023, 5, 1)),
        new Magazine("Time", "Time Inc.", 2023, "Print", ItemCondition::Standard, 15, QDate(2023, 4, 15)),
        new Magazine("The Economist", "The Economist Group", 2023, "Print", ItemCondition::Worn, 22, QDate(2023, 5, 8))
    };
    for (Magazine* mag : magazines) { if (!saveItem(mag)) { db.rollback(); qDeleteAll(magazines); return false; } delete mag; }
    db.commit();
    return true;
}

// Inserts a new catalogue item into the database
bool DatabaseManager::saveItem(Item* item) {
    if (!item) return false;
    QSqlQuery query;
    query.prepare(
        "INSERT INTO Items (itemId, itemType, title, creator, publicationYear, format, condition, status, dueDate, "
        "isbn, deweyClass, issueNumber, publicationDate, genre, rating, platform) "
        "VALUES (:itemId, :itemType, :title, :creator, :publicationYear, :format, :condition, :status, :dueDate, "
        ":isbn, :deweyClass, :issueNumber, :publicationDate, :genre, :rating, :platform)"
    );
    query.bindValue(":itemId", item->itemId.toString());
    query.bindValue(":itemType", item->typeName());
    query.bindValue(":title", item->title);
    query.bindValue(":creator", item->creator);
    query.bindValue(":publicationYear", item->publicationYear);
    query.bindValue(":format", item->format);
    QString conditionStr;
    switch (item->condition) {
        case ItemCondition::New: conditionStr = "New"; break;
        case ItemCondition::Standard: conditionStr = "Standard"; break;
        case ItemCondition::Worn: conditionStr = "Worn"; break;
    }
    query.bindValue(":condition", conditionStr);
    QString statusStr;
    switch (item->status) {
        case ItemStatus::Available: statusStr = "Available"; break;
        case ItemStatus::CheckedOut: statusStr = "CheckedOut"; break;
        case ItemStatus::OnHold: statusStr = "OnHold"; break;
    }
    query.bindValue(":status", statusStr);
    query.bindValue(":dueDate", item->dueDate.isValid() ? item->dueDate.toString(Qt::ISODate) : QVariant());
    query.bindValue(":isbn", QVariant());
    query.bindValue(":deweyClass", QVariant());
    query.bindValue(":issueNumber", QVariant());
    query.bindValue(":publicationDate", QVariant());
    query.bindValue(":genre", QVariant());
    query.bindValue(":rating", QVariant());
    query.bindValue(":platform", QVariant());
    if (auto* fb = dynamic_cast<FictionBook*>(item)) {
        query.bindValue(":isbn", fb->isbn);
    } else if (auto* nf = dynamic_cast<NonFictionBook*>(item)) {
        query.bindValue(":isbn", nf->isbn);
        query.bindValue(":deweyClass", nf->deweyClass);
    } else if (auto* mag = dynamic_cast<Magazine*>(item)) {
        query.bindValue(":issueNumber", mag->issueNumber);
        query.bindValue(":publicationDate", mag->publicationDate.toString(Qt::ISODate));
    } else if (auto* mov = dynamic_cast<Movie*>(item)) {
        query.bindValue(":genre", mov->genre);
        query.bindValue(":rating", mov->rating);
    } else if (auto* vg = dynamic_cast<VideoGame*>(item)) {
        query.bindValue(":genre", vg->genre);
        query.bindValue(":platform", vg->platform);
        query.bindValue(":rating", vg->rating);
    }
    return query.exec();
}

// Updates an existing item's status and due date in the database after a borrow or return
bool DatabaseManager::updateItem(Item* item) {
    if (!item) return false;
    QSqlQuery query;
    query.prepare("UPDATE Items SET status = :status, dueDate = :dueDate WHERE itemId = :itemId");
    query.bindValue(":itemId", item->itemId.toString());
    QString statusStr;
    switch (item->status) {
        case ItemStatus::Available: statusStr = "Available"; break;
        case ItemStatus::CheckedOut: statusStr = "CheckedOut"; break;
        case ItemStatus::OnHold: statusStr = "OnHold"; break;
    }
    query.bindValue(":status", statusStr);
    query.bindValue(":dueDate", item->dueDate.isValid() ? item->dueDate.toString(Qt::ISODate) : QVariant());
    return query.exec();
}

// Removes an item from the database
bool DatabaseManager::deleteItem(const QString& itemId) {
    QSqlQuery query;
    query.prepare("DELETE FROM Items WHERE itemId = :itemId");
    query.bindValue(":itemId", itemId);
    return query.exec();
}

// Retrieves a single item from the database
Item* DatabaseManager::loadItemById(const QString& itemId) {
    QSqlQuery query;
    query.prepare("SELECT * FROM Items WHERE itemId = :itemId");
    query.bindValue(":itemId", itemId);
    if (!query.exec() || !query.next()) return nullptr;
    QString itemType = query.value("itemType").toString();
    QString title = query.value("title").toString();
    QString creator = query.value("creator").toString();
    int year = query.value("publicationYear").toInt();
    QString format = query.value("format").toString();
    ItemCondition condition = ItemCondition::Standard;
    QString condStr = query.value("condition").toString();
    if (condStr == "New") condition = ItemCondition::New;
    else if (condStr == "Worn") condition = ItemCondition::Worn;
    Item* item = nullptr;
    if (itemType == "Fiction") {
        QString isbn = query.value("isbn").toString();
        item = new FictionBook(title, creator, year, format, condition, isbn);
    } else if (itemType == "Non-Fiction") {
        QString isbn = query.value("isbn").toString();
        QString dewey = query.value("deweyClass").toString();
        item = new NonFictionBook(title, creator, year, format, condition, isbn, dewey);
    } else if (itemType == "Magazine") {
        int issue = query.value("issueNumber").toInt();
        QDate pubDate = QDate::fromString(query.value("publicationDate").toString(), Qt::ISODate);
        item = new Magazine(title, creator, year, format, condition, issue, pubDate);
    } else if (itemType == "Movie") {
        QString genre = query.value("genre").toString();
        int rating = query.value("rating").toInt();
        item = new Movie(title, creator, year, format, condition, genre, rating);
    } else if (itemType == "Video Game") {
        QString platform = query.value("platform").toString();
        QString genre = query.value("genre").toString();
        int rating = query.value("rating").toInt();
        item = new VideoGame(title, creator, year, format, condition, platform, genre, rating);
    }
    if (item) {
        item->itemId = QUuid(itemId);
        QString statusStr = query.value("status").toString();
        if (statusStr == "CheckedOut") item->status = ItemStatus::CheckedOut;
        else if (statusStr == "OnHold") item->status = ItemStatus::OnHold;
        else item->status = ItemStatus::Available;
        QString dueDateStr = query.value("dueDate").toString();
        if (!dueDateStr.isEmpty()) {
            item->dueDate = QDate::fromString(dueDateStr, Qt::ISODate);
        }
        item->holdQueue = loadHoldQueueForItem(itemId);
    }
    return item;
}

// Loads all catalogue items from the database into memory
QVector<Item*> DatabaseManager::loadAllItems() {
    QVector<Item*> items;
    QSqlQuery query("SELECT itemId FROM Items");
    while (query.next()) {
        Item* item = loadItemById(query.value(0).toString());
        if (item) items.append(item);
    }
    return items;
}

// Loads all patrons from the database along with their active loans and holds
QVector<Patron> DatabaseManager::loadAllPatrons() {
    QVector<Patron> patrons;
    QSqlQuery query("SELECT * FROM Patrons");
    while (query.next()) {
        Patron patron(query.value("name").toString());
        patron.outstandingFines = query.value("outstandingFines").toDouble();
        QSqlQuery loanQuery;
        loanQuery.prepare("SELECT itemId FROM Loans WHERE patronName = :name");
        loanQuery.bindValue(":name", patron.name);
        if (loanQuery.exec()) {
            while (loanQuery.next()) {
                patron.activeLoans.append(QUuid(loanQuery.value(0).toString()));
            }
        }
        QSqlQuery holdQuery;
        holdQuery.prepare("SELECT itemId FROM Holds WHERE patronName = :name");
        holdQuery.bindValue(":name", patron.name);
        if (holdQuery.exec()) {
            while (holdQuery.next()) {
                patron.activeHolds.append(QUuid(holdQuery.value(0).toString()));
            }
        }
        patrons.append(patron);
    }
    return patrons;
}

QVector<Librarian> DatabaseManager::loadAllLibrarians() {
    QVector<Librarian> librarians;
    QSqlQuery query("SELECT * FROM Librarians");
    while (query.next()) {
        librarians.append(Librarian(query.value("name").toString()));
    }
    return librarians;
}

QVector<SystemAdmin> DatabaseManager::loadAllSystemAdmins() {
    QVector<SystemAdmin> admins;
    QSqlQuery query("SELECT * FROM SystemAdmins");
    while (query.next()) {
        admins.append(SystemAdmin(query.value("name").toString()));
    }
    return admins;
}

bool DatabaseManager::updatePatron(const Patron& patron) {
    QSqlQuery query;
    query.prepare("UPDATE Patrons SET outstandingFines = :fines WHERE name = :name");
    query.bindValue(":name", patron.name);
    query.bindValue(":fines", patron.outstandingFines);
    return query.exec();
}

// Records a new loan in the database linking a patron to an item with a due date
bool DatabaseManager::saveLoan(const QString& patronName, const QString& itemId, const QDate& dueDate) {
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO Loans (patronName, itemId, dueDate) VALUES (:patron, :item, :due)");
    query.bindValue(":patron", patronName);
    query.bindValue(":item", itemId);
    query.bindValue(":due", dueDate.toString(Qt::ISODate));
    return query.exec();
}

bool DatabaseManager::deleteLoan(const QString& patronName, const QString& itemId) {
    QSqlQuery query;
    query.prepare("DELETE FROM Loans WHERE patronName = :patron AND itemId = :item");
    query.bindValue(":patron", patronName);
    query.bindValue(":item", itemId);
    return query.exec();
}

// Saves a hold request to the database with the patrons position in the queue
bool DatabaseManager::saveHold(const QString& patronName, const QString& itemId, int position) {
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO Holds (patronName, itemId, position) VALUES (:patron, :item, :pos)");
    query.bindValue(":patron", patronName);
    query.bindValue(":item", itemId);
    query.bindValue(":pos", position);
    return query.exec();
}

bool DatabaseManager::deleteHold(const QString& patronName, const QString& itemId) {
    QSqlQuery query;
    query.prepare("DELETE FROM Holds WHERE patronName = :patron AND itemId = :item");
    query.bindValue(":patron", patronName);
    query.bindValue(":item", itemId);
    return query.exec();
}

// Retrieves the ordered list of patron names waiting for a specific item
QVector<QString> DatabaseManager::loadHoldQueueForItem(const QString& itemId) {
    QVector<QString> queue;
    QSqlQuery query;
    query.prepare("SELECT patronName FROM Holds WHERE itemId = :item ORDER BY position");
    query.bindValue(":item", itemId);
    if (query.exec()) {
        while (query.next()) {
            queue.append(query.value(0).toString());
        }
    }
    return queue;
}

// Recalculates and saves hold queue positions after a hold is cancelled or fulfilled
bool DatabaseManager::updateHoldPositions(const QString& itemId, const QVector<QString>& queue) {
    db.transaction();
    QSqlQuery deleteQuery;
    deleteQuery.prepare("DELETE FROM Holds WHERE itemId = :item");
    deleteQuery.bindValue(":item", itemId);
    if (!deleteQuery.exec()) { db.rollback(); return false; }
    for (int i = 0; i < queue.size(); ++i) {
        if (!saveHold(queue[i], itemId, i)) { db.rollback(); return false; }
    }
    db.commit();
    return true;
}
