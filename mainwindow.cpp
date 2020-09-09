#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);

    this->setWindowTitle("SMTP-client");

    setDefaultSettings();

    smtp = std::make_unique<Smtp>();

    connect(ui->attachButton, &QPushButton::clicked, this, [=] {
        ui->attachedLabel->setVisible(true);

        file = QFileDialog::getOpenFileName(nullptr, "Attach files", QDir::currentPath(), "All files (*.*)");

        QString filesName = file;
        filesName += "\n";
        ui->attachedFiles->setText(filesName);
    });
    connect(ui->sendButton, &QPushButton::clicked, this, [=] {
        ui->textEdit->clear();
        consoleText.clear();

        mailData = smtp->getMailData();
        fillMailData(mailData);
        smtp->setMailData(mailData);

        smtp->sendEmail(!file.isEmpty());

        ui->sendButton->setDisabled(true);
    });
    connect(smtp.get(), &Smtp::displayState, this, [=](const QString &text) {
        consoleText += text;
        ui->textEdit->setText(consoleText);
    });
    connect(smtp.get(), &Smtp::finish, this, [=]() {
        ui->sendButton->setEnabled(true);
        ui->attachedLabel->setVisible(false);
        ui->attachedFiles->setText("");
    });
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setDefaultSettings() {
    setAttachButtonImage();
    setDefaulValues();
    ui->attachedLabel->setVisible(false);
}

void MainWindow::setAttachButtonImage() {
    QPixmap pixmap(":/image/resources/attach.png");
    QIcon ButtonIcon(pixmap);
    ui->attachButton->setIcon(ButtonIcon);
    ui->attachButton->setIconSize(pixmap.rect().size());
}

void MainWindow::setDefaulValues() {
    ui->hostEdit->setText("mail.miluris.ru");
    ui->portEdit->setText("25");
    ui->mailToLine->setText("konstantin.soloviev@miluris.ru");
    ui->mailFromLine->setText("konstantin.soloviev@miluris.ru");
    ui->subjectLine->setText("");
}

void MainWindow::fillMailData(Smtp::MailData &mailData) {
    mailData.host = ui->hostEdit->text();
    mailData.port = ui->portEdit->text().toInt();
    mailData.senderAddress = ui->mailFromLine->text();
    mailData.recipientAddress = ui->mailToLine->text();
    mailData.subject = ui->subjectLine->text();
    mailData.file = file;
    mailData.text = ui->text->toPlainText();
}
