#ifndef SMTP_H
#define SMTP_H

#include <QObject>
#include <QTcpSocket>
#include <QMessageBox>
#include <QDateTime>
#include <QHostInfo>
#include <QTextCodec>

#include <memory>

class Smtp : public QObject {
    Q_OBJECT

public:
    explicit Smtp(QObject *parent = nullptr);
    virtual ~Smtp();

    enum State {
        OPEN,
        HANDSNAKE,
        AUTH,
        USER,
        PASS,
        MAIL_FROM,
        RCPT_TO,
        DATA,
        BODY,
        QUIT,
        CLOSE,
        ERROR
    };

    enum Request {
        SERVER_IS_READY = 220,
        AUTH_OK = 235,
        COMMAND_OK = 250,
        RESPONSE_AUTH_OK = 334,
        START_CONTENT_TRANSFER = 354
    };

    void setState(int state) {
        this->state = state;
    }
    int getState() const {
        return state;
    }

    struct MailData {
        QString host;
        int port;
        QString senderAddress;
        QString recipientAddress;
        QString subject;
        QString file;
        QString text;
    } mailData;

    void setMailData(MailData data) {
        mailData = data;
    }
    MailData getMailData() const {
        return mailData;
    }

    void sendEmail(bool attach);
    void outputInformation(int state, QString respMessage, QString respCode);

signals:
    void status(const QString &status);
    void displayState(const QString &status);
    void finish();

public slots:
    void readyRead();
    void errorReceived(QAbstractSocket::SocketError);

private:
    std::unique_ptr<QTcpSocket> socket;
    std::unique_ptr<QTextStream> stream;

    int state;

    QString message;
    QString response;
    QString messageText;

    int timeout;

    QString errorString;
    QString fileName;
};

#endif // SMTP_H
