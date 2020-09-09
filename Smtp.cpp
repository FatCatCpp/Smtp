#include "smtp.h"
#include <QFile>
#include <QDir>

Smtp::Smtp(QObject *parent): QObject(parent) {
    socket = std::make_unique<QTcpSocket>(this);
    timeout = 3000;

    connect(socket.get(), SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(errorReceived(QAbstractSocket::SocketError)));
    connect(socket.get(), &QTcpSocket::readyRead, this, &Smtp::readyRead);
}

Smtp::~Smtp() {

}

void Smtp::outputInformation(int state, QString respMessage, QString respCode) {
    QString text;

    switch (state) {
        case State::OPEN: {
            text.append("OPEN process:\t");
            break;
        }
        case State::HANDSNAKE: {
            text.append("HANDSNAKE process:\t");
            break;
        }
        case State::AUTH: {
            text.append("AUTH process:\t");
            break;
        }
        case State::USER: {
            text.append("USER process:\t");
            break;
        }
        case State::PASS: {
            text.append("PASS process:\t");
            break;
        }
        case State::MAIL_FROM: {
            text.append("MAIL_FROM:\t");
            break;
        }
        case State::RCPT_TO: {
            text.append("RCPT_TO:\t");
            break;
        }
        case State::DATA: {
            text.append("DATA process:\t");
            break;
        }
        case State::BODY: {
            text.append("BODY process:\t");
            break;
        }
        case State::QUIT: {
            text.append("QUIT process:\t");
            break;
        }
        case State::CLOSE: {
            text.append("CLOSE process:\t");
            break;
        }
    }
    text.append("Server response code:" + respCode);
    text.append("Server response: " + respMessage + "\n\n");

    qDebug() << text;

    emit displayState(text);
}

void Smtp::readyRead() {
    QString responseLine;

    do {
        responseLine = socket->readLine();
        response += responseLine;
    }
    while (socket->canReadLine() && ' ' != responseLine.at(3));

    responseLine.truncate(3);

    outputInformation(state, response, responseLine);

    switch (getState()) {
        case State::OPEN: {
            if (Request::SERVER_IS_READY == responseLine.toInt()) {
                *stream << "EHLO localhost" << "\r\n";
                stream->flush();

                state = State::MAIL_FROM;
            } else {
                state = State::ERROR;
            }

            break;
        }
        case State::MAIL_FROM: {
            if (Request::COMMAND_OK == responseLine.toInt()) {
                qDebug() << "MAIL FROM:<" << mailData.senderAddress << ">";
                *stream << "MAIL FROM:<" << mailData.senderAddress << ">\r\n";
                stream->flush();
                state = State::RCPT_TO;
            } else {
                state = State::ERROR;
            }

            break;
        }
        case State::RCPT_TO: {
            if (Request::COMMAND_OK == responseLine.toInt()) {
                *stream << "RCPT TO:<" << mailData.recipientAddress << ">\r\n";
                stream->flush();
                state = State::DATA;
            } else {
                state = State::ERROR;
            }

            break;
        }
        case State::DATA: {
            if (Request::COMMAND_OK == responseLine.toInt()) {
                *stream << "DATA\r\n";
                stream->flush();
                state = State::BODY;
            } else {
                state = State::ERROR;
            }

            break;
        }
        case State::BODY: {
            if (Request::START_CONTENT_TRANSFER == responseLine.toInt()) {
                *stream << message << "\r\n.\r\n";
                stream->flush();
                state = State::QUIT;
            } else {
                state = State::ERROR;
            }

            break;
        }
        case State::QUIT: {
            if (Request::COMMAND_OK == responseLine.toInt()) {
                *stream << "QUIT\r\n";
                stream->flush();
                state = State::CLOSE;
                emit status(tr("Message sent"));
                emit finish();
            } else {
                state = State::ERROR;
            }

            break;
        }
        case State::CLOSE: {
            break;
        }
        case State::ERROR: {
            emit status("Error");
            break;
        }
        default: {
            qDebug() << "Unexpected reply from SMTP server:\n\n" << response;
            state = State::CLOSE;
            emit status(tr("Failed to send message"));
        }
    }

    response.clear();
}

void Smtp::sendEmail(bool attach) {
    messageText = mailData.text;

    message = "To: " + mailData.recipientAddress + "\n";
    message.append("From: " + mailData.senderAddress + "\n");
    message.append("Subject: " + mailData.subject + "\n");

    fileName = mailData.file;

    if (attach) {
        message.append("MIME-Version: 1.0\n");
        message.append("Content-Type: multipart/mixed; boundary=splitter\n\n");

        message.append("--splitter\n");
        message.append("Content-Type: text/plain\n\n");
        message.append(messageText);
        message.append("\n\n");

        message.append("--splitter\n");

        QFile file(fileName);
        if (!file.exists()) {
            qDebug() << "File not exist";
            return;
        }
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Couldn't open the file";
            return;
        }

        QByteArray bytes = file.readAll();
        file.close();

        QFileInfo fi(fileName);
        message.append("Content-Type: application/octet-stream\nContent-Disposition: attachment; filename=" +
                fi.fileName() + ";\nContent-Transfer-Encoding: base64\n\n");
        message.append(bytes.toBase64());
        message.append("\n");

        message.append("--splitter--\n");

    } else {
        message.append("\n\n");
        message.append(messageText);
    }

    message.replace(QString::fromLatin1("\n"), QString::fromLatin1("\r\n"));
    message.replace(QString::fromLatin1("\r\n.\r\n"), QString::fromLatin1("\r\n..\r\n"));

    qDebug() << message;

    state = State::OPEN;
    socket->connectToHost(mailData.host, mailData.port);
    if (!socket->waitForConnected(timeout)) {
         qDebug() << "=====" << socket->errorString();
    }

    stream = std::make_unique<QTextStream>(socket.get());
}

void Smtp::errorReceived(QAbstractSocket::SocketError socketError) {
    qDebug() << "Error: " << socketError;
}
