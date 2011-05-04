#include <QByteArray>

#include "LogSender.h"

LogSender::LogSender()
{
    m_networkManager = NULL;
}

LogSender::~LogSender()
{
    if(m_networkManager)
    {
        delete m_networkManager;
        m_networkManager = NULL;
    }
}

bool LogSender::init()
{
    bool bRet = false;
    // Initialize proxy settings
    QUrl proxyUrl(getenv("http_proxy"));
    if(proxyUrl.isValid())
    {
    qDebug() << "Setting proxy::" << proxyUrl.host() << proxyUrl.port();
        QNetworkProxy proxy(QNetworkProxy::HttpProxy, proxyUrl.host(), proxyUrl.port(), proxyUrl.userName(), proxyUrl.password());
        m_networkManager->setProxy(proxy);
    }

    bRet = QObject::connect(m_networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    bRet = QObject::connect(m_networkManager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)),
            this, SLOT(slotProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));
    return bRet;
}

void LogSender::deInit()
{
}

void LogSender::setTextToSend(const QString &text)
{
	this->textToSend = QString(text);
	qDebug() << "Text to send set to: " << this->textToSend;
}

void LogSender::setGUID(const QString &guid)
{
	this->runGUID = QString(guid);
	qDebug() << "GUID set to: " << this->runGUID;
}

void LogSender::run()
{
	if(m_networkManager!=NULL)
	{
		delete m_networkManager;
		m_networkManager=0;
	}
	m_networkManager = new QNetworkAccessManager();
	m_networkManager->moveToThread(this);
	this->init();

    qDebug("Sending request.............");

    QUrl url("http://pistelogs.appspot.com/api/depositlog");

    QNetworkRequest request(url);

    request.setRawHeader("Host", "pistelogs.appspot.com");

    QByteArray postData;
    QUrl params;
    params.addQueryItem("log_data", this->textToSend);
    params.addQueryItem("run_guid", this->runGUID);
    params.addQueryItem("user_id","john");
    postData = params.encodedQuery();

    // Following was needed during debug:
    //qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");

    request.setHeader(QNetworkRequest::ContentLengthHeader, postData.size());
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    QNetworkReply *reply = m_networkManager->post(request, postData);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                    this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
                 this, SLOT(slotSslErrors(QList<QSslError>)));
    exec();
}

void LogSender::replyFinished(QNetworkReply* reply)
{
    // TODO reply could be showed in the app UI also
    qDebug() << "Send finished";
    reply->deleteLater();
    exit();
}

void LogSender::slotError(QNetworkReply::NetworkError error)
{
    qDebug() << "Error!!" << error;
    emit sendError("Network Error");
    exit();
}

void LogSender::slotSslErrors(QList<QSslError> /*errorList*/)
{
    qDebug() << "SSL errors!!";
    emit sendError("SSL Errors");
    exit();
}

void LogSender::slotProxyAuthenticationRequired(const QNetworkProxy& /*proxy*/, QAuthenticator* /*auth*/)
{
    qDebug() << "Proxy authentication needed, error!!";
    emit sendError("Proxy Authentication Needed");
    exit();
}
