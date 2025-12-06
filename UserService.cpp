#include "UserService.h"
#include "DatabaseManager.h"

// Initializes the service by loading all user data from the database
UserService::UserService() : currentPatronIndex(0) {
    loadUsers();
}

void UserService::loadUsers() {
    patrons = DatabaseManager::instance().loadAllPatrons();
    librarians = DatabaseManager::instance().loadAllLibrarians();
    systemAdmins = DatabaseManager::instance().loadAllSystemAdmins();
}

// Validates the username and returns the users role (Patron, Librarian, SystemAdmin, or Invalid)
Patron* UserService::authenticateUser(const QString& username, QString& role) {
    for (int i = 0; i < patrons.size(); ++i) {
        if (patrons[i].name == username) {
            role = "Patron";
            currentPatronIndex = i;
            return &patrons[i];
        }
    }

    for (const Librarian& l : librarians) {
        if (l.name == username) {
            role = "Librarian";
            int idx = -1;
            for (int i = 0; i < patrons.size(); ++i) {
                if (patrons[i].name == username) {
                    idx = i;
                    break;
                }
            }
            if (idx == -1) {
                patrons.append(Patron(username));
                idx = patrons.size() - 1;
            }
            currentPatronIndex = idx;
            return &patrons[idx];
        }
    }

    for (const SystemAdmin& a : systemAdmins) {
        if (a.name == username) {
            role = "Admin";
            return nullptr;
        }
    }

    role = "Invalid";
    return nullptr;
}

QVector<Patron>& UserService::getPatrons() {
    return patrons;
}

const QVector<Patron>& UserService::getPatrons() const {
    return patrons;
}

QVector<Librarian>& UserService::getLibrarians() {
    return librarians;
}

const QVector<Librarian>& UserService::getLibrarians() const {
    return librarians;
}

QVector<SystemAdmin>& UserService::getSystemAdmins() {
    return systemAdmins;
}

const QVector<SystemAdmin>& UserService::getSystemAdmins() const {
    return systemAdmins;
}

// Returns a pointer to the currently logged in patron for performing library operations
Patron* UserService::getCurrentPatron() {
    if (currentPatronIndex < 0 || currentPatronIndex >= patrons.size()) {
        return nullptr;
    }
    return &patrons[currentPatronIndex];
}

const Patron* UserService::getCurrentPatron() const {
    if (currentPatronIndex < 0 || currentPatronIndex >= patrons.size()) {
        return nullptr;
    }
    return &patrons[currentPatronIndex];
}

void UserService::setCurrentPatronIndex(int index) {
    currentPatronIndex = index;
}

int UserService::getCurrentPatronIndex() const {
    return currentPatronIndex;
}
