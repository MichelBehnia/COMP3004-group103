#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QUuid>

#include "LibraryService.h"
#include "UserService.h"
#include "LoanService.h"
#include "HoldService.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(LibraryService* libService,
                       UserService* userService,
                       LoanService* loanService,
                       HoldService* holdService,
                       QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Account status entry
    void on_accountStatusButton_clicked();
    void accountStatusActionTriggered();
    void on_backFromAccountButton_clicked();

    //Manage catalogue page entry
    void on_manageCatalogueButton_clicked();
    void on_backFromManageCatalogueButton_clicked();

    // Borrow
    void on_borrowSelectedButton_clicked();

    // Hold
    void on_placeHoldButton_clicked();
    void on_cancelHoldButton_clicked();

    // Return
    void on_unborrowSelectedButton_clicked();
    void unborrowActionTriggered();

    //Librarian functionality (add/remove item from catalogue)
    void on_addItemButton_clicked();
    void on_removeItemButton_clicked();

    // Logout
    void on_logoutButton_clicked();

private:
    Ui::MainWindow *ui;

    // Services
    LibraryService* libraryService;
    UserService* userService;
    LoanService* loanService;
    HoldService* holdService;

    // UI helpers
    QTableWidget* currentTable() const;
    QUuid idForRow(QTableWidget* table, int row) const;

    //Remove item helper
    void removeFromRow(QTableWidget* table, int row);

    // Borrowing helper
    void borrowFromRow(QTableWidget* table, int row);

    // Return helper
    void returnFromRow(QTableWidget* table, int row);

    // Table population
    void setupTableHeaders(QTableWidget* table);
    void populateFictionTable();
    void populateNonFictionTable();
    void populateMagazineTable();
    void populateMovieTable();
    void populateVideoGameTable();
    void refreshAllTables();

    //Manage Catalogue Page
    void showManageCataloguePage();

    // Account page
    void showAccountStatusPage();
    void populateAccountStatus();

    // Utility
    QString conditionToString(ItemCondition c) const;
    QString statusToString(ItemStatus s) const;

    // UI initialization
    void setupConnections();
    void setupLoginHandling();
    void showLoginScreen();
};

#endif // MAINWINDOW_H
