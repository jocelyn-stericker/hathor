#ifndef HSEARCHCONTEXT_H
#define HSEARCHCONTEXT_H

#include <QWidget>

namespace Ui {
class HSearchContext;
}

class HSearchContext : public QWidget
{
    Q_OBJECT
    
    static HSearchContext* s_singleton;
    explicit HSearchContext(QWidget *parent = 0);
    QString s_term;

    int s_albumCount;
    int s_artistCount;
    int s_tagCount;
    int s_trackCount;
    int s_userCount;
    int s_venueCount;
    QList<QWidget*> s_results;
public:
    static HSearchContext* singleton() { return s_singleton=(s_singleton?s_singleton:new HSearchContext); } //could change!!! so don't connect to setSearchTerm
    static void detach() { s_singleton=0; }

    ~HSearchContext();

public slots:
    void setSearchTerm(QString); // singleton could change!!! so don't connect to setSearchTerm

    void getMoreAlbums(QString s);
    void getMoreArtists(QString s);
    void getMoreTags(QString s);
    void getMoreTracks(QString s);
    void getMoreUsers(QString s);
    void getMoreVenues(QString s);

signals:
    void searchTermChanged();
private:
    Ui::HSearchContext *ui;
};

#endif // HSEARCHCONTEXT_H
