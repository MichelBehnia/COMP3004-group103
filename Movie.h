#ifndef MOVIE_H
#define MOVIE_H

#include "Item.h"

class Movie : public Item {
public:
    QString genre;
    int rating;

    Movie(const QString& title,
          const QString& director,
          int year,
          const QString& format,
          ItemCondition condition,
          const QString& genre,
          int rating);

    QString typeName() const override;
};

#endif // MOVIE_H
