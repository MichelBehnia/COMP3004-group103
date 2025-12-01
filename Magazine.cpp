#include "Magazine.h"

Magazine::Magazine(const QString& title,
                   const QString& publisher,
                   int year,
                   const QString& format,
                   ItemCondition condition,
                   int issueNum,
                   const QDate& pubDate)
    : Item(title, publisher, year, format, condition), issueNumber(issueNum), publicationDate(pubDate)
{
}

QString Magazine::typeName() const {
    return "Magazine";
}
