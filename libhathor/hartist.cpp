#include "hartist.h"
#include "htrack.h"
#include "htag.h"

#include <QUrl>
#include <QDesktopServices>
#include <QFile>
#include <QUrl>
#include <QDir>
#include <QCryptographicHash>
#include <QHttp>
#include <QSettings>
#include <lastfm/ws.h>
#include <QDomDocument>
#include "lastfmext.h"

QHash<QString, HArtist*> HArtist::_map;

HArtist& HArtist::get(QString name) {
    if(_map.value(name,0)) {
        return *_map.value(name);
    }

    _map.insert(name,new HArtist(name));
    return get(name);
}

int** HArtist::sendPic(PictureSize p, QObject *obj, QString member) {
    connect(obj,SIGNAL(destroyed(QObject*)),this,SLOT(removeFromQueue(QObject*)));
    s_picQueue[p].push_back(qMakePair(obj,member));
    return sendPicNames(p,this,QString("sendPic_2_"+QString::number(p)).toUtf8().data(),obj);
}

void HArtist::sendPic_2(PictureSize p,QString pic) {
    if(!s_cachedPixmap[p]) s_cachedPixmap[p]=HCachedPixmap::get(QUrl(pic));
    for(int i=0;i<s_picQueue[p].size();i++) {
        *s_cachedPixmap[p]->send(s_picQueue[p][i].first,s_picQueue[p][i].second)=*s_infoData.getPriorityForProperty(s_picQueue[p][i].first,"pics::"+QString::number(p));
    }
    s_picQueue[p].clear();
}

int** HArtist::sendTagNames(QObject *o, QString m, QObject *g) { return s_infoData.sendProperty("tagNames",o,m,g); }

int** HArtist::sendTags(QObject *obj, QString member) {
    connect(obj,SIGNAL(destroyed(QObject*)),this,SLOT(removeFromQueue(QObject*)));
    s_tagQueue.push_back(qMakePair(obj,member));
    return sendTagNames(this,"sendTags_2",obj);
}

void HArtist::sendTags_2(QStringList t) {
    QMutexLocker locker(&queueMutex);
    QList<HTag*> ret;
    for(int i=0;i<t.size();i++) {
        ret.push_back(&HTag::get(t[i]));
    }
    while(s_tagQueue.size()) {
        QPair< QObject*, QString > p=s_tagQueue.takeFirst();
        QMetaObject::invokeMethod(p.first,p.second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(QList<HTag*>,ret));
    }
    s_tagQueue.clear();
}

int** HArtist::sendPicNames(PictureSize size, QObject *obj, QString member,QObject* g) { return s_infoData.sendProperty("pics::"+QString::number(size),obj,member,g); }
int** HArtist::sendMoreTagNames(QObject *o, QString m,QObject* g) { return s_extraTagData.sendProperty("tagNames",o,m,g);}

int** HArtist::sendMoreTags(QObject *obj, const char *member) {
    connect(obj,SIGNAL(destroyed(QObject*)),this,SLOT(removeFromQueue(QObject*)));
    s_moreTagQueue.push_back(qMakePair(obj,QString(member)));
    return sendMoreTagNames(this,"sendMoreTags_2",obj);
}

void HArtist::sendMoreTags_2(QStringList t) {
    QMutexLocker locker(&queueMutex);
    QList<HTag*> ret;
    for(int i=0;i<t.size();i++) {
        ret.push_back(&HTag::get(t[i]));
    }
    for(int i=0;i<s_moreTagQueue.size();i++) {
        QMetaObject::invokeMethod(s_moreTagQueue[i].first,s_moreTagQueue[i].second.toUtf8().data(),Q_ARG(QList<HTag*>,ret));
    }
    s_moreTagQueue.clear();
}

int** HArtist::sendListenerCount(QObject *o, QString m) { return s_infoData.sendProperty("listeners",o,m); }
int** HArtist::sendPlayCount(QObject *o, QString m) { return s_infoData.sendProperty("playCount",o,m); }
int** HArtist::sendUserPlayCount(QObject *o, QString m) { return s_infoData.sendProperty("userPlayCount",o,m); }
int** HArtist::sendBio(QObject *o, QString m) { return s_infoData.sendProperty("bio",o,m); }
int** HArtist::sendBioShort(QObject *o, QString m) { return s_infoData.sendProperty("bioShort",o,m); }
int** HArtist::sendAlbumsNames(QObject *o, QString m,QObject* g) { return s_albumData.sendProperty("albums",o,m,g); }

int** HArtist::sendAlbums(QObject *obj, QString member,int count) {
    connect(obj,SIGNAL(destroyed(QObject*)),this,SLOT(removeFromQueue(QObject*)));
    s_albumQueue.push_back(HTriple(obj,QString(member),count));
    return sendAlbumsNames(this,"sendAlbums_2",obj);
}

void HArtist::sendAlbums_2(QStringList t) {
    QMutexLocker locker(&queueMutex);
    QList<HAlbum*> ret;
    for(int i=0;i<t.size();i++) {
        bool ok=0;
        for(int j=0;j<s_albumQueue.size();j++) {
            if((i<s_albumQueue[j].third)||(s_albumQueue[j].third==-1)) {
                ok=1;
                QMetaObject::invokeMethod(s_albumQueue[j].first,s_albumQueue[j].second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(HAlbum*,&HAlbum::get(getName(),t[i])));
            }
        }
        if(!ok) break;
    }
    s_albumQueue.clear();
}

int** HArtist::sendTrackNames(QObject *o, QString m, QObject* g) { return s_trackData.sendProperty("tracks",o,m,g); }

int** HArtist::sendTracks(QObject *obj, QString member, int count) {
    connect(obj,SIGNAL(destroyed(QObject*)),this,SLOT(removeFromQueue(QObject*)));
    s_trackQueue.push_back(HTriple(obj,QString(member),count));
    return sendTrackNames(this,"sendTracks_2",obj);
}

void HArtist::sendTracks_2(QStringList t) {
    QMutexLocker locker(&queueMutex);
    for(int i=0;i<t.size();i++) {
        bool ok=0;
        for(int j=0;j<s_trackQueue.size();j++) {
            if((i<s_trackQueue[j].third)||(s_trackQueue[j].third==-1)) {
                ok=1;
                QMetaObject::invokeMethod(s_trackQueue[j].first,s_trackQueue[j].second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(HTrack*,&HTrack::get(getName(),t[i])));
            }
        }
        if(!ok) break;
    }
    s_trackQueue.clear();
}

int** HArtist::sendSimilarNames(QObject *o, QString m, QObject* g) { return s_similarData.sendProperty("similar",o,m,g); }

int** HArtist::sendSimilar(QObject *obj, QString member, int count) {
    connect(obj,SIGNAL(destroyed(QObject*)),this,SLOT(removeFromQueue(QObject*)));
    s_similarQueue.push_back(HTriple(obj,QString(member),count));
    return sendSimilarNames(this,"sendSimilar_2",obj);
}

void HArtist::sendSimilar_2(QStringList t) {
    QMutexLocker locker(&queueMutex);
    for(int i=0;i<t.size();i++) {
        bool ok=0;
        for(int j=0;j<s_similarQueue.size();j++) {
            if((i<s_similarQueue[j].third)||(s_similarQueue[j].third==-1)) {
                ok=1;
                QMetaObject::invokeMethod(s_similarQueue[j].first,s_similarQueue[j].second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(HArtist*,&HArtist::get(t[i])));
            }
        }
        if(!ok) break;
    }
    s_similarQueue.clear();
}

int** HArtist::sendShouts(QObject *obj, QString member) {
//    s_shoutData.shoutQueue.push_back(qMakePair(obj,member));
//    s_shoutData.sendData(s_name);
    return 0;
}

int** HArtist::sendSimilarScores(QObject *o, QString m) { return s_similarData.sendProperty("similarScores",o,m); }

void HArtist::sendExtraPics(QObject* o, QString s, int count) {
    return s_extraPictureData.sendPics(o,s,count);
}
//////////////////////////////////////////////////////////////////////////////////////////////


HArtist::HArtist(QString name) : s_name(name), s_infoData(name), s_extraTagData(name), s_albumData(name), s_trackData(name), s_similarData(name), s_extraPictureData(name)
{
    for(int i=0;i<4;i++) s_cachedPixmap[i]=0;
}

HArtist::ArtistInfo::ArtistInfo(QString artist) {
    QMap<QString, QString> params;
    params["method"] = "artist.getInfo";
    params["username"] = lastfm::ws::Username;
    params["artist"] = artist;
    setParams(params);

    QByteArray b=QCryptographicHash::hash(artist.toUtf8()+"ARTISTINFO",QCryptographicHash::Md5).toHex();
    addProperty<QString>("pics::0",b);
    addProperty<QString>("pics::1",b);
    addProperty<QString>("pics::2",b);
    addProperty<QString>("pics::3",b);

    addProperty<QString>("bioShort",b);
    addProperty<QString>("bio",b);
    addProperty<QStringList>("tagNames",b);
    addProperty<int>("listeners",b);
    addProperty<int>("playCount",b);
    addProperty<int>("userPlayCount",b);
}
bool HArtist::ArtistInfo::process(const QString &data) {
    try {
        QStringList tags;
        QDomDocument doc;
        doc.setContent( data );

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    if ( m.nodeName() == "image") {
                        if(m.attributes().namedItem("size").nodeValue()=="small") setProperty("pics::0", l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="medium") setProperty("pics::1", l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="large") setProperty("pics::2", l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="mega") setProperty("pics::3", l.toText().data() );
                    }
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "summary" ) {
                            QString tBioShort=k.toText().data();

                            tBioShort.remove( QRegExp( "<[^>]*>" ) );
                            tBioShort.remove( QRegExp( "&[^;]*;" ) );

                            setProperty("bioShort", HObject::eliminateHtml(tBioShort) );
                        }
                        else if ( l.nodeName() == "content" ) setProperty("bio", HObject::eliminateHtml(k.toText().data()) );
                        else if ( l.nodeName() == "listeners") setProperty("listeners", k.toText().data().toInt() );
                        else if ( l.nodeName() == "playcount") setProperty("playCount", k.toText().data().toInt() );
                        else if ( l.nodeName() == "userplaycount" ) setProperty("userPlayCount", k.toText().data().toInt() );
                        for (QDomNode j = k.firstChild(); !j.isNull(); j = j.nextSibling()) {
                            if ( l.nodeName() == "tag" && k.nodeName() == "name" ) tags.append( j.toText().data() );
                        }
                    }
                }
            }
        }
        setProperty("tagNames", tags );

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
        return 0;
    }
    return 1;
}

HArtist::ArtistAlbumData::ArtistAlbumData(QString artist) {
    QMap<QString, QString> params;
    params["method"] = "artist.getTopAlbums";
    params["username"] = lastfm::ws::Username;
    params["artist"] = artist;
    setParams(params);

    QByteArray b=QCryptographicHash::hash(artist.toUtf8()+"ARTISTALBUMS",QCryptographicHash::Md5).toHex();
    addProperty<QStringList>("albums",b);
}

bool HArtist::ArtistAlbumData::process(const QString &data) {
    try {
        QDomDocument doc;
        doc.setContent( data );

        QStringList albums;

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "name" ) albums.push_back( k.toText().data() );
                    }
                }
            }
        }

        setProperty("albums",albums);

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
        return 0;
    }
    return 1;
}

HArtist::ArtistExtraTagData::ArtistExtraTagData(QString artist) {
    QMap<QString, QString> params;
    params["method"] = "artist.getTopTags";
    params["artist"] = artist;
    setParams(params);

    QByteArray b=QCryptographicHash::hash(artist.toUtf8()+"ARTISTEXTRATAGS",QCryptographicHash::Md5).toHex();
    addProperty<QStringList>("tagNames",b);
}

bool HArtist::ArtistExtraTagData::process(const QString &data) {
    try {
        QDomDocument doc;
        doc.setContent( data );
        QStringList tags;

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "name" ) tags.push_back( k.toText().data() );
                    }
                }
            }
        }
        setProperty("tagNames",tags);

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
        return 0;
    }
    return 1;
}

HArtist::ArtistTrackData::ArtistTrackData(QString artist) {
    QMap<QString, QString> params;
    params["method"] = "artist.getTopTracks";
    params["username"] = lastfm::ws::Username;
    params["artist"] = artist;
    setParams(params);

    QByteArray b=QCryptographicHash::hash(artist.toUtf8()+"ARTISTTRACKS",QCryptographicHash::Md5).toHex();
    addProperty<QStringList>("tracks",b);
}

bool HArtist::ArtistTrackData::process(const QString &data) {
    try {
        QDomDocument doc;
        doc.setContent( data );

        QStringList tracks;

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "name" ) tracks.push_back( k.toText().data() );
                    }
                }
            }
        }
        setProperty("tracks",tracks);

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
        return 0;
    }
    return 1;
}

HArtist::ArtistSimilarData::ArtistSimilarData(QString artist) {
    QMap<QString, QString> params;
    params["method"] = "artist.getSimilar";
    params["username"] = lastfm::ws::Username;
    params["artist"] = artist;
    setParams(params);

    QByteArray b=QCryptographicHash::hash(artist.toUtf8()+"ARTISTSIMILAR",QCryptographicHash::Md5).toHex();
    addProperty<QStringList>("similar",b);
    addProperty<QList<double> >("similarScores",b);
}

bool HArtist::ArtistSimilarData::process(const QString &data) {
    try {
        QDomDocument doc;
        doc.setContent( data );
        QStringList similar;
        QList<double> score;

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "name" ) similar.push_back( k.toText().data() );
                        if ( l.nodeName() == "match" ) score.push_back( k.toText().data().toDouble() );
                    }
                }
            }
        }

        if(similar.size()) {
            setProperty("similar",similar);
            setProperty("similarScores",score);
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
        return 0;
    }
    return 1;
}

ExtraPictureData::ExtraPictureData(QString artist) : s_errored(0), got_urls(0),getting(0), s_artist(artist), sett("Nettek","Hathor_artistExtraImages") {}

void ExtraPictureData::sendPics(QObject *o, QString m, int c) {
    connect(o,SIGNAL(destroyed(QObject*)),this,SLOT(removeFromQueue(QObject*)));
    if(got_urls) {
        Q_ASSERT(o);
        s_queue.push_back(HEPTriplet(o,m,c));
        procQueue();
        return;
    } else {
        if(o) {
            s_queue.push_back(HEPTriplet(o,m,c));
        }
        if(getting) return;
    }

    getting=1;

    if(sett.value("cache for "+s_artist,0).toInt()==2) {
        pic_urls=sett.value("pic_urls for "+s_artist).toStringList();
        got_urls=1;
        getting=0;
        procQueue();
        return;
    }

    QMap<QString, QString> params;
    params["method"] = "artist.getImages";
    params["username"] = lastfm::ws::Username;
    params["artist"] = s_artist;
    qDebug()<<s_artist;
    QNetworkReply* reply = lastfmext_post( params );
    connect(reply,SIGNAL(finished()),this,SLOT(processPicUrls()));
}

void ExtraPictureData::processPicUrls() {
    QNetworkReply* reply= dynamic_cast<QNetworkReply*>(sender());
    Q_ASSERT(reply);
    if(!reply) return;

    if(!reply->isFinished()||reply->error()!=QNetworkReply::NoError) {
        qDebug()<<"COULD NOT GET PICS"<<reply->readAll();
        getting=0;
        if(!s_errored) {
            s_errored=1;
            sendPics(0,"",0);
        }
        return;
    }

    try {
        QDomDocument doc;
        doc.setContent( QString::fromUtf8(reply->readAll().data()) );

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        for (QDomNode j = k.firstChild(); !j.isNull(); j = j.nextSibling()) {
                            if ( k.nodeName() == "size" && k.attributes().namedItem("name").nodeValue()=="original" &&
                                 (k.attributes().namedItem("width").nodeValue().toInt()*k.attributes().namedItem("height").nodeValue().toInt()<757800)) {
                                pic_urls.push_back( j.toText().data() );
                            }
                        }
                    }
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
    sett.setValue("cache for "+s_artist,2);
    sett.setValue("pic_urls for "+s_artist,pic_urls);

    got_urls=1;
    getting=0;
    procQueue();
}

void ExtraPictureData::procQueue() {
    while(s_queue.size()) {
        int c=s_queue.first().third;
        QString m=s_queue.first().second;
        QObject* o=s_queue.first().first;
        s_queue.pop_front();

        for(int i=0;i<c&&i<pic_urls.size();i++) {
            if(i>=pics.size()) {
                pics.push_back(HCachedPixmap::get(pic_urls[i]));
            }
            pics[i]->send(o,m);
        }
    }
}

void ArtistShoutData::sendData(QString artist) {
    QMutexLocker lock(&mutex);  // DO NOT USE QMetaObject::invokeMethod
    this->artist=artist;

    if(getting) connect(getting,SIGNAL(notify()),this,SLOT(sendData_processQueue()));
    else if(got) sendData_processQueue();
    else {
        // no cache?
        QMap<QString, QString> params;
        params["method"] = "artist.getShouts";
        params["artist"] = artist;
        QNetworkReply* reply = lastfmext_post( params );

        getting=new HRunOnceNotifier;   // !!
        connect(reply,SIGNAL(finished()),this,SLOT(sendData_process()));
    }
}

void ArtistShoutData::sendData_process() {
    QNetworkReply* reply=qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT(reply);

    if(!reply->isFinished()||reply->error()!=QNetworkReply::NoError) {
        qDebug()<<"GOT ERROR - INVALID DATA RECORDED!";
        getting->emitNotify();
        getting=0;
        return;
    }

    try {
        QString body,author,date;
        QDomDocument doc;
        doc.setContent( QString::fromUtf8(reply->readAll().data()) );

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "body" ) body = k.toText().data();
                        else if ( l.nodeName() == "author" ) author = k.toText().data();
                        else if ( l.nodeName() == "date") {
                            date = k.toText().data();
                            shouts.push_back(new HShout(body,HUser::get(author),date));
                        }
                    }
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }

    got=1;    // !!
    getting->emitNotify();
    getting=0;
    sendData_processQueue();
}

void ArtistShoutData::sendData_processQueue() {
    QMutexLocker locker(&mutex_2);
    while(shoutQueue.size()) {
        QPair< QObject*, QString > p=shoutQueue.takeFirst();
        QMetaObject::invokeMethod(p.first,p.second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(QList<HShout*>,shouts));
    }
    shoutQueue.clear();
}
