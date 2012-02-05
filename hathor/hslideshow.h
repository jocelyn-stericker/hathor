#ifndef HSLIDESHOW_H
#define HSLIDESHOW_H

#include <QWidget>
#include <QList>
#include <QPixmap>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QTime>
#include <QPropertyAnimation>
#include "htrack.h"
#include "hartist.h"

class SlideshowItem : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
    Q_PROPERTY(qreal echoOpacity READ echoOpacity WRITE setEchoOpacity)
    Q_PROPERTY(qreal echoScale READ echoScale WRITE setEchoScale)
    Q_PROPERTY(qreal echoX READ echoX WRITE setEchoX)
    Q_PROPERTY(qreal echoY READ echoY WRITE setEchoY)
    QTime wt,xt,yt,zt;
public slots:
    void setEchoOpacity(qreal opacity){
        if(wt.msecsTo(QTime::currentTime())<50) return;
        wt=QTime::currentTime();
        QGraphicsPixmapItem::setOpacity(opacity);
    }
    void setEchoScale(qreal scale){
        if(xt.msecsTo(QTime::currentTime())<50) return;
        xt=QTime::currentTime();
        QGraphicsPixmapItem::setScale(scale);
    }
    void setEchoX(qreal x) {
        if(yt.msecsTo(QTime::currentTime())<50) return;
        yt=QTime::currentTime();
        setPos(x,y());
    }
    void setEchoY(qreal y) {
        if(zt.msecsTo(QTime::currentTime())<50) return;
        zt=QTime::currentTime();
        setPos(x(),y);
    }

public:
    qreal echoOpacity() { return opacity(); }
    qreal echoScale() { return scale(); }
    qreal echoX() { return x(); }
    qreal echoY() { return y(); }

    SlideshowItem( QPixmap pmap ) : QGraphicsPixmapItem(pmap) {}
};

class HSlideshow : public QGraphicsView
{
    Q_OBJECT
    static QHash<QString, HSlideshow*> s_u;
    HTrack& s_track;
    QGraphicsScene sc;
    int s_i;
    int s_z;
    bool s_done;
    QSize minimumSizeHint() const {
        return sizeHint();
    }
    explicit HSlideshow(HTrack& track,QWidget* parent=0);
public:
    static HSlideshow* getSlideshow(HTrack&);
public slots:
    void newPic();
    void pause() { s_done=1; }
    void resume() { if(!s_done) { s_done=0; QTimer::singleShot(0,this,SLOT(newPic())); } }
};

#endif // HSLIDESHOW_H
