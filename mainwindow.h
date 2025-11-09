#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QDate>
#include <QUuid>

enum class ItemCondition {
    New,
    Standard,
    Worn
};

class Item {
public:
    QUuid itemId;
    QString title;
    QString creator;
    int publicationYear;
    QString format;
    ItemCondition condition;

    Item(const QString &title,
         const QString &creator,
         int pubYear,
         const QString &format,
         ItemCondition cond)
        : itemId(QUuid::createUuid()),
          title(title),
          creator(creator),
          publicationYear(pubYear),
          format(format),
          condition(cond) {}

    virtual ~Item() = default;
};

class FictionBook : public Item {
public:
    QString ISBN;
    FictionBook(const QString &title, const QString &creator, int pubYear, const QString &format,
                ItemCondition cond, const QString &isbn)
        : Item(title, creator, pubYear, format, cond), ISBN(isbn) {}
};

class NonFictionBook : public Item {
public:
    QString ISBN;
    QString deweyDecimal;
    NonFictionBook(const QString &title, const QString &creator, int pubYear, const QString &format,
                   ItemCondition cond, const QString &isbn, const QString &dewey)
        : Item(title, creator, pubYear, format, cond), ISBN(isbn), deweyDecimal(dewey) {}
};

class Magazine : public Item {
public:
    int issueNumber;
    QDate publicationDate;
    Magazine(const QString &title, const QString &creator, int pubYear, const QString &format,
             ItemCondition cond, int issue, const QDate &pubDate)
        : Item(title, creator, pubYear, format, cond),
          issueNumber(issue), publicationDate(pubDate) {}
};

class Movie : public Item {
public:
    QString genre;
    int rating;
    Movie(const QString &title, const QString &creator, int pubYear, const QString &format,
          ItemCondition cond, const QString &genre, int rating)
        : Item(title, creator, pubYear, format, cond), genre(genre), rating(rating) {}
};

class VideoGame : public Item {
public:
    QString genre;
    int rating;
    VideoGame(const QString &title, const QString &creator, int pubYear, const QString &format,
              ItemCondition cond, const QString &genre, int rating)
        : Item(title, creator, pubYear, format, cond), genre(genre), rating(rating) {}
};

class Patron {
public:
    explicit Patron(const QString &name) : username(name) {}
    QString username;
};

class Librarian {
public:
    explicit Librarian(const QString &name) : username(name) {}
    QString username;
};

class SystemAdmin {
public:
    explicit SystemAdmin(const QString &name) : username(name) {}
    QString username;
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoginButtonClicked();

private:
    Ui::MainWindow *ui;

    QVector<Patron> patrons;
    QVector<Librarian> librarians;
    QVector<SystemAdmin> systemAdmins;
    QVector<Item*> catalogue;

    void populateFictionTable();
    void populateNonFictionTable();
    void populateMagazineTable();
    void populateMovieTable();
    void populateVideoGameTable();
};

#endif // MAINWINDOW_H
