#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHeaderView>
#include <QPushButton>
#include <QAction>
#include <QLabel>
#include <QStackedWidget>
#include <QTableWidgetItem>
#include <QLineEdit>
#include <QMessageBox>
#include <algorithm>

//small helpers local
static QString condToString(ItemCondition c) {
    switch (c) {
    case ItemCondition::New:      return "New";
    case ItemCondition::Standard: return "Standard";
    case ItemCondition::Worn:     return "Worn";
    }
    return "Standard";
}

static QString statToString(ItemStatus s) {
    switch (s) {
    case ItemStatus::Available:  return "Available";
    case ItemStatus::CheckedOut: return "Checked Out";
    case ItemStatus::OnHold:     return "On Hold";
    }
    return "Available";
}

//find item by id
static Item* findById(QVector<Item*>& catalogue, const QUuid& id) {
    for (Item* it : catalogue) if (it->itemId == id) return it;
    return nullptr;
}

//get child by name
template <typename T>
T* get(QObject* root, const char* name) {
    return root ? root->findChild<T*>(name) : nullptr;
}

//set label text if exists
static void setTextIf(QLabel* lbl, const QString& s) { if (lbl) lbl->setText(s); }

//mainwindow
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->unborrowSelectedButton_account, &QPushButton::clicked,
        this, &MainWindow::on_unborrowSelectedButton_clicked);

    connect(ui->cancelHoldButton_account, &QPushButton::clicked,
        this, &MainWindow::on_cancelHoldButton_clicked);


    auto* accountButtonsBox = get<QGroupBox>(this, "accountButtonsGroupBox");
    auto* actionButtonsBox = get<QGroupBox>(this, "actionButtonsGroupBox");
    auto* tableButtonsBox = get<QWidget>(this, "tableButtonGroupBox");

    if (accountButtonsBox) accountButtonsBox->hide();
    if (actionButtonsBox) actionButtonsBox->hide();
    if (tableButtonsBox) tableButtonsBox->hide();

    //seed users
    patrons.append(Patron("michel"));
    patrons.append(Patron("eddie"));
    patrons.append(Patron("joseph"));
    patrons.append(Patron("andrey"));
    patrons.append(Patron("bob"));

    librarians.append(Librarian("lauren"));
    systemAdmins.append(SystemAdmin("adam"));

    //demo fines
    if (!patrons.isEmpty()) patrons[0].outstandingFines = 0;

    //seed catalogue
    catalogue.append(new FictionBook("To Kill a Mockingbird", "Harper Lee", 1960, "Hardcover", ItemCondition::Standard, "978-0-06-112008-4"));
    catalogue.append(new FictionBook("1984", "George Orwell", 1949, "Paperback", ItemCondition::Worn, "978-0-452-28423-4"));
    catalogue.append(new FictionBook("Pride and Prejudice", "Jane Austen", 1813, "Hardcover", ItemCondition::New, "978-0-19-280238-5"));
    catalogue.append(new FictionBook("Moby Dick", "Herman Melville", 1851, "Paperback", ItemCondition::Worn, "978-0-14-243724-7"));
    catalogue.append(new FictionBook("The Hobbit", "J.R.R. Tolkien", 1937, "Hardcover", ItemCondition::Standard, "978-0-618-00221-3"));

    catalogue.append(new NonFictionBook("Sapiens: A Brief History of Humankind", "Yuval Noah Harari", 2011, "Paperback", ItemCondition::Standard, "978-0-06-231609-7", "909"));
    catalogue.append(new NonFictionBook("The Selfish Gene", "Richard Dawkins", 1976, "Paperback", ItemCondition::New, "978-0-19-878860-7", "576"));
    catalogue.append(new NonFictionBook("The Immortal Life of Henrietta Lacks", "Rebecca Skloot", 2010, "Hardcover", ItemCondition::Worn, "978-1-4000-5217-2", "616"));
    catalogue.append(new NonFictionBook("A Short History of Nearly Everything", "Bill Bryson", 2003, "Paperback", ItemCondition::Standard, "978-0-385-66094-5", "500"));
    catalogue.append(new NonFictionBook("Thinking, Fast and Slow", "Daniel Kahneman", 2011, "Hardcover", ItemCondition::Worn, "978-0-374-53355-7", "153.4"));

    catalogue.append(new Movie("Inception", "Christopher Nolan", 2010, "Blu-ray", ItemCondition::Standard, "Sci-fi", 8));
    catalogue.append(new Movie("The Godfather", "Francis Ford Coppola", 1972, "DVD", ItemCondition::New, "Crime", 10));
    catalogue.append(new Movie("Forrest Gump", "Robert Zemeckis", 1994, "Blu-ray", ItemCondition::Worn, "Drama", 9));

    catalogue.append(new VideoGame("The Legend of Zelda", "Nintendo", 1986, "Cartridge", ItemCondition::Worn, "NES", "Adventure"));
    catalogue.append(new VideoGame("Super Mario Bros.", "Nintendo", 1985, "Cartridge", ItemCondition::Standard, "NES", "Platformer"));
    catalogue.append(new VideoGame("Halo: Combat Evolved", "Bungie", 2001, "Disc", ItemCondition::Standard, "Xbox", "Shooter"));
    catalogue.append(new VideoGame("Minecraft", "Mojang", 2011, "Digital", ItemCondition::New, "PC", "Sandbox"));

    catalogue.append(new Magazine("National Geographic", "NG Society", 2023, "Print", ItemCondition::New, 5, QDate(2023, 5, 1)));
    catalogue.append(new Magazine("Time", "Time Inc.", 2023, "Print", ItemCondition::Standard, 15, QDate(2023, 4, 15)));
    catalogue.append(new Magazine("The Economist", "The Economist Group", 2023, "Print", ItemCondition::Worn, 22, QDate(2023, 5, 8)));

    //cache widgets
    auto* stacked = get<QStackedWidget>(this, "stackedWidget");
    auto* tableButtons = get<QWidget>(this, "tableButtonGroupBox");

    //initial page
    if (stacked) stacked->setCurrentIndex(0);

    //page nav buttons
    if (auto* b = get<QPushButton>(this, "fictionButton"))
        connect(b, &QPushButton::clicked, this, [this, stacked]() { if (stacked) stacked->setCurrentIndex(1); }, Qt::UniqueConnection);
    if (auto* b = get<QPushButton>(this, "nonFictionButton"))
        connect(b, &QPushButton::clicked, this, [this, stacked]() { if (stacked) stacked->setCurrentIndex(2); }, Qt::UniqueConnection);
    if (auto* b = get<QPushButton>(this, "magazineButton"))
        connect(b, &QPushButton::clicked, this, [this, stacked]() { if (stacked) stacked->setCurrentIndex(3); }, Qt::UniqueConnection);
    if (auto* b = get<QPushButton>(this, "movieButton"))
        connect(b, &QPushButton::clicked, this, [this, stacked]() { if (stacked) stacked->setCurrentIndex(4); }, Qt::UniqueConnection);
    if (auto* b = get<QPushButton>(this, "videoGameButton"))
        connect(b, &QPushButton::clicked, this, [this, stacked]() { if (stacked) stacked->setCurrentIndex(5); }, Qt::UniqueConnection);

    //login wiring
    if (auto* loginBtn = get<QPushButton>(this, "loginButton")) {
        connect(loginBtn, &QPushButton::clicked, this, [this, stacked, tableButtons]() {
            const auto* userEdit = get<QLineEdit>(this, "usernameLineEdit");
            const QString username = userEdit ? userEdit->text().trimmed() : QString();

            bool found = false;
            QString role;

            for (int i = 0; i < patrons.size(); ++i) {
                if (patrons[i].name == username) { found = true; role = "Patron"; currentPatronIndex = i; break; }
            }
            if (!found) for (const Librarian& l : librarians)  if (l.name == username) { found = true; role = "Librarian"; break; }
            if (!found) for (const SystemAdmin& a : systemAdmins) if (a.name == username) { found = true; role = "Admin"; break; }

            auto* roleLbl = get<QLabel>(this, "userRoleLabel");
            if (!found) { setTextIf(roleLbl, "Invalid username"); return; }

            setTextIf(roleLbl, username + ": " + role);

            auto* accountButtonsBox = get<QGroupBox>(this, "accountButtonsGroupBox");
            auto* actionButtonsBox = get<QGroupBox>(this, "actionButtonsGroupBox");
            auto* tableButtonsBox = get<QWidget>(this, "tableButtonGroupBox");

            if (accountButtonsBox) accountButtonsBox->show();
            if (actionButtonsBox) actionButtonsBox->show();
            if (tableButtonsBox) tableButtonsBox->show();

            populateFictionTable();
            populateNonFictionTable();
            populateMagazineTable();
            populateMovieTable();
            populateVideoGameTable();

            //enable double click borrow
            hookDoubleClickBorrow();

            if (stacked) stacked->setCurrentIndex(1);
        }, Qt::UniqueConnection);
    }

    //account status hooks
    if (auto* b = get<QPushButton>(this, "accountStatusButton"))
        connect(b, &QPushButton::clicked, this, &MainWindow::on_accountStatusButton_clicked, Qt::UniqueConnection);
    if (auto* a = get<QAction>(this, "actionAccount_Status"))
        connect(a, &QAction::triggered, this, &MainWindow::accountStatusActionTriggered, Qt::UniqueConnection);
    if (auto* a = get<QAction>(this, "actionUnborrow"))
        connect(a, &QAction::triggered, this, &MainWindow::unborrowActionTriggered, Qt::UniqueConnection);
    if (auto* b = get<QPushButton>(this, "backFromAccountButton"))
        connect(b, &QPushButton::clicked, this, &MainWindow::on_backFromAccountButton_clicked, Qt::UniqueConnection);
}

MainWindow::~MainWindow() {
    for (Item* item : catalogue) delete item;
    delete ui;
}

void MainWindow::on_accountStatusButton_clicked() {
    showAccountStatusPage();

    auto* actionButtonsBox = get<QGroupBox>(this, "actionButtonsGroupBox");
    auto* tableButtonsBox = get<QWidget>(this, "tableButtonGroupBox");

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
    auto* tableButtonsBox = get<QWidget>(this, "tableButtonGroupBox");

    if (actionButtonsBox) actionButtonsBox->show();
    if (tableButtonsBox) tableButtonsBox->show();
}

//borrow button slot
void MainWindow::on_borrowSelectedButton_clicked() {
    if (QTableWidget* t = currentTable()) {
        int row = t->currentRow();
        if (row >= 0) borrowFromRow(t, row);
    }
}

//unborrow button slot
void MainWindow::on_unborrowSelectedButton_clicked() {
    QTableWidget* t = currentTable();

    auto* stacked = get<QStackedWidget>(this, "stackedWidget");
    QWidget* accPage = get<QWidget>(this, "accountStatusPage");
    if (!accPage) accPage = get<QWidget>(this, "accountPage");

    if (!t && stacked && accPage && stacked->currentWidget() == accPage) {
        t = get<QTableWidget>(this, "loansTableWidget");
    }

    if (!t) return;
    int row = t->currentRow();
    if (row >= 0) {
        returnFromRow(t, row);
    }
}


//unborrow action slot
void MainWindow::unborrowActionTriggered() {
    QTableWidget* t = currentTable();

    auto* stacked = get<QStackedWidget>(this, "stackedWidget");
    QWidget* accPage = get<QWidget>(this, "accountStatusPage");
    if (!accPage) accPage = get<QWidget>(this, "accountPage");

    if (!t && stacked && accPage && stacked->currentWidget() == accPage) {
        t = get<QTableWidget>(this, "loansTableWidget");
    }

    if (!t) return;
    int row = t->currentRow();
    if (row >= 0) {
        returnFromRow(t, row);
    }
}


//place hold button slot
void MainWindow::on_placeHoldButton_clicked()
{
    if (QTableWidget* t = currentTable()) {
        int row = t->currentRow();
        if (row < 0) return;
        const QUuid id = idForRow(t, row);
        QString msg;
        placeHoldById(id, &msg);
        populateAccountStatus();
        statusBar()->showMessage(msg, 3000);
    }
}

//cancel hold button slot
void MainWindow::on_cancelHoldButton_clicked()
{
    //get stacked + account page
    auto* stacked = get<QStackedWidget>(this, "stackedWidget");
    QWidget* accPage = get<QWidget>(this, "accountStatusPage");
    if (!accPage) accPage = get<QWidget>(this, "accountPage");

    QTableWidget* t = nullptr;

    //if on account page, use holds table
    if (stacked && accPage && stacked->currentWidget() == accPage) {
        t = get<QTableWidget>(this, "holdsTableWidget");
    } 
    //otherwise use current catalogue table
    else {
        t = currentTable();
    }

    if (!t) return;

    int row = t->currentRow();
    if (row < 0) return;

    const QUuid id = idForRow(t, row);
    if (id.isNull()) return;

    QString msg;
    cancelHoldById(id, &msg);

    //update displays
    populateAccountStatus();
    populateFictionTable();
    populateNonFictionTable();
    populateMagazineTable();
    populateMovieTable();
    populateVideoGameTable();

    statusBar()->showMessage(msg, 3000);
}


void MainWindow::on_logoutButton_clicked() {
    auto* stacked = get<QStackedWidget>(this, "stackedWidget");
    auto* tableButtons = get<QWidget>(this, "tableButtonGroupBox");
    auto* roleLbl = get<QLabel>(this, "userRoleLabel");
    auto* usernameField = get<QLineEdit>(this, "usernameLineEdit");
    auto* accountButtonsBox = get<QGroupBox>(this, "accountButtonsGroupBox");
    auto* actionButtonsBox = get<QGroupBox>(this, "actionButtonsGroupBox");

    if (accountButtonsBox) accountButtonsBox->hide();
    if (actionButtonsBox) actionButtonsBox->hide();
    if (tableButtons)     tableButtons->hide();

    if (stacked) stacked->setCurrentIndex(0);
    if (roleLbl) roleLbl->setText("Logged out");
    if (usernameField) usernameField->clear();
}

// Remaining code unchanged and as you posted...
// (populate methods, helpers, etc.)

//borrowing helpers
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

QUuid MainWindow::idForRow(QTableWidget* table, int row) const {
    if (!table || row < 0 || row >= table->rowCount()) return {};
    QTableWidgetItem* it = table->item(row, 0);
    if (!it) return {};
    const QString idStr = it->data(Qt::UserRole).toString();
    return QUuid(idStr);
}

bool MainWindow::borrowById(const QUuid& id, QString* err) {
    Patron* p = currentPatron();
    if (!p) { if (err) *err = "No patron logged in."; return false; }

    Item* it = findById(catalogue, id);
    if (!it) { if (err) *err = "Item not found."; return false; }

    if (patronHasLoan(*p, id)) {
        if (err) *err = "You already borrowed this item.";
        return false;
    }
    if (p->activeLoans.size() >= 3) {
        if (err) *err = "Max 3 active loans reached (D1).";
        return false;
    }

    switch (it->status) {
    case ItemStatus::Available:
        break;

    case ItemStatus::CheckedOut:
        if (err) *err = "Item is already checked out.";
        return false;

    case ItemStatus::OnHold:
        if (it->holdQueue.front() != p->name) {
            if (err) *err = "Item is on hold for another patron.";
            return false;
        }

        it->holdQueue.pop_front();
        break;
    }

    it->status  = ItemStatus::CheckedOut;
    it->dueDate = QDate::currentDate().addDays(14); //d1 rule
    p->activeLoans.push_back(id);
    if (err) *err = "Borrowed successfully.";
    return true;
}

void MainWindow::borrowFromRow(QTableWidget* table, int row) {
    if (!table) return;
    QString msg;
    const QUuid id = idForRow(table, row);
    if (id.isNull()) return;

    const bool ok = borrowById(id, &msg);
    Q_UNUSED(ok);

    //refresh tables and account
    populateFictionTable();
    populateNonFictionTable();
    populateMagazineTable();
    populateMovieTable();
    populateVideoGameTable();
    populateAccountStatus();

    if (!msg.isEmpty()) statusBar()->showMessage(msg, 3000);

    if (row >= 0 && row < table->rowCount()) table->setCurrentCell(row, 0);
}

void MainWindow::hookDoubleClickBorrow() {
    auto hook = [this](QTableWidget* t) {
        if (!t) return;
        if (!t->property("borrowHooked").toBool()) {
            QObject::connect(t, &QTableWidget::cellDoubleClicked, this,
                             [this, t](int r, int /*c*/) { borrowFromRow(t, r); },
                             Qt::UniqueConnection);
            t->setProperty("borrowHooked", true);
        }
    };
    hook(get<QTableWidget>(this, "fictionTableWidget"));
    hook(get<QTableWidget>(this, "nonFictionTableWidget"));
    hook(get<QTableWidget>(this, "magazineTableWidget"));
    hook(get<QTableWidget>(this, "movieTableWidget"));
    hook(get<QTableWidget>(this, "videoGameTableWidget"));
}

//hold helpers
bool MainWindow::placeHoldById(const QUuid& id, QString* err)
{
    Patron* p = currentPatron();
    if (!p) {
        if (err) *err = "No patron logged in.";
        return false;
    }

    Item* it = findById(catalogue, id);
    if (!it) {
        if (err) *err = "Item not found.";
        return false;
    }

    //must be checked out
    if (it->status != ItemStatus::CheckedOut) {
        if (err) *err = "You can only place holds on checked-out items.";
        return false;
    }

    //cannot hold your own loan
    if (patronHasLoan(*p, id)) {
        if (err) *err = "You cannot place a hold on an item you are currently borrowing.";
        return false;
    }

    //already holding
    if (patronHasHold(*p, id)) {
        if (err) *err = "You already have a hold on this item.";
        return false;
    }

    //push to queue
    it->holdQueue.push_back(p->name);
    p->activeHolds.push_back(id);

    int position = it->holdQueue.size();
    if (err) *err = QString("Hold placed successfully. You are #%1 in the queue.").arg(position);
    return true;
}



bool MainWindow::cancelHoldById(const QUuid& id, QString* err)
{
    Patron* p = currentPatron();
    if (!p) {
        if (err) *err = "No patron logged in.";
        return false;
    }

    Item* it = findById(catalogue, id);
    if (!it) {
        if (err) *err = "Item not found.";
        return false;
    }

    auto holdIt = std::find(p->activeHolds.begin(), p->activeHolds.end(), id);
    if (holdIt == p->activeHolds.end()) {
        if (err) *err = "You do not have a hold on this item.";
        return false;
    }

    // Remove from patron list
    p->activeHolds.erase(holdIt);

    // Remove from item queue
    auto qIt = std::find(it->holdQueue.begin(), it->holdQueue.end(), p->name);
    if (qIt != it->holdQueue.end())
        it->holdQueue.erase(qIt);

    if (err) *err = "Hold canceled successfully.";
    return true;
}



//return helpers
bool MainWindow::returnById(const QUuid& id, QString* err) {
    Patron* p = currentPatron();
    if (!p) { if (err) *err = "No patron logged in."; return false; }

    Item* it = findById(catalogue, id);
    if (!it) { if (err) *err = "Item not found."; return false; }

    //must be checked out by this patron
    auto itPos = std::find(p->activeLoans.begin(), p->activeLoans.end(), id);
    if (it->status != ItemStatus::CheckedOut || itPos == p->activeLoans.end()) {
        if (err) *err = "You don't have this item on loan.";
        return false;
    }

    //return item
    p->activeLoans.erase(itPos);

    if (!it->holdQueue.isEmpty()) {
        it->status = ItemStatus::OnHold;
        QString nextPatronName = it->holdQueue.front();
    } else {
        it->status = ItemStatus::Available;
    }

    it->dueDate = QDate();


    if (err) *err = "Returned successfully.";
    return true;
}

void MainWindow::returnFromRow(QTableWidget* table, int row) {
    if (!table) return;
    QString msg;
    const QUuid id = idForRow(table, row);
    if (id.isNull()) return;

    const bool ok = returnById(id, &msg);
    Q_UNUSED(ok);

    //refresh tables and account
    populateFictionTable();
    populateNonFictionTable();
    populateMagazineTable();
    populateMovieTable();
    populateVideoGameTable();
    populateAccountStatus();

    if (!msg.isEmpty()) statusBar()->showMessage(msg, 3000);

    if (row >= 0 && row < table->rowCount())
        table->setCurrentCell(std::min(row, table->rowCount() - 1), 0);
}

//account status page
void MainWindow::showAccountStatusPage() {
    populateAccountStatus();

    auto* stacked = get<QStackedWidget>(this, "stackedWidget");
    QWidget* accPage = get<QWidget>(this, "accountStatusPage");
    if (!accPage) accPage = get<QWidget>(this, "accountPage");

    if (stacked && accPage) {
        int idx = stacked->indexOf(accPage);
        if (idx >= 0) stacked->setCurrentIndex(idx);
    }
}

void MainWindow::populateAccountStatus() {
    const Patron* p = currentPatron();
    if (!p) return;

    setTextIf(get<QLabel>(this, "accountNameLabel"), p->name);
    if (auto* role = get<QLabel>(this, "accountRoleLabel")) role->setText("Patron");
    else if (auto* role2 = get<QLabel>(this, "accountTitleLabel")) role2->setText("Patron");

    if (auto* fines = get<QLabel>(this, "finesValueLabel"))
        fines->setText(QString("$%1").arg(QString::number(p->outstandingFines, 'f', 2)));

    //loans table
    if (auto* t = get<QTableWidget>(this, "loansTableWidget")) {
        t->clear();
        t->setRowCount(0);
        t->setColumnCount(5);
        t->setHorizontalHeaderLabels({"Title", "Type", "Status", "Due", "Condition"});
        int row = 0;
        for (const QUuid& id : p->activeLoans) {
        if (Item* it = findById(catalogue, id)) {
            t->insertRow(row);

            //title column + store itemId so returnFromRow/idForRow work here too
            auto *titleItem = new QTableWidgetItem(it->title);
            titleItem->setData(Qt::UserRole, it->itemId.toString());
            t->setItem(row, 0, titleItem);

            t->setItem(row, 1, new QTableWidgetItem(it->typeName()));
            t->setItem(row, 2, new QTableWidgetItem(statToString(it->status)));
            t->setItem(row, 3, new QTableWidgetItem(it->dueDate.isValid() ? it->dueDate.toString() : "-"));
            t->setItem(row, 4, new QTableWidgetItem(condToString(it->condition)));
            ++row;
        }
    }

        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }

    //holds table
    if (auto* t = get<QTableWidget>(this, "holdsTableWidget")) {
        t->clear();
        t->setRowCount(0);
        t->setColumnCount(4);
        t->setHorizontalHeaderLabels({"Title", "Type", "Status", "Queue Position"});

        int row = 0;
        for (const QUuid& id : p->activeHolds) {
            if (Item* it = findById(catalogue, id)) {
                t->insertRow(row);
                t->setItem(row, 0, new QTableWidgetItem(it->title));
                t->item(row, 0)->setData(Qt::UserRole, it->itemId.toString()); 
                t->setItem(row, 1, new QTableWidgetItem(it->typeName()));
                t->setItem(row, 2, new QTableWidgetItem(statToString(it->status)));

                // queue position logic
                int pos = -1;
                if (!it->holdQueue.empty()) {
                    for (int i = 0; i < it->holdQueue.size(); ++i) {
                        if (it->holdQueue[i] == p->name) {
                            pos = i + 1;
                            break;
                        }
                    }
                }
                QString posText = (pos > 0) ? QString::number(pos) : "-";
                t->setItem(row, 3, new QTableWidgetItem(posText));

                ++row;
            }
        }
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

//current patron helpers
Patron* MainWindow::currentPatron() {
    if (currentPatronIndex < 0 || currentPatronIndex >= patrons.size()) return nullptr;
    return &patrons[currentPatronIndex];
}
const Patron* MainWindow::currentPatron() const {
    if (currentPatronIndex < 0 || currentPatronIndex >= patrons.size()) return nullptr;
    return &patrons[currentPatronIndex];
}

//string helpers
QString MainWindow::conditionToString(ItemCondition c) const { return condToString(c); }
QString MainWindow::statusToString(ItemStatus s) const { return statToString(s); }

//patron query helpers
bool MainWindow::patronHasLoan(const Patron& p, const QUuid& id) const {
    return std::any_of(p.activeLoans.begin(), p.activeLoans.end(),
                       [&](const QUuid& x){ return x == id; });
}
bool MainWindow::patronHasHold(const Patron& p, const QUuid& id) const {
    return std::any_of(p.activeHolds.begin(), p.activeHolds.end(),
                       [&](const QUuid& x){ return x == id; });
}

//populate tables
void MainWindow::populateFictionTable() {
    if (auto* t = get<QTableWidget>(this, "fictionTableWidget")) {
        t->clear();
        t->setColumnCount(7);
        t->setHorizontalHeaderLabels({"Title", "Author", "Year", "Format", "Condition", "ISBN", "Availability"});
        t->setRowCount(0);
        int row = 0;
        for (Item* item : catalogue) {
            if (auto* fb = dynamic_cast<FictionBook*>(item)) {
                t->insertRow(row);
                t->setItem(row, 0, new QTableWidgetItem(fb->title));
                t->item(row, 0)->setData(Qt::UserRole, fb->itemId.toString());
                t->setItem(row, 1, new QTableWidgetItem(fb->creator));
                t->setItem(row, 2, new QTableWidgetItem(QString::number(fb->publicationYear)));
                t->setItem(row, 3, new QTableWidgetItem(fb->format));
                t->setItem(row, 4, new QTableWidgetItem(condToString(fb->condition)));
                t->setItem(row, 5, new QTableWidgetItem(fb->isbn));
                t->setItem(row, 6, new QTableWidgetItem(statToString(fb->status)));

                ++row;
            }
        }
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

void MainWindow::populateNonFictionTable() {
    if (auto* t = get<QTableWidget>(this, "nonFictionTableWidget")) {
        t->clear();
        t->setColumnCount(8);
        t->setHorizontalHeaderLabels({"Title","Author","Year","Format","Condition","ISBN","Dewey","Availability"});
        t->setRowCount(0);
        int row = 0;
        for (Item* item : catalogue) {
            if (auto* nf = dynamic_cast<NonFictionBook*>(item)) {
                t->insertRow(row);
                t->setItem(row, 0, new QTableWidgetItem(nf->title));
                t->item(row, 0)->setData(Qt::UserRole, nf->itemId.toString());
                t->setItem(row, 1, new QTableWidgetItem(nf->creator));
                t->setItem(row, 2, new QTableWidgetItem(QString::number(nf->publicationYear)));
                t->setItem(row, 3, new QTableWidgetItem(nf->format));
                t->setItem(row, 4, new QTableWidgetItem(condToString(nf->condition)));
                t->setItem(row, 5, new QTableWidgetItem(nf->isbn));
                t->setItem(row, 6, new QTableWidgetItem(nf->deweyClass));
                t->setItem(row, 7, new QTableWidgetItem(statToString(nf->status)));
                ++row;
            }
        }
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

void MainWindow::populateMagazineTable() {
    if (auto* t = get<QTableWidget>(this, "magazineTableWidget")) {
        t->clear();
        t->setColumnCount(8);
        t->setHorizontalHeaderLabels({"Title","Publisher","Year","Format","Condition","Issue","Pub Date","Availability"});
        t->setRowCount(0);
        int row = 0;
        for (Item* item : catalogue) {
            if (auto* mag = dynamic_cast<Magazine*>(item)) {
                t->insertRow(row);
                t->setItem(row, 0, new QTableWidgetItem(mag->title));
                t->item(row, 0)->setData(Qt::UserRole, mag->itemId.toString());
                t->setItem(row, 1, new QTableWidgetItem(mag->creator));
                t->setItem(row, 2, new QTableWidgetItem(QString::number(mag->publicationYear)));
                t->setItem(row, 3, new QTableWidgetItem(mag->format));
                t->setItem(row, 4, new QTableWidgetItem(condToString(mag->condition)));
                t->setItem(row, 5, new QTableWidgetItem(QString::number(mag->issueNumber)));
                t->setItem(row, 6, new QTableWidgetItem(mag->publicationDate.toString()));
                t->setItem(row, 7, new QTableWidgetItem(statToString(mag->status)));
                ++row;
            }
        }
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

void MainWindow::populateMovieTable() {
    if (auto* t = get<QTableWidget>(this, "movieTableWidget")) {
        t->clear();
        t->setColumnCount(8);
        t->setHorizontalHeaderLabels({"Title","Director","Year","Format","Condition","Genre","Rating","Availability"});
        t->setRowCount(0);
        int row = 0;
        for (Item* item : catalogue) {
            if (auto* mov = dynamic_cast<Movie*>(item)) {
                t->insertRow(row);
                t->setItem(row, 0, new QTableWidgetItem(mov->title));
                t->item(row, 0)->setData(Qt::UserRole, mov->itemId.toString());
                t->setItem(row, 1, new QTableWidgetItem(mov->creator));
                t->setItem(row, 2, new QTableWidgetItem(QString::number(mov->publicationYear)));
                t->setItem(row, 3, new QTableWidgetItem(mov->format));
                t->setItem(row, 4, new QTableWidgetItem(condToString(mov->condition)));
                t->setItem(row, 5, new QTableWidgetItem(mov->genre));
                t->setItem(row, 6, new QTableWidgetItem(QString::number(mov->rating)));
                t->setItem(row, 7, new QTableWidgetItem(statToString(mov->status)));
                ++row;
            }
        }
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

void MainWindow::populateVideoGameTable() {
    if (auto* t = get<QTableWidget>(this, "videoGameTableWidget")) {
        t->clear();
        t->setColumnCount(8);
        t->setHorizontalHeaderLabels({"Title","Studio","Year","Format","Condition","Platform","Genre","Availability"});
        t->setRowCount(0);
        int row = 0;
        for (Item* item : catalogue) {
            if (auto* vg = dynamic_cast<VideoGame*>(item)) {
                t->insertRow(row);
                t->setItem(row, 0, new QTableWidgetItem(vg->title));
                t->item(row, 0)->setData(Qt::UserRole, vg->itemId.toString());
                t->setItem(row, 1, new QTableWidgetItem(vg->creator));
                t->setItem(row, 2, new QTableWidgetItem(QString::number(vg->publicationYear)));
                t->setItem(row, 3, new QTableWidgetItem(vg->format));
                t->setItem(row, 4, new QTableWidgetItem(condToString(vg->condition)));
                t->setItem(row, 5, new QTableWidgetItem(vg->platform));
                t->setItem(row, 6, new QTableWidgetItem(vg->genre));
                t->setItem(row, 7, new QTableWidgetItem(statToString(vg->status)));
                ++row;
            }
        }
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}
