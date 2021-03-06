#include "notification.h"
#include "ui_notification.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QDesktopWidget>

#define DIVISION_VALUE  12

Notification::Notification(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Notification)
{
    ui->setupUi(this);

    this->setModal(false);
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);

    m_timerThread = new QThread(this);
    m_timer = new QTimer(0);
    m_timer->setInterval(5000);
    m_timer->moveToThread(m_timerThread);
    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(hideNotification()));
    QObject::connect(m_timerThread, SIGNAL(finished()), this, SLOT(deleteLater()));
    QObject::connect(m_timerThread, SIGNAL(started()), m_timer, SLOT(start()));
}

Notification::~Notification()
{
    delete ui;
    m_timerThread->quit();
    m_timerThread->wait();
    delete m_timerThread;
    delete m_timer;
}

void Notification::showNotification(QString t_text, QString t_picture) {
    if(!isVisible()){
        ui->userLabel->setText(t_text);
        createIcon(t_picture);

        setPosition();

        this->show();

        m_timerThread->start();
    }
}

void Notification::hideNotification(){
    if(isVisible()){
        this->hide();

        emit terminated();
    }
}

void Notification::setPosition(){
    this->adjustSize();
    QRect screenSize = QApplication::desktop()->availableGeometry();
    int desktopWidth = screenSize.width(); int desktopHeigh = screenSize.height();
    this->move( (desktopWidth - this->width()), (desktopHeigh - this->height()));

}

void Notification::createIcon(const QString& t_picture){
    QGraphicsScene* scene = new QGraphicsScene();
    ui->iconView->setScene(scene);
    QImage* image = new QImage(t_picture);
    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(*image));
    scene->addItem(item);
}
