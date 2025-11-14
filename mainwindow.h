#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QDate>
#include <QUuid>
#include <QTableWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

//domain model
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
    QString creator; //author or director etc
    int publicationYear = 0;
    QString format; //format like hardcover paperback dvd
    ItemCondition condition = ItemCondition::Standard;
    ItemStatus status = ItemStatus::Available;
    QDate dueDate; //set on checkout
    QVector<QString> holdQueue;  // usernames waiting for this item

    explicit Item(const QString& t,
                  const QString& c,
                  int y,
                  const QString& f,
                  ItemCondition cond)
        : itemId(QUuid::createUuid()),
          title(t), creator(c), publicationYear(y), format(f), condition(cond) {}

    virtual ~Item() = default;

    //type name for tables
    virtual QString typeName() const = 0;
};

class FictionBook : public Item {
public:
    QString isbn;
    FictionBook(const QString& t, const QString& a, int y, const QString& f,
                ItemCondition cond, const QString& isbn)
        : Item(t, a, y, f, cond), isbn(isbn) {}
    QString typeName() const override { return "Fiction"; }
};

class NonFictionBook : public Item {
public:
    QString isbn;
    QString deweyClass; //ex 153.4
    NonFictionBook(const QString& t, const QString& a, int y, const QString& f,
                   ItemCondition cond, const QString& isbn, const QString& cls)
        : Item(t, a, y, f, cond), isbn(isbn), deweyClass(cls) {}
    QString typeName() const override { return "Non-Fiction"; }
};

class Magazine : public Item {
public:
    int issueNumber = 0;
    QDate publicationDate;
    Magazine(const QString& t, const QString& a, int y, const QString& f,
             ItemCondition cond, int issue, const QDate& pubDate)
        : Item(t, a, y, f, cond), issueNumber(issue), publicationDate(pubDate) {}
    QString typeName() const override { return "Magazine"; }
};

class Movie : public Item {
public:
    QString genre;
    int rating = 0; //rating scale
    Movie(const QString& t, const QString& a, int y, const QString& f,
          ItemCondition cond, const QString& g, int r)
        : Item(t, a, y, f, cond), genre(g), rating(r) {}
    QString typeName() const override { return "Movie"; }
};

class VideoGame : public Item {
public:
    QString platform; //pc ps5 switch etc
    QString genre;
    VideoGame(const QString& t, const QString& a, int y, const QString& f,
              ItemCondition cond, const QString& plat, const QString& g)
        : Item(t, a, y, f, cond), platform(plat), genre(g) {}
    QString typeName() const override { return "Video Game"; }
};

struct Patron {
    QString name;
    QVector<QUuid> activeLoans;
    QVector<QUuid> activeHolds;
    double outstandingFines = 0.0;

    Patron() = default;
    explicit Patron(const QString& n) : name(n) {}
};

struct Librarian {
    QString name;
    explicit Librarian(const QString& n = {}) : name(n) {}
};

struct SystemAdmin {
    QString name;
    explicit SystemAdmin(const QString& n = {}) : name(n) {}
};

//result for actions
struct ActionResult {
    bool ok;
    QString msg;
};

//main window
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    //account status entry
    void on_accountStatusButton_clicked(); //button
    void accountStatusActionTriggered(); //menu or toolbar
    void on_backFromAccountButton_clicked(); //back button

    //borrow
    void on_borrowSelectedButton_clicked();

    //hold
    void on_placeHoldButton_clicked();
    void on_cancelHoldButton_clicked();


    //return
    void on_unborrowSelectedButton_clicked();
    void unborrowActionTriggered();

    //logout
    void on_logoutButton_clicked();

private:
    Ui::MainWindow *ui;

    //mock db
    QVector<Patron> patrons;
    QVector<Librarian> librarians;
    QVector<SystemAdmin> systemAdmins;
    QVector<Item*> catalogue;

    //current patron index
    int currentPatronIndex = 0;

    //borrowing helpers
    QTableWidget* currentTable() const; //table for current page
    QUuid idForRow(QTableWidget* table, int row) const; //read id from row

    void borrowFromRow(QTableWidget* table, int row);
    bool borrowById(const QUuid& id, QString* err);
    void hookDoubleClickBorrow();

    //hold helpers
    bool placeHoldById(const QUuid& id, QString* err);
    bool cancelHoldById(const QUuid& id, QString* err);

    //return helpers
    void returnFromRow(QTableWidget* table, int row);
    bool returnById(const QUuid& id, QString* err);

    //shared helpers
    void setupTableHeaders(QTableWidget* table);
    void addRowForItem(QTableWidget* table, Item* item);
    void refreshRow(QTableWidget* table, int row, Item* item);
    bool patronHasLoan(const Patron& p, const QUuid& id) const;
    bool patronHasHold(const Patron& p, const QUuid& id) const;

    //page population
    void populateFictionTable();
    void populateNonFictionTable();
    void populateMagazineTable();
    void populateMovieTable();
    void populateVideoGameTable();

    //account page
    void showAccountStatusPage(); //switch to account page
    void populateAccountStatus(); //fill account info

    //small helpers
    Patron* currentPatron();
    const Patron* currentPatron() const;
    QString conditionToString(ItemCondition c) const;
    QString statusToString(ItemStatus s) const;
};

#endif //mainwindow_h
