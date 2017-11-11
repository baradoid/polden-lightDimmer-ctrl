#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QTimer>
#include <QSettings>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void on_pushButtonConnect_clicked();

    void handleSocketConnected();
    void handleSocketHostFound();
    void handleSocketError(QAbstractSocket::SocketError);
    void handleSocketStateChanged(QAbstractSocket::SocketState);
    void handleSendHB();
    void handleSendTimer();

    void handleBytesWritten(qint64);
    void handleReadyRead();
    void on_verticalSlider_sliderMoved(int position);

    void on_lineEditHost_editingFinished();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButtonPonOff1_clicked();

    void on_pushButtonPonOn1_clicked();

    void on_pushButtonPonOnAll_clicked();

    void on_pushButtonPonOffAll_clicked();

    void on_pushButtonOn2_clicked();

    void on_pushButtonOff2_clicked();

    void on_pushButtonOn3_clicked();

    void on_pushButtonOff3_clicked();

    void on_pushButtonOn4_clicked();

    void on_pushButtonOff4_clicked();

private:
    Ui::Dialog *ui;
    QTcpSocket tcpSock;
    QTimer hbTimer, sendTimer;
    QSettings settings;
    int pos, sendCnt;

    void createMsg(QByteArray &ba, int d);
    quint64 lastSendTime;

};

#endif // DIALOG_H
