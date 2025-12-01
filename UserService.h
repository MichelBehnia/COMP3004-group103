#ifndef USERSERVICE_H
#define USERSERVICE_H

#include "User.h"
#include <QVector>
#include <QString>

class UserService {
public:
    UserService();

    Patron* authenticateUser(const QString& username, QString& role);

    QVector<Patron>& getPatrons();
    const QVector<Patron>& getPatrons() const;
    QVector<Librarian>& getLibrarians();
    const QVector<Librarian>& getLibrarians() const;
    QVector<SystemAdmin>& getSystemAdmins();
    const QVector<SystemAdmin>& getSystemAdmins() const;

    Patron* getCurrentPatron();
    const Patron* getCurrentPatron() const;
    void setCurrentPatronIndex(int index);
    int getCurrentPatronIndex() const;

private:
    QVector<Patron> patrons;
    QVector<Librarian> librarians;
    QVector<SystemAdmin> systemAdmins;
    int currentPatronIndex;

    void loadUsers();
};

#endif // USERSERVICE_H
