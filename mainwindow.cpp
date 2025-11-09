#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTableWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    patrons.append(Patron("michel"));
    patrons.append(Patron("eddie"));
    patrons.append(Patron("joseph"));
    patrons.append(Patron("andrey"));
    patrons.append(Patron("bob"));
    librarians.append(Librarian("lauren"));
    systemAdmins.append(SystemAdmin("adam"));

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
    catalogue.append(new Magazine("National Geographic", "NG Society", 2023, "Print", ItemCondition::New, 5, QDate(2023, 5, 1)));
    catalogue.append(new Magazine("Time", "Time Inc.", 2023, "Print", ItemCondition::Standard, 15, QDate(2023, 4, 15)));
    catalogue.append(new Magazine("The Economist", "The Economist Group", 2023, "Print", ItemCondition::Worn, 22, QDate(2023, 5, 8)));
    catalogue.append(new VideoGame("The Legend of Zelda", "Nintendo", 1986, "Cartridge", ItemCondition::Worn, "Adventure", 9));
    catalogue.append(new VideoGame("Super Mario Bros.", "Nintendo", 1985, "Cartridge", ItemCondition::Standard, "Platformer", 8));
    catalogue.append(new VideoGame("Halo: Combat Evolved", "Bungie", 2001, "Disc", ItemCondition::Standard, "Shooter", 9));
    catalogue.append(new VideoGame("Minecraft", "Mojang", 2011, "Digital", ItemCondition::New, "Sandbox", 10));

    ui->stackedWidget->setCurrentIndex(0);
    ui->tableButtonGroupBox->hide();

    connect(ui->fictionButton,     &QPushButton::clicked, [this]() { ui->stackedWidget->setCurrentIndex(1); });
    connect(ui->nonFictionButton,  &QPushButton::clicked, [this]() { ui->stackedWidget->setCurrentIndex(2); });
    connect(ui->magazineButton,    &QPushButton::clicked, [this]() { ui->stackedWidget->setCurrentIndex(3); });
    connect(ui->movieButton,       &QPushButton::clicked, [this]() { ui->stackedWidget->setCurrentIndex(4); });
    connect(ui->videoGameButton,   &QPushButton::clicked, [this]() { ui->stackedWidget->setCurrentIndex(5); });

    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::onLoginButtonClicked);
}

MainWindow::~MainWindow() {
    for (Item* item : catalogue) {
        delete item;
    }
    delete ui;
}

void MainWindow::populateFictionTable() {
    ui->fictionTableWidget->clear();
    ui->fictionTableWidget->setColumnCount(6);
    ui->fictionTableWidget->setHorizontalHeaderLabels({"Title", "Author", "Year", "Format", "Condition", "ISBN"});
    int row = 0;
    ui->fictionTableWidget->setRowCount(0);
    for (Item* item : catalogue) {
        if (auto* fb = dynamic_cast<FictionBook*>(item)) {
            ui->fictionTableWidget->insertRow(row);
            ui->fictionTableWidget->setItem(row, 0, new QTableWidgetItem(fb->title));
            ui->fictionTableWidget->setItem(row, 1, new QTableWidgetItem(fb->creator));
            ui->fictionTableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(fb->publicationYear)));
            ui->fictionTableWidget->setItem(row, 3, new QTableWidgetItem(fb->format));
            ui->fictionTableWidget->setItem(row, 4, new QTableWidgetItem(
                fb->condition == ItemCondition::New ? "New" :
                fb->condition == ItemCondition::Standard ? "Standard" : "Worn"));
            ui->fictionTableWidget->setItem(row, 5, new QTableWidgetItem(fb->ISBN));
            ++row;
        }
    }
    ui->fictionTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::populateNonFictionTable() {
    ui->nonFictionTableWidget->clear();
    ui->nonFictionTableWidget->setColumnCount(7);
    ui->nonFictionTableWidget->setHorizontalHeaderLabels({"Title", "Author", "Year", "Format", "Condition", "ISBN", "Dewey"});
    int row = 0;
    ui->nonFictionTableWidget->setRowCount(0);
    for (Item* item : catalogue) {
        if (auto* nf = dynamic_cast<NonFictionBook*>(item)) {
            ui->nonFictionTableWidget->insertRow(row);
            ui->nonFictionTableWidget->setItem(row, 0, new QTableWidgetItem(nf->title));
            ui->nonFictionTableWidget->setItem(row, 1, new QTableWidgetItem(nf->creator));
            ui->nonFictionTableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(nf->publicationYear)));
            ui->nonFictionTableWidget->setItem(row, 3, new QTableWidgetItem(nf->format));
            ui->nonFictionTableWidget->setItem(row, 4, new QTableWidgetItem(
                nf->condition == ItemCondition::New ? "New" :
                nf->condition == ItemCondition::Standard ? "Standard" : "Worn"));
            ui->nonFictionTableWidget->setItem(row, 5, new QTableWidgetItem(nf->ISBN));
            ui->nonFictionTableWidget->setItem(row, 6, new QTableWidgetItem(nf->deweyDecimal));
            ++row;
        }
    }
    ui->nonFictionTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::populateMagazineTable() {
    ui->magazineTableWidget->clear();
    ui->magazineTableWidget->setColumnCount(7);
    ui->magazineTableWidget->setHorizontalHeaderLabels(
        {"Title", "Publisher", "Year", "Format", "Condition", "Issue", "Pub Date"});
    int row = 0;
    ui->magazineTableWidget->setRowCount(0);
    for (Item* item : catalogue) {
        if (auto* mag = dynamic_cast<Magazine*>(item)) {
            ui->magazineTableWidget->insertRow(row);
            ui->magazineTableWidget->setItem(row, 0, new QTableWidgetItem(mag->title));
            ui->magazineTableWidget->setItem(row, 1, new QTableWidgetItem(mag->creator));
            ui->magazineTableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(mag->publicationYear)));
            ui->magazineTableWidget->setItem(row, 3, new QTableWidgetItem(mag->format));
            ui->magazineTableWidget->setItem(row, 4, new QTableWidgetItem(
                mag->condition == ItemCondition::New ? "New" :
                mag->condition == ItemCondition::Standard ? "Standard" : "Worn"));
            ui->magazineTableWidget->setItem(row, 5, new QTableWidgetItem(QString::number(mag->issueNumber)));
            ui->magazineTableWidget->setItem(row, 6, new QTableWidgetItem(mag->publicationDate.toString()));
            ++row;
        }
    }
    ui->magazineTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::populateMovieTable() {
    ui->movieTableWidget->clear();
    ui->movieTableWidget->setColumnCount(7);
    ui->movieTableWidget->setHorizontalHeaderLabels(
        {"Title", "Director", "Year", "Format", "Condition", "Genre", "Rating"});
    int row = 0;
    ui->movieTableWidget->setRowCount(0);
    for (Item* item : catalogue) {
        if (auto* mov = dynamic_cast<Movie*>(item)) {
            ui->movieTableWidget->insertRow(row);
            ui->movieTableWidget->setItem(row, 0, new QTableWidgetItem(mov->title));
            ui->movieTableWidget->setItem(row, 1, new QTableWidgetItem(mov->creator));
            ui->movieTableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(mov->publicationYear)));
            ui->movieTableWidget->setItem(row, 3, new QTableWidgetItem(mov->format));
            ui->movieTableWidget->setItem(row, 4, new QTableWidgetItem(
                mov->condition == ItemCondition::New ? "New"
                : mov->condition == ItemCondition::Standard ? "Standard" : "Worn"));
            ui->movieTableWidget->setItem(row, 5, new QTableWidgetItem(mov->genre));
            ui->movieTableWidget->setItem(row, 6, new QTableWidgetItem(QString::number(mov->rating)));
            ++row;
        }
    }
    ui->movieTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::populateVideoGameTable() {
    ui->videoGameTableWidget->clear();
    ui->videoGameTableWidget->setColumnCount(7);
    ui->videoGameTableWidget->setHorizontalHeaderLabels(
        {"Title", "Studio", "Year", "Format", "Condition", "Genre", "Rating"});
    int row = 0;
    ui->videoGameTableWidget->setRowCount(0);
    for (Item* item : catalogue) {
        if (auto* vg = dynamic_cast<VideoGame*>(item)) {
            ui->videoGameTableWidget->insertRow(row);
            ui->videoGameTableWidget->setItem(row, 0, new QTableWidgetItem(vg->title));
            ui->videoGameTableWidget->setItem(row, 1, new QTableWidgetItem(vg->creator));
            ui->videoGameTableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(vg->publicationYear)));
            ui->videoGameTableWidget->setItem(row, 3, new QTableWidgetItem(vg->format));
            ui->videoGameTableWidget->setItem(row, 4, new QTableWidgetItem(
                vg->condition == ItemCondition::New ? "New"
                : vg->condition == ItemCondition::Standard ? "Standard" : "Worn"));
            ui->videoGameTableWidget->setItem(row, 5, new QTableWidgetItem(vg->genre));
            ui->videoGameTableWidget->setItem(row, 6, new QTableWidgetItem(QString::number(vg->rating)));
            ++row;
        }
    }
    ui->videoGameTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::onLoginButtonClicked() {
    QString username = ui->usernameLineEdit->text().trimmed();

    bool found = false;
    QString role;

    for (const Patron &p : patrons) {
        if (p.username == username) {
            found = true;
            role = "Patron";
            break;
        }
    }
    if (!found) {
        for (const Librarian &l : librarians) {
            if (l.username == username) {
                found = true;
                role = "Librarian";
                break;
            }
        }
    }
    if (!found) {
        for (const SystemAdmin &a : systemAdmins) {
            if (a.username == username) {
                found = true;
                role = "Admin";
                break;
            }
        }
    }

    if (!found) {
        ui->userRoleLabel->setText("Invalid username");
        return;
    }

    ui->userRoleLabel->setText(username + ": " + role);
    ui->tableButtonGroupBox->show();

    populateFictionTable();
    populateNonFictionTable();
    populateMagazineTable();
    populateMovieTable();
    populateVideoGameTable();
    ui->stackedWidget->setCurrentIndex(1);
}
