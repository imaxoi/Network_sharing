#include "receivermanager.h"
#include "define.h"
#include <QApplication>
#include <QMessageBox>

using namespace std;

ReceiverManager::ReceiverManager():automaticMode(false) {
    socketThread = new QThread(this);
    timerThread = new QThread(this);
    timer = new QTimer();
    socketLoop = new SocketThread();
    timer->setInterval(1000);
    QObject::connect(this, SIGNAL(error(QString)), this, SLOT(showError(QString)));
    QObject::connect(socketLoop, SIGNAL(createUI()), this, SLOT(createUI()), Qt::BlockingQueuedConnection);
    QObject::connect(socketThread, SIGNAL(started()), socketLoop, SLOT(loop()));
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(checkProgress()));
    QObject::connect(timerThread, SIGNAL(started()), timer, SLOT(start()));
    timer->moveToThread(timerThread);
    socketLoop->moveToThread(socketThread);
}

ReceiverManager::~ReceiverManager(){
    socketLoop->disable();
    socketThread->quit();
    QThread::sleep(2);
    socketThread->wait();
    delete socketLoop;
    timerThread->quit();
    timerThread->wait();
    delete timerThread;
    delete socketThread;

}
void ReceiverManager::start(){
    socketThread->start();
    timerThread->start();
}

void ReceiverManager::createUI(){
    ReceivingObject newReceiver(path, move(socketLoop->getSocket()));
    int ret = QMessageBox::Ok;
    if(!automaticMode){
        QMessageBox msgBox;
        string message("Ti sta arrivando un file o cartella\n");
        message.append(newReceiver.receiver->getFileName());
        msgBox.setText(message.c_str());
        msgBox.setInformativeText("Vuoi accettarlo?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::No);
        ret = msgBox.exec();
    }

    newReceiver.receivingThread = unique_ptr<std::thread>(new std::thread(&FileReceiver::receive, newReceiver.receiver));

    if (ret == QMessageBox::Ok){
        newReceiver.progressUI->show();
        receivingList.push_back(move(newReceiver));
    }
    else{
        newReceiver.receiver->stop();
        if (newReceiver.receivingThread->joinable())
            newReceiver.receivingThread->join();
    }
}

void ReceiverManager::checkProgress(){
    for (auto it = receivingList.begin(); it != receivingList.end();){
        uint8_t status = it->receiver->getStatus();
        if (status == FT_ERROR){
            it->progressUI->close();
            emit error("Il trasferimento è fallito");
        }

        if (!it->progressUI->isClosed()){
            it->progressUI->setProgress(status);
            it++;
        }
         else {
            if ( (status != FT_COMPLETE) && (status != FT_ERROR) )
                it->receiver->stop();
            if (it->receivingThread->joinable())
                it->receivingThread->join();
            it = receivingList.erase(it);
        }
    }
}

void ReceiverManager::showError(QString errorText){
    QMessageBox errorBox;
    errorBox.setText(errorText);
    errorBox.exec();
}

