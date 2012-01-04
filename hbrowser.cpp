/****************************************************
HBrowser.cpp

                    Part of Hathor
        Copyright (C) Joshua Netterfield 2011

                 All rights reserved.
*****************************************************/

#include "hbrowser.h"
#include <QtWebKit/QWebFrame>
#include <QGridLayout>
#include <QDebug>
#include <QtWebKit/QWebElement>
#include <QtOAuth>
#include <QApplication>
#include <QDebug>
#include <QTimer>

HBrowser::HBrowser(QWidget *parent)
    : QWidget(parent), s_webView(new QWebView(this))
{
    setWindowTitle("Hathor");
    setLayout(new QGridLayout);
    layout()->addWidget(s_webView);
    s_webView->setPage(new HWebPage);
    s_webView->settings()->setAttribute(QWebSettings::PluginsEnabled,true);
    connect(s_webView->page(),SIGNAL(loadFinished(bool)),this,SLOT(loadFinishedLogic(bool)));
    connect(s_webView,SIGNAL(urlChanged(QUrl)),this,SLOT(urlChangedLogic(QUrl)));
}

HBrowser::~HBrowser()
{

}

QString HBrowser::html() const
{
    return s_webView->page()->mainFrame()->toHtml();
}

bool HBrowser::htmlContains(const QString& contains) const
{
    return s_webView->page()->mainFrame()->toHtml().contains(contains);
}

QVariant HBrowser::doJS(const QString &js)
{
    return s_webView->page()->mainFrame()->evaluateJavaScript(js);
}

void HBrowser::loadPage(QString page)
{
    s_webView->page()->mainFrame()->setUrl(QUrl(page));
}

void HBrowser::loadFinishedLogic(bool s)
{
    if(!s)
    {
        QWebPage::ErrorPageExtensionReturn ret;
        s_webView->page()->extension(QWebPage::ErrorPageExtension,0,&ret);

        if(ret.content.isEmpty())
        {
            s_webView->setHtml("<center><b>An unknown error occured in loading the webpage.</B></center>");
        }
        else
        {
            s_webView->setUrl(ret.baseUrl);
        }
    }
    else
    {
        QWebFrame *frame = s_webView->page()->mainFrame();
        elements.clear();
        elements.push_back(frame->documentElement().firstChild());
        for(int i=0;i<elements.size();i++)
        {
            if(!elements[i].nextSibling().isNull())
            {
                elements.push_back(elements[i].nextSibling());
            }
            if(!elements[i].firstChild().isNull())
            {
                elements.push_back(elements[i].firstChild());
            }
        }
    }
    emit ready();
}

void HBrowser::urlChangedLogic(QUrl)
{
//    s_webPath->setText(url.toString());
}

void HBrowser::setInput(QString input, QString value)
{
    if(input.startsWith("#"))
    {
        input.remove(0,1);
    }
    for(int i=0;i<elements.size();i++)
    {
        if(/*elements[i].localName()=="input"&&*/elements[i].attribute("id")==input)
        {
            elements[i].evaluateJavaScript("this.focus()");
            elements[i].evaluateJavaScript("this.value=\""+value+"\"");
            elements[i].evaluateJavaScript("this.blur()");
            return;
        }
    }
    qWarning()<<"Could not find an input named"<<input<<"and give it a value of"<<value;
}

void HBrowser::clickInput(QString input)
{
    if(input.startsWith("#"))
    {
        input.remove(0,1);
    }
    for(int i=0;i<elements.size();i++)
    {
        if(elements[i].localName()=="input"&&elements[i].attribute("id")==input)
        {
            elements[i].evaluateJavaScript("this.click()");
            return;
        }
    }
    qWarning()<<"Could not click"<<input;
}

QStringList HBrowser::getInputs()
{
    QStringList ret;
    for(int i=0;i<elements.size();i++)
    {
        if(elements[i].localName()=="input")
        {
            ret.push_back(elements[i].attribute("id"));
        }
    }
    return ret;
}

QMultiMap<QByteArray,QByteArray> HBrowser::request(const QByteArray& consumerKey,const QByteArray& consumerSecret,const QString& url,QMultiMap<QByteArray,QByteArray> params)
{
    QOAuth::Interface a;
    a.setConsumerKey(consumerKey);
    a.setConsumerSecret(consumerSecret);
    return a.requestToken(url,QOAuth::POST,QOAuth::HMAC_SHA1,params);
}

QMultiMap<QByteArray,QByteArray> HBrowser::request(const QByteArray& consumerKey,const QByteArray& consumerSecret,const QString& url,QMultiMap<QByteArray,QByteArray> params,const QByteArray &token,const QByteArray &tokenShared)
{
    QOAuth::Interface a;
    a.setConsumerKey(consumerKey);
    a.setConsumerSecret(consumerSecret);
    return a.accessToken(url,QOAuth::POST,token,tokenShared,QOAuth::HMAC_SHA1,params);
}
