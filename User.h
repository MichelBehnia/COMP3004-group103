#ifndef USER_H
#define USER_H

#include <QString>
#include <QVector>
#include <QUuid>

struct Patron {
    QString name;
    QVector<QUuid> activeLoans;
    QVector<QUuid> activeHolds;
    double outstandingFines;

    Patron();
    explicit Patron(const QString& name);
};

struct Librarian {
    QString name;

    Librarian();
    explicit Librarian(const QString& name);
};

struct SystemAdmin {
    QString name;

    SystemAdmin();
    explicit SystemAdmin(const QString& name);
};

struct ActionResult {
    bool ok;
    QString msg;
};

#endif // USER_H
