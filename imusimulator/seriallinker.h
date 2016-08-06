#ifndef SERIALLINKER_H
#define SERIALLINKER_H

#include <QObject>
#include <QDebug>
#include <QtSerialPort/QSerialPort>

class SerialLinker : public QObject
{
    Q_OBJECT
public:
    explicit SerialLinker(QString sName, qint32 sSpeed, QObject *parent = 0);
    ~SerialLinker();

    enum LinkCommand
    {
        NoCommand,
        Connect,
        DataStart,
        DataStop
    };

    bool makeLinkToPort();
    LinkCommand receiveCommand();
    bool replyConnectCommand();
    bool sendIMUData( QByteArray &buffer );

    static const int commandLength = 3;



private:
    QSerialPort *serial;
    QString     serialName;
    qint32      serialSpeed;

    int initLink();

signals:
    void error( QString errStr, qint64 errCode );

public slots:

};

#endif // SERIALLINKER_H
