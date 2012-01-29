#include "htrackcontext.h"
#include "ui_htrackcontext.h"
#include "htrackbox.h"
#include "htrackbox.h"
#include "htagbox.h"
#include "hshoutbox.h"
#include "hartistbox.h"
#include "halbumbox.h"
#include <QRect>
#include <QScrollBar>
#include <QMenu>

#include "hrdioprovider.h"

HTrackContext::HTrackContext(HTrack& rep, QWidget *parent) :
    QWidget(parent),
    s_rep(rep),
    s_albumLoadCount(0),
    s_artistLoadCount(0),
    s_trackLoadCount(0),
    s_tagLoadCount(0),
    s_similarLoadCount(0),
    s_shoutLoadCount(0),
    s_contentSet(0),
    ui(new Ui::HTrackContext)
{
    ui->setupUi(this);
    s_rep.sendSummary(this,"setContent");
    s_rep.sendPlayCount(this,"setPlayCount");
    s_rep.sendListenerCount(this,"setListenerCount");
    s_rep.sendUserPlayCount(this,"setUserPlayCount");
    ui->label_track->setText(s_rep.getTrackName());

    connect(ui->label_moreDescription,SIGNAL(linkActivated(QString)),this,SLOT(showMoreBio()));

    connect(ui->button_play,SIGNAL(clicked()),this,SLOT(playTrack()));
//    connect(ui->button_more,SIGNAL(clicked()),this,SLOT(playSimilar()));

    ui->widget_artist->setLayout(new QVBoxLayout);
    ui->widget_albums->setLayout(new QVBoxLayout);
    ui->widget_tags->setLayout(new QVBoxLayout);
    ui->widget_similar->setLayout(new QVBoxLayout);
    ui->widget_comments->setLayout(new QVBoxLayout);

    loadArtist();
    loadAlbum();
    loadSimilar();
    loadShouts();
    loadTags();

    connect(ui->label_moreTracks,SIGNAL(linkActivated(QString)),this,SLOT(loadSimilar()));

    connect(ui->label_moreTags,SIGNAL(linkActivated(QString)),this,SLOT(loadTags()));

    connect(ui->label_moreShoutbox,SIGNAL(linkActivated(QString)),this,SLOT(loadShouts()));

    ui->label_you->setPixmap(HUser::get(lastfm::ws::Username).getPic(HUser::Medium).scaledToWidth(70,Qt::SmoothTransformation));

    ui->frame_header->adjustSize();

    // menus
//    QMenu* playMenu=new QMenu;
//    playMenu->addAction("Queue",this,SLOT(playTrack()));
//    playMenu->addAction("Replace",this,SLOT(playReplacing()));
//    ui->button_play->setMenu(playMenu);

//    QMenu* moreMenu=new QMenu;
//    moreMenu->addAction("Queue five similar tracks",this,SLOT(playSimilar()));
//    moreMenu->addAction("Queue ten similar tracks",this,SLOT(playMoreSimilar()));
//    moreMenu->addAction("Replace queue with ten songs",this,SLOT(playMoreSimilarReplacing()));
//    ui->button_more->setMenu(moreMenu);

    s_rep.sendBpm(this,"setBpm");
    s_rep.sendValence(this,"setValence");
    s_rep.sendAggression(this,"setAggression");
    s_rep.sendAvgLoudness(this,"setAvgLoudness");
    s_rep.sendPercussiveness(this,"setPercussiveness");
    s_rep.sendKey(this,"setKey");
    s_rep.sendEnergy(this,"setEnergy");
    s_rep.sendPunch(this,"setPunch");
    s_rep.sendSoundCreativity(this,"setSoundCreativity");
    s_rep.sendChordalClarity(this,"setChordalClarity");
    s_rep.sendTempoInstability(this,"setTempoInstability");
    s_rep.sendRhythmicIntricacy(this,"setRhythmicIntricacy");
    s_rep.sendSpeed(this,"setSpeed");
}

HTrackContext::~HTrackContext()
{
    delete ui;
}

void HTrackContext::showMoreBio()
{
    ui->label_moreDescription->setText("Loading...");
    s_rep.sendContent(this,"setContent");
}

void HTrackContext::loadArtist()
{
    ui->label_moreArtists->setText("<p align=\"right\"><i>Loading...</i></p>");
    {
        HArtistBox* ab=new HArtistBox(s_rep.getArtist());
        QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
        pa->setStartValue(0);
        pa->setEndValue(ab->sizeHint().height());
        pa->setDuration(500);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
        ui->widget_artist->layout()->addWidget(ab);
    }
    s_artistLoadCount=1;
    ui->label_moreArtists->hide();
}

void HTrackContext::loadAlbum()
{
    ui->label_moreAlbums->setText("<p align=\"right\"><i>Loading...</i></p>");
    s_rep.sendAlbums(this,"setAlbums");
}

void HTrackContext::loadTags()
{
    ui->label_moreTags->setText("<p align=\"right\"><i>Loading...</i></p>");
    if(s_tagLoadCount) s_rep.sendMoreTags(this,"setTags");
    else s_rep.sendTags(this,"setTags");
}

void HTrackContext::loadShouts()
{
    ui->label_moreShoutbox->setText("<p align=\"right\"><i>Loading...</i></p>");
    s_rep.sendShouts(this,"setShouts");
}

void HTrackContext::loadSimilar()
{
    ui->label_moreTracks->setText("<p align=\"right\"><i>Loading...</i></p>");
    s_rep.sendSimilar(this,"setSimilar");
}

void HTrackContext::playTrack() {
    HPlayer::singleton()->clear();
    HPlayer::singleton()->getStandardQueue()->queue(&s_rep);
}

void HTrackContext::setContent(QString t) {
    ui->label_description->setText(t.size()?t:"No article is available for this track.");
    ui->label_description->adjustSize();

    if(s_contentSet) ui->label_moreDescription->hide();
    s_contentSet=1;
}

void HTrackContext::setPlayCount(int t) {
    s_playCountCache=t;
    updateBoxes();
}

void HTrackContext::setListenerCount(int t) {
    s_listenerCountCache=t;
    updateBoxes();
}

void HTrackContext::updateBoxes() {
    ui->label_playcount->setText("<B>"+QString::number(s_playCountCache)+"</B> plays by <B>"+QString::number(s_listenerCountCache)+"</B> users");
}

void HTrackContext::setUserPlayCount(int t) {
    ui->label_userplaycount->setText("<B>"+QString::number(t)+"</B> plays in your library");
}

void HTrackContext::setAlbums(QList<HAlbum *> t) {
    if(t.size())
    {
        HAlbumBox* ab=new HAlbumBox(*t[0]);
        QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
        pa->setStartValue(0);
        pa->setEndValue(ab->sizeHint().height());
        pa->setDuration(500);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
        ui->widget_albums->layout()->addWidget(ab);
    }
    s_albumLoadCount=1;
    ui->label_moreArtists->hide();
}

void HTrackContext::setTags(QList<HTag *> tags) {
    int i;
    int toLoad=s_tagLoadCount?s_tagLoadCount*2:4;
    for(i=s_tagLoadCount;i<tags.size()&&i-s_tagLoadCount<toLoad;i++) {
        tags[i]->getContent();    //CACHE
        HTagBox* ab=new HTagBox(*tags[i]);
        QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
        pa->setStartValue(0);
        pa->setEndValue(40);
        pa->setDuration(500);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
        ui->widget_tags->layout()->addWidget(ab);
    }
    if(i-s_tagLoadCount!=toLoad) {
        ui->label_moreTags->hide();
    } else {
        ui->label_moreTags->setText(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> "
            "p, li { white-space: pre-wrap; }"
            "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
            "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"a1\">"
            "<span style=\" text-decoration: underline; color:#0057ae;\">more...</span></a></p></body></html>");
    }
    s_tagLoadCount=i;
}

void HTrackContext::setShouts(QList<HShout *> shouts) {
    int i;
    int toLoad=s_shoutLoadCount?s_shoutLoadCount*2:10;
    for(i=s_shoutLoadCount;i<shouts.size()&&i-s_shoutLoadCount<toLoad;i++) {
        shouts[i]->getShouter().getPic(HUser::Medium);    //CACHE
        HShoutBox* ab=new HShoutBox(*shouts[i],this);
        {
            QPropertyAnimation* pa=new QPropertyAnimation(ab,"minimumHeight");
            pa->setStartValue(0);
            pa->setEndValue(ab->heightForWidth(ui->widget_comments->width()-20));
            pa->setDuration(500);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        }
        {
            QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
            pa->setStartValue(0);
            pa->setEndValue(ab->heightForWidth(ui->widget_comments->width()-20));
            pa->setDuration(500);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        }
        ui->widget_comments->layout()->addWidget(ab);

    }
    if(i-s_shoutLoadCount!=toLoad) {
        ui->label_moreShoutbox->hide();
    } else {
        ui->label_moreShoutbox->setText(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> "
            "p, li { white-space: pre-wrap; }"
            "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
            "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"a1\">"
            "<span style=\" text-decoration: underline; color:#0057ae;\">more...</span></a></p></body></html>");
    }
    s_shoutLoadCount=i;
}

void HTrackContext::setSimilar(QList<HTrack *> tracks) {
    int i;
    int toLoad=s_similarLoadCount?s_similarLoadCount*2:4;
    for(i=s_similarLoadCount;i<tracks.size()&&i-s_similarLoadCount<toLoad;i++) {
        HTrackBox* ab=new HTrackBox(*tracks[i]);
        QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
        pa->setStartValue(0);
        pa->setEndValue(40);
        pa->setDuration(500);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
        ui->widget_similar->layout()->addWidget(ab);
    }
    if(i-s_similarLoadCount!=toLoad) {
        ui->label_moreTracks->hide();
    } else {
        ui->label_moreTracks->setText(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> "
            "p, li { white-space: pre-wrap; }"
            "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
            "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"a1\">"
            "<span style=\" text-decoration: underline; color:#0057ae;\">more...</span></a></p></body></html>");
    }

    s_similarLoadCount=i;
}

void HTrackContext::setSlideshow(QWidget *w) {
    ui->content->addWidget(w);
    ui->content->setAlignment(w,Qt::AlignCenter);
    ui->scrollArea_3->hide();
    w->adjustSize();
    ui->frame_header->adjustSize();
}

void HTrackContext::setBpm(double d) {
    s_bpm=d;
    ui->label->setText(s_character+QString(s_character.size()?", ":"")+"<B>"+QString::number(s_bpm)+"</B> bpm");
}

void HTrackContext::setValence(double d) {
    if(d<-0.3) s_character+=QString(s_character.size()?", ":" ")+"moody";
    if(d>-0.01) s_character+=QString(s_character.size()?", ":" ")+"happy";
}

void HTrackContext::setAggression(double d) {
    if(d>-0.985) s_character+=QString(s_character.size()?", ":" ")+"aggressive";
    if(d<-0.99) s_character+=QString(s_character.size()?", ":" ")+"smooth";
}

void HTrackContext::setAvgLoudness(double d) {
    if(d>-16.5) s_character+=QString(s_character.size()?", ":" ")+"loud";
}

void HTrackContext::setPercussiveness(double d) {
    if(d>0.65) s_character+=QString(s_character.size()?", ":" ")+"percussive";
    setBpm(s_bpm);
}

void HTrackContext::setKey(int z) {

}

void HTrackContext::setEnergy(double d) {
    if(d>0.7) s_character+=QString(s_character.size()?", ":" ")+"energetic";
    if(d<0.35) s_character+=QString(s_character.size()?", ":" ")+"calm";
    setBpm(s_bpm);
}

void HTrackContext::setPunch(double d) {
    if(d>0.45) s_character+=QString(s_character.size()?", ":" ")+"punchy";
    if(d<0.2) s_character+=QString(s_character.size()?", ":" ")+"not punchy";
}

void HTrackContext::setSoundCreativity(double d) {
//        qDebug()<<d<<"(loudness)";

    if(d>0.55) s_character+=QString(s_character.size()?", ":" ")+"interesting";
    if(d<0.4) s_character+=QString(s_character.size()?", ":" ")+"conventional";

}

void HTrackContext::setChordalClarity(double d) {
    if(d>0.6) s_character+=QString(s_character.size()?", ":" ")+"clear";
    if(d<0.4) s_character+=QString(s_character.size()?", ":" ")+"not music";

}

void HTrackContext::setTempoInstability(double d) {
}

void HTrackContext::setRhythmicIntricacy(double d) {
    if(d>0.6) s_character+=QString(s_character.size()?", ":" ")+"rhymic";
    if(d<0.3) s_character+=QString(s_character.size()?", ":" ")+"not rhymic";
}

void HTrackContext::setSpeed(double d) {

}