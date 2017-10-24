#ifndef UDP_MANAGER_H
#define UDP_MANAGER_H

#include "UDPDiscover/UDPDiscover.h"
#include "DataStruct/userlistsingleton.h"
#include <QObject>
#include <QThread>
#include <QTimer>
#include <queue>
#include <thread>
#include <string>

class UDP_Manager : public QObject
{
    Q_OBJECT
private:
    UDPDiscover* udp;

    std::queue<struct User> newQueue;
    std::queue<struct User> deletedQueue;

    std::string username;
    std::string picture;

    std::thread* newThread;
    std::thread* deletedThread;

    bool running;
    bool mode;

    void run(bool);

    QThread* timerThread;
    QTimer* timer;
public:
    explicit UDP_Manager(QObject *parent = 0 );
    ~UDP_Manager();
    void start(string, bool, std::string);
signals:
    void showSignal(QString, QString);
    void addUser(User);
    void deleteUser(User);
public slots:
    void checkUser();
};

#endif // UDP_MANAGER_H
