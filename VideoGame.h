#ifndef VIDEOGAME_H
#define VIDEOGAME_H

#include "Item.h"

class VideoGame : public Item {
public:
    QString platform;
    QString genre;
    int rating;

    VideoGame(const QString& title,
              const QString& studio,
              int year,
              const QString& format,
              ItemCondition condition,
              const QString& platform,
              const QString& genre,
              int rating);

    QString typeName() const override;
};

#endif // VIDEOGAME_H
