#ifndef HARTIST_H
#define HARTIST_H

#include "hobject.h"
#include <QStringList>
#include <QPixmap>
#include <QVariant>
#include <QObject>
#include <QTimer>
#include "halbum.h"
#include "htrack.h"
#include "hshout.h"
#include "libhathor_global.h"

struct HTag;

class LIBHATHORSHARED_EXPORT ArtistInfo : public HCachedInfo {
    friend class HArtist;
    ArtistInfo(QString artist);
    bool process(const QString& data);
};

class LIBHATHORSHARED_EXPORT ArtistExtraTagData : public HCachedInfo {
    friend class HArtist;
    ArtistExtraTagData(QString artist);
    bool process(const QString& data);
};

class LIBHATHORSHARED_EXPORT ArtistAlbumData : public HCachedInfo {
    friend class HArtist;
    ArtistAlbumData(QString artist);
    bool process(const QString& data);
};

class LIBHATHORSHARED_EXPORT ArtistTrackData : public HCachedInfo {
    friend class HArtist;
    ArtistTrackData(QString artist);
    bool process(const QString& data);
};

class LIBHATHORSHARED_EXPORT ArtistSimilarData : public HCachedInfo {
    friend class HArtist;
    ArtistSimilarData(QString artist);
    bool process(const QString& data);
};

struct LIBHATHORSHARED_EXPORT ArtistShoutData : public QObject {
    Q_OBJECT
    friend class HArtist;
private:
    QMutex mutex, mutex_2;
    QString artist;
    QList<HShout*> shouts;  QList< QPair<QObject*,QString> > shoutQueue;
    bool got;
    HRunOnceNotifier* getting;
    ArtistShoutData() : got(0), getting(0) {}
public slots:
    void sendData(QString artist);
    void sendData_process();
    void sendData_processQueue();
};

class LIBHATHORSHARED_EXPORT HArtist : public HObject
{
    Q_OBJECT
    QMutex queueMutex;
    HCachedPixmap* s_cachedPixmap[4];
    QList< QPair<QObject*,QString> > s_picQueue[4];
    QList< QPair<QObject*,QString> > s_tagQueue;
    QList< QPair<QObject*,QString> > s_moreTagQueue;
    struct HTriple {
        QObject* first;
        QString second;
        int third;
        HTriple(QObject* a,const QString& b,const int& c) : first(a),second(b),third(c) {}
    };

    QList< HTriple > s_albumQueue;
    QList< HTriple > s_trackQueue;
    QList< HTriple > s_similarQueue;

public:
    static HArtist& get(QString name);
    enum PictureSize {
        Small=0,
        Medium=1,
        Large=2,
        Mega=3
    };

public slots:
    QString getName() { return s_name; }
    void sendPic(PictureSize p,QObject* o,QString m); /* QPixmap */
    void sendPic_2_0(QString pic) { sendPic_2((PictureSize)0,pic); }
    void sendPic_2_1(QString pic) { sendPic_2((PictureSize)1,pic); }
    void sendPic_2_2(QString pic) { sendPic_2((PictureSize)2,pic); }
    void sendPic_2_3(QString pic) { sendPic_2((PictureSize)3,pic); }
    void sendPicNames(PictureSize p,QObject* o,QString m); /* QString */
    void sendTagNames(QObject* o,QString m); /* QStringList */
    void sendTags(QObject* o,QString m); /* QList<HTag*> */
    void sendMoreTagNames(QObject* o,QString m); /* QStringList */
    void sendMoreTags(QObject* obj, const char* member); /*QList<HTag*>*/
    void sendMoreTags_2(QStringList); /*for internal use*/
    void sendListenerCount(QObject* o,QString m); /*int*/
    void sendPlayCount(QObject* o,QString m); /*int*/
    void sendUserPlayCount(QObject* o,QString m); /*int*/
    void sendBio(QObject* o,QString m); /*QString*/
    void sendBioShort(QObject* o,QString m); /*QString*/
    void sendAlbumsNames(QObject* o,QString m); /* QStringList */
    void sendAlbums(QObject* o,QString m, int count=-1); /* multiple HAlbum* */
    void sendTrackNames(QObject* o,QString m); /* QString */
    void sendTracks(QObject* o,QString m, int count=-1); /* multiple HTrack* */
    void sendSimilarNames(QObject* o,QString m); /* QStringList */
    void sendSimilar(QObject* o,QString m, int count=-1); /* multiple HArtist* */
    void sendShouts(QObject* o,QString m); /* QList<HShout*> */
    void sendSimilarScores(QObject* o,QString m); /* QList<double> */
    QPixmap getExtraPic(int which); //which<=getExtraPicCachedCount() /*QPixmap */
    int getExtraPicCount(); /* int */
    int getExtraPicCachedCount(); /* int */

private:
    static QMap<QString, HArtist*> _map;
    HArtist(QString name);  // use HArtist::get(name)

    QString s_name;

    ArtistInfo s_infoData;
    ArtistExtraTagData s_extraTagData;
    ArtistAlbumData s_albumData;
    ArtistTrackData s_trackData;
    ArtistSimilarData s_similarData;
    ArtistShoutData s_shoutData;

    struct ExtraPictureData {
        QList<QPixmap> pics;
        QStringList pic_urls;
        bool got_urls;
        HRunOnceNotifier* getting;
        ExtraPictureData() : got_urls(0),getting(0) {}
        void getData(QString artist);
        void fetchAnother();
    } s_extraPictureData;

public slots:
    void sendPic_2(PictureSize p,QString pic); /* for internal use */
    void sendTags_2(QStringList); /* for internal use */
    void sendAlbums_2(QStringList); /* for internal use */
    void sendTracks_2(QStringList); /* for internal use */
    void sendSimilar_2(QStringList); /* for internal use */

private:
    //Degenerate copy and assignment
    HArtist(const HArtist&);
    HArtist& operator=(const HArtist&);
};

#endif // HARTIST_H