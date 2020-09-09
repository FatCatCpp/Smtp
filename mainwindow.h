#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>

#include <memory>

#include "Smtp.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setDefaultSettings();
    void setAttachButtonImage();
    void setDefaulValues();
    void fillMailData(Smtp::MailData &mailData);

private:
    Ui::MainWindow *ui;

    Smtp::MailData mailData;

    QString file;
    QString consoleText;

    std::unique_ptr<Smtp> smtp;
};

#endif // MAINWINDOW_H
