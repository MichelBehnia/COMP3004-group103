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

// Validates the username and returns the user's role (Patron, Librarian, SystemAdmin, or Invalid)
Patron* UserService::authenticateUser(const QString& username, QString& role) {

    // 1. Check Librarians FIRST — they MUST NOT be treated as patrons
    for (const Librarian& l : librarians) {
        if (l.name == username) {
            role = "Librarian";
            currentPatronIndex = -1;  // librarians are not patrons
            return nullptr;           // return no Patron*
        }
    }

    // 2. Check System Admins
    for (const SystemAdmin& a : systemAdmins) {
        if (a.name == username) {
            role = "Admin";
            currentPatronIndex = -1;
            return nullptr;
        }
    }

    // 3. Check Patrons
    for (int i = 0; i < patrons.size(); ++i) {
        if (patrons[i].name == username) {
            role = "Patron";
            currentPatronIndex = i;
            return &patrons[i];
        }
    }

    // 4. Invalid username
    role = "Invalid";
    currentPatronIndex = -1;
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
