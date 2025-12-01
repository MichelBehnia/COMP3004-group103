#include "Movie.h"

Movie::Movie(const QString& title,
             const QString& director,
             int year,
             const QString& format,
             ItemCondition condition,
             const QString& genre,
             int rating)
    : Item(title, director, year, format, condition), genre(genre), rating(rating)
{
}

QString Movie::typeName() const {
    return "Movie";
}
