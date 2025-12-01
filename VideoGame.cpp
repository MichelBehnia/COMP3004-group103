#include "VideoGame.h"

VideoGame::VideoGame(const QString& title,
                     const QString& studio,
                     int year,
                     const QString& format,
                     ItemCondition condition,
                     const QString& platform,
                     const QString& genre,
                     int rating)
    : Item(title, studio, year, format, condition), platform(platform), genre(genre), rating(rating)
{
}

QString VideoGame::typeName() const {
    return "Video Game";
}
