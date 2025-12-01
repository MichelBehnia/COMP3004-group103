#include "User.h"

// Patron implementation
Patron::Patron() : outstandingFines(0.0) {
}

Patron::Patron(const QString& name) : name(name), outstandingFines(0.0) {
}

// Librarian implementation
Librarian::Librarian() {
}

Librarian::Librarian(const QString& name) : name(name) {
}

// SystemAdmin implementation
SystemAdmin::SystemAdmin() {
}

SystemAdmin::SystemAdmin(const QString& name) : name(name) {
}
