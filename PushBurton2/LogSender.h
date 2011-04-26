#ifndef LOGSENDER_H
#define LOGSENDER_H

#include <QThread>
#include <QtCore>
#include <QtNetwork>

class LogSender : public QThread
{
Q_OBJECT
public:
    LogSender();
    ~LogSender();
    bool init();
    void deInit();
    void setTextToSend(const QString& text);

    virtual void run();
signals:
    void sendError(const QString& sendError);
private slots:
    void replyFinished(QNetworkReply* reply);
    void slotError(QNetworkReply::NetworkError error);
    void slotSslErrors(QList<QSslError> sslError);
    void slotProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*);
private:
    QNetworkAccessManager *m_networkManager;
    QString textToSend;
};

#endif // LOGSENDER_H
