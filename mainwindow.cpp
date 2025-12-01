#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QStackedWidget>
#include <QHeaderView>
#include <QAction>
#include <algorithm>

// Helper function to get child widgets by name
template<typename T>
T* get(QObject* root, const char* name) {
    return root ? root->findChild<T*>(name) : nullptr;
}

// Utility functions for enum to string conversion
static QString condToString(ItemCondition c) {
    switch (c) {
        case ItemCondition::New: return "New";
        case ItemCondition::Standard: return "Standard";
        case ItemCondition::Worn: return "Worn";
    }
    return "Standard";
}

static QString statToString(ItemStatus s) {
    switch (s) {
        case ItemStatus::Available: return "Available";
        case ItemStatus::CheckedOut: return "Checked Out";
        case ItemStatus::OnHold: return "On Hold";
    }
    return "Available";
}

MainWindow::MainWindow(LibraryService* libService,
                       UserService* userService,
                       LoanService* loanService,
                       HoldService* holdService,
                       QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      libraryService(libService),
      userService(userService),
      loanService(loanService),
      holdService(holdService)
{
    Q_ASSERT(libraryService != nullptr);
    Q_ASSERT(userService != nullptr);
    Q_ASSERT(loanService != nullptr);
    Q_ASSERT(holdService != nullptr);

    ui->setupUi(this);
    setupConnections();
    showLoginScreen();
}
MainWindow::~MainWindow() {
    delete ui;
}

// Connects all UI buttons and actions to their corresponding slot functions for user interaction
void MainWindow::setupConnections() {
    // Account page unborrow button
    connect(ui->unborrowSelectedButton_account, &QPushButton::clicked,
            this, &MainWindow::on_unborrowSelectedButton_clicked);
    connect(ui->cancelHoldButton_account, &QPushButton::clicked,
            this, &MainWindow::on_cancelHoldButton_clicked);

    // Account status hooks
    if (auto* b = get<QPushButton>(this, "accountStatusButton"))
        connect(b, &QPushButton::clicked, this, &MainWindow::on_accountStatusButton_clicked, Qt::UniqueConnection);
    if (auto* a = get<QAction>(this, "actionAccount_Status"))
        connect(a, &QAction::triggered, this, &MainWindow::accountStatusActionTriggered, Qt::UniqueConnection);
    if (auto* a = get<QAction>(this, "actionUnborrow"))
        connect(a, &QAction::triggered, this, &MainWindow::unborrowActionTriggered, Qt::UniqueConnection);
    if (auto* b = get<QPushButton>(this, "backFromAccountButton"))
        connect(b, &QPushButton::clicked, this, &MainWindow::on_backFromAccountButton_clicked, Qt::UniqueConnection);

    // Page navigation buttons
    auto* stacked = get<QStackedWidget>(this, "stackedWidget");
    if (auto* b = get<QPushButton>(this, "fictionButton"))
        connect(b, &QPushButton::clicked, this, [ stacked]() {
            if (stacked) stacked->setCurrentIndex(1);
        }, Qt::UniqueConnection);
    if (auto* b = get<QPushButton>(this, "nonFictionButton"))
        connect(b, &QPushButton::clicked, this, [ stacked]() {
            if (stacked) stacked->setCurrentIndex(2);
        }, Qt::UniqueConnection);
    if (auto* b = get<QPushButton>(this, "magazineButton"))
        connect(b, &QPushButton::clicked, this, [ stacked]() {
            if (stacked) stacked->setCurrentIndex(3);
        }, Qt::UniqueConnection);
    if (auto* b = get<QPushButton>(this, "movieButton"))
        connect(b, &QPushButton::clicked, this, [ stacked]() {
            if (stacked) stacked->setCurrentIndex(4);
        }, Qt::UniqueConnection);
    if (auto* b = get<QPushButton>(this, "videoGameButton"))
        connect(b, &QPushButton::clicked, this, [ stacked]() {
            if (stacked) stacked->setCurrentIndex(5);
        }, Qt::UniqueConnection);

    setupLoginHandling();
}

// Configures the login button to authenticate users and display appropriate UI based on their role
void MainWindow::setupLoginHandling() {
    auto* stacked = get<QStackedWidget>(this, "stackedWidget");

    if (auto* loginBtn = get<QPushButton>(this, "loginButton")) {
        connect(loginBtn, &QPushButton::clicked, this, [this, stacked]() {
            if (!userService) {
                return;
            }

            const auto* userEdit = get<QLineEdit>(this, "usernameLineEdit");
            const QString username = userEdit ? userEdit->text().trimmed() : QString();

            QString role;
            userService->authenticateUser(username, role);

            auto* roleLbl = get<QLabel>(this, "userRoleLabel");

            if (role == "Invalid") {
                if (roleLbl) roleLbl->setText("Invalid username");
                return;
            }

            QString displayRole = role;
            if (role == "Librarian") displayRole = "Librarian / Patron";
            if (roleLbl) roleLbl->setText(username + ": " + displayRole);

            auto* accountButtonsBox = get<QGroupBox>(this, "accountButtonsGroupBox");
            auto* actionButtonsBox = get<QGroupBox>(this, "actionButtonsGroupBox");
            auto* tableButtonsBox = get<QGroupBox>(this, "tableButtonGroupBox");
            auto* accountStatusBtn = get<QPushButton>(this, "accountStatusButton");

            // Admin: only show logout
            if (role == "Admin") {
                if (auto* loginBox = get<QGroupBox>(this, "loginGroupBox"))
                    loginBox->hide();
                if (accountButtonsBox) accountButtonsBox->show();
                if (actionButtonsBox) actionButtonsBox->hide();
                if (tableButtonsBox) tableButtonsBox->hide();
                if (accountStatusBtn) accountStatusBtn->setVisible(false);
                if (stacked) stacked->setCurrentIndex(0);
                statusBar()->showMessage("Admin features coming in D2.", 3000);
                return;
            }

            // Patron/Librarian: show full functionality
            if (accountStatusBtn) accountStatusBtn->setVisible(true);
            if (accountButtonsBox) accountButtonsBox->show();
            if (actionButtonsBox) actionButtonsBox->show();
            if (tableButtonsBox) tableButtonsBox->show();

            refreshAllTables();

            if (stacked) stacked->setCurrentIndex(1);
        }, Qt::UniqueConnection);
    }
}

// Resets the UI to the initial login state by hiding authenticated user controls
void MainWindow::showLoginScreen() {
    auto* accountButtonsBox = get<QGroupBox>(this, "accountButtonsGroupBox");
    auto* actionButtonsBox = get<QGroupBox>(this, "actionButtonsGroupBox");
    auto* tableButtonsBox = get<QGroupBox>(this, "tableButtonGroupBox");

    if (accountButtonsBox) accountButtonsBox->hide();
    if (actionButtonsBox) actionButtonsBox->hide();
    if (tableButtonsBox) tableButtonsBox->hide();

    auto* stacked = get<QStackedWidget>(this, "stackedWidget");
    if (stacked) stacked->setCurrentIndex(0);
}

// Account Status Slots
void MainWindow::on_accountStatusButton_clicked() {
    showAccountStatusPage();
    auto* actionButtonsBox = get<QGroupBox>(this, "actionButtonsGroupBox");
    auto* tableButtonsBox = get<QGroupBox>(this, "tableButtonGroupBox");
    if (actionButtonsBox) actionButtonsBox->hide();
    if (tableButtonsBox) tableButtonsBox->hide();
}

void MainWindow::accountStatusActionTriggered() {
    showAccountStatusPage();
}

void MainWindow::on_backFromAccountButton_clicked() {
    if (auto* stacked = get<QStackedWidget>(this, "stackedWidget"))
        stacked->setCurrentIndex(1);
    auto* actionButtonsBox = get<QGroupBox>(this, "actionButtonsGroupBox");
    auto* tableButtonsBox = get<QGroupBox>(this, "tableButtonGroupBox");
    if (actionButtonsBox) actionButtonsBox->show();
    if (tableButtonsBox) tableButtonsBox->show();
}

// Borrow button slot
void MainWindow::on_borrowSelectedButton_clicked() {
    if (QTableWidget* t = currentTable()) {
        int row = t->currentRow();
        if (row >= 0) borrowFromRow(t, row);
    }
}

// Unborrow button slot
void MainWindow::on_unborrowSelectedButton_clicked() {
    QTableWidget* t = currentTable();
    auto* stacked = get<QStackedWidget>(this, "stackedWidget");
    QWidget* accPage = get<QWidget>(this, "accountPage");

    if (!t && stacked && accPage && stacked->currentWidget() == accPage) {
        t = get<QTableWidget>(this, "loansTableWidget");
    }

    if (!t) return;
    int row = t->currentRow();
    if (row >= 0) {
        returnFromRow(t, row);
    }
}

// Unborrow action slot
void MainWindow::unborrowActionTriggered() {
    QTableWidget* t = currentTable();
    auto* stacked = get<QStackedWidget>(this, "stackedWidget");
    QWidget* accPage = get<QWidget>(this, "accountPage");

    if (!t && stacked && accPage && stacked->currentWidget() == accPage) {
        t = get<QTableWidget>(this, "loansTableWidget");
    }

    if (!t) return;
    int row = t->currentRow();
    if (row >= 0) {
        returnFromRow(t, row);
    }
}

// Place hold button slot
void MainWindow::on_placeHoldButton_clicked() {
    if (QTableWidget* t = currentTable()) {
        int row = t->currentRow();
        if (row < 0) return;
        const QUuid id = idForRow(t, row);

        Patron* patron = userService->getCurrentPatron();
        ActionResult result = holdService->placeHold(patron, id);

        populateAccountStatus();
        statusBar()->showMessage(result.msg, 3000);
    }
}

// Cancel hold button slot
void MainWindow::on_cancelHoldButton_clicked() {
    auto* stacked = get<QStackedWidget>(this, "stackedWidget");
    QWidget* accPage = get<QWidget>(this, "accountPage");

    QTableWidget* t = nullptr;

    if (stacked && accPage && stacked->currentWidget() == accPage) {
        t = get<QTableWidget>(this, "holdsTableWidget");
    } else {
        t = currentTable();
    }

    if (!t) return;
    int row = t->currentRow();
    if (row < 0) return;
    const QUuid id = idForRow(t, row);
    if (id.isNull()) return;

    Patron* patron = userService->getCurrentPatron();
    ActionResult result = holdService->cancelHold(patron, id);

    populateAccountStatus();
    refreshAllTables();
    statusBar()->showMessage(result.msg, 3000);
}

// Logout slot
void MainWindow::on_logoutButton_clicked() {
    auto* stacked = get<QStackedWidget>(this, "stackedWidget");
    auto* tableButtons = get<QGroupBox>(this, "tableButtonGroupBox");
    auto* roleLbl = get<QLabel>(this, "userRoleLabel");
    auto* usernameField = get<QLineEdit>(this, "usernameLineEdit");
    auto* accountButtonsBox = get<QGroupBox>(this, "accountButtonsGroupBox");
    auto* actionButtonsBox = get<QGroupBox>(this, "actionButtonsGroupBox");
    auto* loginBox = get<QGroupBox>(this, "loginGroupBox");

    if (accountButtonsBox) accountButtonsBox->hide();
    if (actionButtonsBox) actionButtonsBox->hide();
    if (tableButtons) tableButtons->hide();

    if (loginBox) loginBox->show();
    if (stacked) stacked->setCurrentIndex(0);
    if (roleLbl) roleLbl->setText("Logged out");
    if (usernameField) usernameField->clear();
}

// Returns the currently visible table widget based on which category page is displayed
QTableWidget* MainWindow::currentTable() const {
    auto* stacked = get<QStackedWidget>(const_cast<MainWindow*>(this), "stackedWidget");
    if (!stacked) return nullptr;
    const int idx = stacked->currentIndex();
    switch (idx) {
        case 1: return get<QTableWidget>(const_cast<MainWindow*>(this), "fictionTableWidget");
        case 2: return get<QTableWidget>(const_cast<MainWindow*>(this), "nonFictionTableWidget");
        case 3: return get<QTableWidget>(const_cast<MainWindow*>(this), "magazineTableWidget");
        case 4: return get<QTableWidget>(const_cast<MainWindow*>(this), "movieTableWidget");
        case 5: return get<QTableWidget>(const_cast<MainWindow*>(this), "videoGameTableWidget");
        default: return nullptr;
    }
}

// Gets the item UUID stored in the first column of a table row
QUuid MainWindow::idForRow(QTableWidget* table, int row) const {
    if (!table || row < 0 || row >= table->rowCount()) return {};
    QTableWidgetItem* it = table->item(row, 0);
    if (!it) return {};
    const QString idStr = it->data(Qt::UserRole).toString();
    return QUuid(idStr);
}

// Initiates a borrow action for the item at the specified table row
void MainWindow::borrowFromRow(QTableWidget* table, int row) {
    if (!table) return;

    const QUuid id = idForRow(table, row);
    if (id.isNull()) return;

    Patron* patron = userService->getCurrentPatron();
    ActionResult result = loanService->borrowItem(patron, id);

    refreshAllTables();
    populateAccountStatus();

    if (!result.msg.isEmpty()) statusBar()->showMessage(result.msg, 3000);
    if (row >= 0 && row < table->rowCount()) table->setCurrentCell(row, 0);
}

// Processes a return action for the item at the specified table row
void MainWindow::returnFromRow(QTableWidget* table, int row) {
    if (!table) return;

    const QUuid id = idForRow(table, row);
    if (id.isNull()) return;

    Patron* patron = userService->getCurrentPatron();
    ActionResult result = loanService->returnItem(patron, id);

    refreshAllTables();
    populateAccountStatus();

    if (!result.msg.isEmpty()) statusBar()->showMessage(result.msg, 3000);
    if (row >= 0 && row < table->rowCount())
        table->setCurrentCell(std::min(row, table->rowCount() - 1), 0);
}

// Navigates to the account status page showing the patrons loans, holds, and fines
void MainWindow::showAccountStatusPage() {
    populateAccountStatus();
    auto* stacked = get<QStackedWidget>(this, "stackedWidget");
    QWidget* accPage = get<QWidget>(this, "accountPage");
    if (stacked && accPage) {
        int idx = stacked->indexOf(accPage);
        if (idx >= 0) stacked->setCurrentIndex(idx);
    }
}

// Fills the account status tables with the current patrons active loans and holds
void MainWindow::populateAccountStatus() {
    const Patron* p = userService->getCurrentPatron();
    if (!p) return;

    if (auto* lbl = get<QLabel>(this, "accountNameLabel"))
        lbl->setText(p->name);

    // Role text
    QString roleText = "Patron";
    for (const Librarian& l : userService->getLibrarians()) {
        if (l.name == p->name) {
            roleText = "Librarian / Patron";
            break;
        }
    }

    if (auto* role = get<QLabel>(this, "accountRoleLabel"))
        role->setText(roleText);
    else if (auto* role2 = get<QLabel>(this, "accountTitleLabel"))
        role2->setText(roleText);

    if (auto* fines = get<QLabel>(this, "finesValueLabel"))
        fines->setText(QString("$%1").arg(QString::number(p->outstandingFines, 'f', 2)));

    // Loans table
    if (auto* t = get<QTableWidget>(this, "loansTableWidget")) {
        t->clear();
        t->setRowCount(0);
        t->setColumnCount(5);
        t->setHorizontalHeaderLabels({"Title", "Type", "Status", "Due", "Condition"});

        int row = 0;
        for (const QUuid& id : p->activeLoans) {
            if (Item* it = libraryService->findItemById(id)) {
                t->insertRow(row);

                auto* titleItem = new QTableWidgetItem(it->title);
                titleItem->setData(Qt::UserRole, it->itemId.toString());
                t->setItem(row, 0, titleItem);
                t->setItem(row, 1, new QTableWidgetItem(it->typeName()));
                t->setItem(row, 2, new QTableWidgetItem(statToString(it->status)));

                QString dueText = "-";
                if (it->dueDate.isValid()) {
                    int days = QDate::currentDate().daysTo(it->dueDate);
                    if (days >= 0) {
                        dueText = QString("%1 (%2 days)")
                                    .arg(it->dueDate.toString())
                                    .arg(days);
                    } else {
                        dueText = QString("%1 (overdue)").arg(it->dueDate.toString());
                    }
                }
                t->setItem(row, 3, new QTableWidgetItem(dueText));
                t->setItem(row, 4, new QTableWidgetItem(condToString(it->condition)));
                ++row;
            }
        }
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }

    // Holds table
    if (auto* t = get<QTableWidget>(this, "holdsTableWidget")) {
        t->clear();
        t->setRowCount(0);
        t->setColumnCount(4);
        t->setHorizontalHeaderLabels({"Title", "Type", "Status", "Queue Position"});

        int row = 0;
        for (const QUuid& id : p->activeHolds) {
            if (Item* it = libraryService->findItemById(id)) {
                t->insertRow(row);

                auto* titleItem = new QTableWidgetItem(it->title);
                titleItem->setData(Qt::UserRole, it->itemId.toString());
                t->setItem(row, 0, titleItem);
                t->setItem(row, 1, new QTableWidgetItem(it->typeName()));
                t->setItem(row, 2, new QTableWidgetItem(statToString(it->status)));

                // Queue position
                int pos = holdService->getQueuePosition(*p, it);
                QString posText = (pos > 0) ? QString::number(pos) : "-";
                t->setItem(row, 3, new QTableWidgetItem(posText));
                ++row;
            }
        }
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

// Utility functions
QString MainWindow::conditionToString(ItemCondition c) const {
    return condToString(c);
}

QString MainWindow::statusToString(ItemStatus s) const {
    return statToString(s);
}

// Reloads all category tables to reflect the current state of the catalogue
void MainWindow::refreshAllTables() {
    populateFictionTable();
    populateNonFictionTable();
    populateMagazineTable();
    populateMovieTable();
    populateVideoGameTable();
}

// Populate Fiction Table
void MainWindow::populateFictionTable() {
    if (auto* t = get<QTableWidget>(this, "fictionTableWidget")) {
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->clear();
        t->setColumnCount(7);
        t->setHorizontalHeaderLabels({"Title", "Author", "Year", "Format", "Condition", "ISBN", "Availability"});
        t->setRowCount(0);

        // Get current patron name
        const Patron* currentP = userService->getCurrentPatron();
        QString patronName = currentP ? currentP->name : QString();

        int row = 0;
        QVector<Item*> items = libraryService->getAllItems();
        for (Item* item : items) {
            if (auto* fb = dynamic_cast<FictionBook*>(item)) {
                t->insertRow(row);

                auto* titleItem = new QTableWidgetItem(fb->title);
                titleItem->setData(Qt::UserRole, fb->itemId.toString());
                t->setItem(row, 0, titleItem);
                t->setItem(row, 1, new QTableWidgetItem(fb->creator));
                t->setItem(row, 2, new QTableWidgetItem(QString::number(fb->publicationYear)));
                t->setItem(row, 3, new QTableWidgetItem(fb->format));
                t->setItem(row, 4, new QTableWidgetItem(condToString(fb->condition)));
                t->setItem(row, 5, new QTableWidgetItem(fb->isbn));

                // USE PATRON-SPECIFIC STATUS
                ItemStatus displayStatus = fb->getStatusForPatron(patronName);
                t->setItem(row, 6, new QTableWidgetItem(statToString(displayStatus)));

                ++row;
            }
        }
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

// Populate Non-Fiction Table
void MainWindow::populateNonFictionTable() {
    if (auto* t = get<QTableWidget>(this, "nonFictionTableWidget")) {
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->clear();
        t->setColumnCount(8);
        t->setHorizontalHeaderLabels({"Title", "Author", "Year", "Format", "Condition", "ISBN", "Dewey", "Availability"});
        t->setRowCount(0);

        // Get current patron name
        const Patron* currentP = userService->getCurrentPatron();
        QString patronName = currentP ? currentP->name : QString();

        int row = 0;
        QVector<Item*> items = libraryService->getAllItems();
        for (Item* item : items) {
            if (auto* nf = dynamic_cast<NonFictionBook*>(item)) {
                t->insertRow(row);

                auto* titleItem = new QTableWidgetItem(nf->title);
                titleItem->setData(Qt::UserRole, nf->itemId.toString());
                t->setItem(row, 0, titleItem);
                t->setItem(row, 1, new QTableWidgetItem(nf->creator));
                t->setItem(row, 2, new QTableWidgetItem(QString::number(nf->publicationYear)));
                t->setItem(row, 3, new QTableWidgetItem(nf->format));
                t->setItem(row, 4, new QTableWidgetItem(condToString(nf->condition)));
                t->setItem(row, 5, new QTableWidgetItem(nf->isbn));
                t->setItem(row, 6, new QTableWidgetItem(nf->deweyClass));

                // USE PATRON-SPECIFIC STATUS
                ItemStatus displayStatus = nf->getStatusForPatron(patronName);
                t->setItem(row, 7, new QTableWidgetItem(statToString(displayStatus)));

                ++row;
            }
        }
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

// Populate Magazine Table
void MainWindow::populateMagazineTable() {
    if (auto* t = get<QTableWidget>(this, "magazineTableWidget")) {
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->clear();
        t->setColumnCount(8);
        t->setHorizontalHeaderLabels({"Title", "Publisher", "Year", "Format", "Condition", "Issue", "Pub Date", "Availability"});
        t->setRowCount(0);

        // Get current patron name
        const Patron* currentP = userService->getCurrentPatron();
        QString patronName = currentP ? currentP->name : QString();

        int row = 0;
        QVector<Item*> items = libraryService->getAllItems();
        for (Item* item : items) {
            if (auto* mag = dynamic_cast<Magazine*>(item)) {
                t->insertRow(row);

                auto* titleItem = new QTableWidgetItem(mag->title);
                titleItem->setData(Qt::UserRole, mag->itemId.toString());
                t->setItem(row, 0, titleItem);
                t->setItem(row, 1, new QTableWidgetItem(mag->creator));
                t->setItem(row, 2, new QTableWidgetItem(QString::number(mag->publicationYear)));
                t->setItem(row, 3, new QTableWidgetItem(mag->format));
                t->setItem(row, 4, new QTableWidgetItem(condToString(mag->condition)));
                t->setItem(row, 5, new QTableWidgetItem(QString::number(mag->issueNumber)));
                t->setItem(row, 6, new QTableWidgetItem(mag->publicationDate.toString()));

                // USE PATRON-SPECIFIC STATUS
                ItemStatus displayStatus = mag->getStatusForPatron(patronName);
                t->setItem(row, 7, new QTableWidgetItem(statToString(displayStatus)));

                ++row;
            }
        }
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

// Populate Movie Table
void MainWindow::populateMovieTable() {
    if (auto* t = get<QTableWidget>(this, "movieTableWidget")) {
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->clear();
        t->setColumnCount(8);
        t->setHorizontalHeaderLabels({"Title", "Director", "Year", "Format", "Condition", "Genre", "Rating", "Availability"});
        t->setRowCount(0);

        // Get current patron name
        const Patron* currentP = userService->getCurrentPatron();
        QString patronName = currentP ? currentP->name : QString();

        int row = 0;
        QVector<Item*> items = libraryService->getAllItems();
        for (Item* item : items) {
            if (auto* mov = dynamic_cast<Movie*>(item)) {
                t->insertRow(row);

                auto* titleItem = new QTableWidgetItem(mov->title);
                titleItem->setData(Qt::UserRole, mov->itemId.toString());
                t->setItem(row, 0, titleItem);
                t->setItem(row, 1, new QTableWidgetItem(mov->creator));
                t->setItem(row, 2, new QTableWidgetItem(QString::number(mov->publicationYear)));
                t->setItem(row, 3, new QTableWidgetItem(mov->format));
                t->setItem(row, 4, new QTableWidgetItem(condToString(mov->condition)));
                t->setItem(row, 5, new QTableWidgetItem(mov->genre));
                t->setItem(row, 6, new QTableWidgetItem(QString::number(mov->rating)));

                // USE PATRON-SPECIFIC STATUS
                ItemStatus displayStatus = mov->getStatusForPatron(patronName);
                t->setItem(row, 7, new QTableWidgetItem(statToString(displayStatus)));

                ++row;
            }
        }
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

// Populate Video Game Table
void MainWindow::populateVideoGameTable() {
    if (auto* t = get<QTableWidget>(this, "videoGameTableWidget")) {
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->clear();
        t->setColumnCount(9);
        t->setHorizontalHeaderLabels({"Title", "Studio", "Year", "Format", "Condition", "Platform", "Genre", "Rating", "Availability"});
        t->setRowCount(0);

        // Get current patron name
        const Patron* currentP = userService->getCurrentPatron();
        QString patronName = currentP ? currentP->name : QString();

        int row = 0;
        QVector<Item*> items = libraryService->getAllItems();
        for (Item* item : items) {
            if (auto* vg = dynamic_cast<VideoGame*>(item)) {
                t->insertRow(row);

                auto* titleItem = new QTableWidgetItem(vg->title);
                titleItem->setData(Qt::UserRole, vg->itemId.toString());
                t->setItem(row, 0, titleItem);
                t->setItem(row, 1, new QTableWidgetItem(vg->creator));
                t->setItem(row, 2, new QTableWidgetItem(QString::number(vg->publicationYear)));
                t->setItem(row, 3, new QTableWidgetItem(vg->format));
                t->setItem(row, 4, new QTableWidgetItem(condToString(vg->condition)));
                t->setItem(row, 5, new QTableWidgetItem(vg->platform));
                t->setItem(row, 6, new QTableWidgetItem(vg->genre));
                t->setItem(row, 7, new QTableWidgetItem(QString::number(vg->rating)));

                // USE PATRON-SPECIFIC STATUS
                ItemStatus displayStatus = vg->getStatusForPatron(patronName);
                t->setItem(row, 8, new QTableWidgetItem(statToString(displayStatus)));

                ++row;
            }
        }
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}


void MainWindow::setupTableHeaders(QTableWidget* table) {
    if (table) {
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}
