#include "mainwindow.h"
#include <QApplication>
#include "DatabaseManager.h"
#include "LibraryService.h"
#include "UserService.h"
#include "LoanService.h"
#include "HoldService.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!DatabaseManager::instance().initialize()) {
        return -1;
    }

    LibraryService libraryService;
    UserService userService;
    LoanService loanService(&libraryService);
    HoldService holdService(&libraryService, &loanService);

    MainWindow w(&libraryService, &userService, &loanService, &holdService);
    w.show();

    return a.exec();
}
