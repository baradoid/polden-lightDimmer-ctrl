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
    ui->verticalSlider->setSingleStep(55000/5);
    ui->verticalSlider->setPageStep(55000/5);

    connect(&tcpSock, SIGNAL(connected()), this, SLOT(handleSocketConnected()));
    connect(&tcpSock, SIGNAL(hostFound()), this, SLOT(handleSocketHostFound()));
    connect(&tcpSock, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(handleSocketStateChanged(QAbstractSocket::SocketState)));
    connect(&tcpSock, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(handleSocketError(QAbstractSocket::SocketError)));
    connect(&tcpSock,SIGNAL(bytesWritten(qint64)), this, SLOT(handleBytesWritten(qint64)));
    connect(&tcpSock, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));



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
    quint64 curTime = QTime::currentTime().msecsSinceStartOfDay();
    if((tcpSock.state() == QAbstractSocket::ConnectedState) && ((curTime - lastSendTime) > 1000)){
        lastSendTime = curTime;
        QByteArray ba;
        createMsg(ba, 0xd314);
        tcpSock.write(ba);
        tcpSock.flush();
    }

//    //qDebug() << position;
//    if(pos == -1){
//        pos = position;
//        sendCnt = 2;
//    }

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

void Dialog::handleReadyRead()
{
    qDebug()<<"handleReadyRead";

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
void Dialog::handleBytesWritten(qint64 bytes)
{
    //qDebug() << bytes;
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




void Dialog::on_pushButtonPonOnAll_clicked()
{
    if((tcpSock.state() == QAbstractSocket::ConnectedState)){
        QByteArray ba;
        ba.append((char)0x55);
        ba.append((char)0x31);
        ba.append((char)0x34);
        ba.append((char)0x38);
        ba.append((char)0x01);
        ba.append((char)0x40);
        ba.append((char)0x02);
        ba.append((char)0x12);
        ba.append((char)0xab);
        ba.append((char)0x00);
        ba.append((char)0xaa);
        ba.append((char)0xaa);
        tcpSock.write(ba);
        tcpSock.flush();
    }
}

void Dialog::on_pushButtonPonOffAll_clicked()
{
    if((tcpSock.state() == QAbstractSocket::ConnectedState)){
        QByteArray ba;
        ba.append((char)0x55);
        ba.append((char)0x31);
        ba.append((char)0x34);
        ba.append((char)0x38);
        ba.append((char)0x01);
        ba.append((char)0x40);
        ba.append((char)0x02);
        ba.append((char)0x12);
        ba.append((char)0xa9);
        ba.append((char)0xfe);
        ba.append((char)0xaa);
        ba.append((char)0xaa);
        tcpSock.write(ba);
        tcpSock.flush();
    }
}

void Dialog::on_pushButtonPonOff1_clicked()
{
    if((tcpSock.state() == QAbstractSocket::ConnectedState)){
        QByteArray ba;
        ba.append((char)0x55);
        ba.append((char)0x31);
        ba.append((char)0x34);
        ba.append((char)0x38);
        ba.append((char)0x01);
        ba.append((char)0x08);
        ba.append((char)0x02);
        ba.append((char)0x0a);

        //ba.append((char)0xeb);
        //ba.append((char)0x2c);

        //ba.append((char)0x0b);
        //ba.append((char)0x4c);

        ba.append((char)0x9b);
        ba.append((char)0xb0);
        ba.append((char)0xaa);
        ba.append((char)0xaa);
        tcpSock.write(ba);
        tcpSock.flush();
    }
}

void Dialog::on_pushButtonPonOn1_clicked()
{
    if((tcpSock.state() == QAbstractSocket::ConnectedState)){
        QByteArray ba;
        ba.append((char)0x55);
        ba.append((char)0x31);
        ba.append((char)0x34);
        ba.append((char)0x38);
        ba.append((char)0x01);
        ba.append((char)0x08);
        ba.append((char)0x02);
        ba.append((char)0x0a);

        //ba.append((char)0xeb);
        //ba.append((char)0x2c);

        //ba.append((char)0x0b);
        //ba.append((char)0x4c);

        ba.append((char)0x9c);
        ba.append((char)0xb1);
        ba.append((char)0xaa);
        ba.append((char)0xaa);
        tcpSock.write(ba);
        tcpSock.flush();
    }
}

void Dialog::on_pushButtonOn2_clicked()
{
    if((tcpSock.state() == QAbstractSocket::ConnectedState)){
        QByteArray ba;
        ba.append((char)0x55);
        ba.append((char)0x31);
        ba.append((char)0x34);
        ba.append((char)0x38);
        ba.append((char)0x01);
        ba.append((char)0x02);
        ba.append((char)0x02);
        ba.append((char)0x0a);
        ba.append((char)0x96);
        ba.append((char)0xa5);
        ba.append((char)0xaa);
        ba.append((char)0xaa);
        tcpSock.write(ba);
        tcpSock.flush();
    }
}

void Dialog::on_pushButtonOff2_clicked()
{
    if((tcpSock.state() == QAbstractSocket::ConnectedState)){
        QByteArray ba;
        ba.append((char)0x55);
        ba.append((char)0x31);
        ba.append((char)0x34);
        ba.append((char)0x38);
        ba.append((char)0x01);
        ba.append((char)0x02);
        ba.append((char)0x02);
        ba.append((char)0x0a);
        ba.append((char)0x95);
        ba.append((char)0xa4);
        ba.append((char)0xaa);
        ba.append((char)0xaa);
        tcpSock.write(ba);
        tcpSock.flush();
    }
}

void Dialog::on_pushButtonOn3_clicked()
{
    if((tcpSock.state() == QAbstractSocket::ConnectedState)){
        QByteArray ba;
        ba.append((char)0x55);
        ba.append((char)0x31);
        ba.append((char)0x34);
        ba.append((char)0x38);
        ba.append((char)0x01);
        ba.append((char)0x01);
        ba.append((char)0x02);
        ba.append((char)0x0a);
        ba.append((char)0x93);
        ba.append((char)0xa1);
        ba.append((char)0xaa);
        ba.append((char)0xaa);
        tcpSock.write(ba);
        tcpSock.flush();
    }
}

void Dialog::on_pushButtonOff3_clicked()
{
    if((tcpSock.state() == QAbstractSocket::ConnectedState)){
        QByteArray ba;
        ba.append((char)0x55);
        ba.append((char)0x31);
        ba.append((char)0x34);
        ba.append((char)0x38);
        ba.append((char)0x01);
        ba.append((char)0x01);
        ba.append((char)0x02);
        ba.append((char)0x0a);
        ba.append((char)0x92);
        ba.append((char)0xa0);
        ba.append((char)0xaa);
        ba.append((char)0xaa);
        tcpSock.write(ba);
        tcpSock.flush();
    }
}

void Dialog::on_pushButtonOn4_clicked()
{
    if((tcpSock.state() == QAbstractSocket::ConnectedState)){
        QByteArray ba;
        ba.append((char)0x55);
        ba.append((char)0x31);
        ba.append((char)0x34);
        ba.append((char)0x38);
        ba.append((char)0x01);
        ba.append((char)0x80);
        ba.append((char)0x02);
        ba.append((char)0x0a);
        ba.append((char)0xa8);
        ba.append((char)0x35);
        ba.append((char)0xaa);
        ba.append((char)0xaa);
        tcpSock.write(ba);
        tcpSock.flush();
    }
}

void Dialog::on_pushButtonOff4_clicked()
{
    if((tcpSock.state() == QAbstractSocket::ConnectedState)){
        QByteArray ba;
        ba.append((char)0x55);
        ba.append((char)0x31);
        ba.append((char)0x34);
        ba.append((char)0x38);
        ba.append((char)0x01);
        ba.append((char)0x80);
        ba.append((char)0x02);
        ba.append((char)0x0a);
        ba.append((char)0xa7);
        ba.append((char)0x34);
        ba.append((char)0xaa);
        ba.append((char)0xaa);
        tcpSock.write(ba);
        tcpSock.flush();
    }
}
