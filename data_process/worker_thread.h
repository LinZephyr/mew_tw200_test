#ifndef MEW_WORKER_THREAD_H
#define MEW_WORKER_THREAD_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QFile>
#include <QJsonArray>

class SerialPortWorker : public QObject
{
    Q_OBJECT

public:
    SerialPortWorker(QObject *parent = nullptr);
    ~SerialPortWorker();
public slots:
    void doWork(const QByteArray hexdata);

signals:
    void resultReady(QJsonArray);
private:
    QFile json_file;
    bool jsfile_opened;
};

#endif // MEW_WORKER_THREAD_H
