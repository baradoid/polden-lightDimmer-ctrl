#include "dialog.h"
#include "ui_dialog.h"
#include <QHostAddress>
#include <QMetaEnum>
#include <QTime>


Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    tcpSock(this),
    hbTimer(this),
    settings("Murinets", "alrightDimmer"),
    pos(-1),
    sendCnt(0)
{
    ui->setupUi(this);


    QString host = settings.value("host").toString();
    ui->lineEditHost->setText(host);

    ui->verticalSlider->setMaximum(55000);

    connect(&tcpSock, SIGNAL(connected()), this, SLOT(handleSocketConnected()));
    connect(&tcpSock, SIGNAL(hostFound()), this, SLOT(handleSocketHostFound()));
    connect(&tcpSock, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(handleSocketStateChanged(QAbstractSocket::SocketState)));
    connect(&tcpSock, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(handleSocketError(QAbstractSocket::SocketError)));

    connect(&hbTimer, SIGNAL(timeout()), this, SLOT(handleSendHB()));
    connect(&sendTimer, SIGNAL(timeout()), this, SLOT(handleSendTimer()));

    sendTimer.setInterval(50);
    sendTimer.setSingleShot(false);
    sendTimer.start();

    hbTimer.setInterval(30000);
    hbTimer.setSingleShot(true);

}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButtonConnect_clicked()
{
    if(tcpSock.state() == QAbstractSocket::UnconnectedState){
        QString hostIp = ui->lineEditHost->text();
        tcpSock.connectToHost(hostIp, 8899);
    }
}

void Dialog::handleSocketConnected()
{
    qDebug() << "handleSocketConnected";
    hbTimer.start();
}

void Dialog::handleSocketHostFound()
{
    qDebug() << "handleSocketHostFound";
}

void Dialog::handleSocketError(QAbstractSocket::SocketError e)
{
    qDebug() << "handleSocketError" << e;
}

void Dialog::handleSocketStateChanged(QAbstractSocket::SocketState ss)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketState>();
//    qDebug() << metaEnum.valueToKey(ss);

//    qDebug() << "handleSocketStateChanged" << ss;
//    QString str = QString("%1").arg(ss);
//    ui->lineEditStatus->setText(metaEnum.valueToKey(ss));
//    if(ss == QAbstractSocket::ConnectedState){
//    }
    QString t = QTime::currentTime().toString("hh:mm:ss.zzz");
    ui->plainTextEditStatus->appendPlainText(t + "> "+ metaEnum.valueToKey(ss));
}

void Dialog::on_verticalSlider_sliderMoved(int position)
{
    //qDebug() << position;
    if(pos == -1){
        pos = position;
        sendCnt = 2;
    }

}

void Dialog::handleSendHB()
{
    if(tcpSock.state() == QAbstractSocket::ConnectedState){
        qDebug() << "hb";
        QByteArray ba;
        ba.append((char)0xff);
        tcpSock.write(ba);

        QString t = QTime::currentTime().toString("hh:mm:ss.zzz");
        ui->plainTextEditStatus->appendPlainText(t + "> hb");

        hbTimer.start();
    }
}

void Dialog::on_lineEditHost_editingFinished()
{
    QString host = ui->lineEditHost->text();
    qDebug() << host;
    settings.setValue("host", host);
    //settings.
}

void Dialog::handleSendTimer()
{

    if((tcpSock.state() == QAbstractSocket::ConnectedState) && (pos != -1)){
        QByteArray ba;
        ba.append((char)0x55);
        ba.append((char)0x1e);
        ba.append((char)0x49);
        ba.append((char)0x71);
        ba.append((char)0x01);
        ba.append((char)0x00);
        ba.append((char)0x08);
        ba.append((char)0x38);

        //ba.append((char)0xeb);
        //ba.append((char)0x2c);

        ba.append((char)0x0b);
        ba.append((char)0x4c);

        //ba.append((char)((pos>>8)&0xff));
        //ba.append((char)(pos&0xff));
        ba.append((char)0xaa);
        ba.append((char)0xaa);

        //qDebug() <<ba.size();
        //for(int i=0; i<4; i++){
        if(sendCnt == 2){
            tcpSock.write(ba);
            tcpSock.flush();
        }
        else if(sendCnt == 1){
            QByteArray baN;
            baN.append(ba);
            baN.append(ba);
            baN.append(ba);

            tcpSock.write(baN);
            tcpSock.flush();
        }
        //}
        hbTimer.start();
        sendCnt--;
        if(sendCnt == 0){
            pos = -1;
        }
        qDebug() << ba.toHex();
    }
}

void Dialog::createMsg(QByteArray &ba, int d)
{
    ba.append((char)0x55);
    ba.append((char)0x1e);
    ba.append((char)0x49);
    ba.append((char)0x71);
    ba.append((char)0x01);
    ba.append((char)0x00);
    ba.append((char)0x08);
    ba.append((char)0x38);

    //ba.append((char)0xeb);
    //ba.append((char)0x2c);

    //ba.append((char)0x0b);
    //ba.append((char)0x4c);

    ba.append((char)((d>>8)&0xff));
    ba.append((char)(d&0xff));
    ba.append((char)0xaa);
    ba.append((char)0xaa);
}

void Dialog::on_pushButton_clicked()
{
    if((tcpSock.state() == QAbstractSocket::ConnectedState)){
        QByteArray ba;
        createMsg(ba, 0x849);
        tcpSock.write(ba);
        tcpSock.flush();
    }

}

void Dialog::on_pushButton_2_clicked()
{
    if((tcpSock.state() == QAbstractSocket::ConnectedState)){
        QByteArray ba;
        createMsg(ba, 0xa4b);
        tcpSock.write(ba);
        tcpSock.flush();
    }
}

void Dialog::on_pushButton_3_clicked()
{
    if((tcpSock.state() == QAbstractSocket::ConnectedState) ){
        QByteArray ba;
        createMsg(ba, 0x1354);
        tcpSock.write(ba);
        tcpSock.flush();
    }
}

void Dialog::on_pushButton_4_clicked()
{
    if((tcpSock.state() == QAbstractSocket::ConnectedState)){
        QByteArray ba;
        createMsg(ba, 0xbcfd);
        tcpSock.write(ba);
        tcpSock.flush();
    }

}

void Dialog::on_pushButton_5_clicked()
{
    if((tcpSock.state() == QAbstractSocket::ConnectedState)){
        QByteArray ba;
        createMsg(ba, 0xcc0d);
        tcpSock.write(ba);
        tcpSock.flush();
    }

}

void Dialog::on_pushButton_6_clicked()
{
    if((tcpSock.state() == QAbstractSocket::ConnectedState) ){
        QByteArray ba;
        createMsg(ba, 0xd314);
        tcpSock.write(ba);
        tcpSock.flush();
    }

}
