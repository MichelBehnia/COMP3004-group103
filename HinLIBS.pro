QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    Item.cpp \
    FictionBook.cpp \
    NonFictionBook.cpp \
    Magazine.cpp \
    Movie.cpp \
    VideoGame.cpp \
    User.cpp \
    LibraryService.cpp \
    UserService.cpp \
    LoanService.cpp \
    HoldService.cpp \
    DatabaseManager.cpp

HEADERS += \
    mainwindow.h \
    Item.h \
    FictionBook.h \
    NonFictionBook.h \
    Magazine.h \
    Movie.h \
    VideoGame.h \
    User.h \
    LibraryService.h \
    UserService.h \
    LoanService.h \
    HoldService.h \
    DatabaseManager.h

FORMS += \
    mainwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
