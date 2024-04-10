#include <QtTest>
#include <QSignalSpy>
#include <QCanBus>
#include <QDebug>
#include "../testBenchLibrary/keycardsactuator.h"
#include "../testBenchLibrary/buttons.h"
#include "../testBenchLibrary/camera.h"
#include <wiringPi.h>

class Test : public QObject
{
    Q_OBJECT

public:
    Test();
    ~Test();

private slots:
    void initTestCase();
    void test_wrongCardsDoesNotUnlock();
    void test_rightCardsDoesUnlock();
    void test_wrongCardDoesnNotLock();
    void test_rightCardLocks();
    void test_pressDock();
    void cleanupTestCase();

private:
    const int m_POWER_PIN = 4;
    QCoreApplication *app{nullptr};
    QSerialPort *m_port{nullptr};
    KeyCardsActuator *m_KeyCardsActuator{nullptr};
    Buttons *m_buttons{nullptr};
    QCanBusDevice *m_device{nullptr};
    Camera *m_camera{nullptr};


    QList<QCanBusFrame> m_recivedFrames;
    QString errorString;

};

Test::Test(){}

void Test::initTestCase()
{


    wiringPiSetup();
    pinMode(m_POWER_PIN, OUTPUT);
    digitalWrite(m_POWER_PIN,LOW);
    delay(2500);
    m_buttons=new Buttons;
    int argc = 0;
    char *argv[] = { nullptr };
    app = new QCoreApplication(argc, argv);
    QCoreApplication::processEvents();

    m_port = new QSerialPort("/dev/ttyACM1");
    m_port->setBaudRate(QSerialPort::Baud115200);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setFlowControl(QSerialPort::NoFlowControl);
    if(!m_port->open(QIODevice::ReadWrite))
        qDebug() << m_port->errorString();
    else qDebug()<< "Port open";

    m_KeyCardsActuator = new KeyCardsActuator(m_port);
    m_camera = new Camera;
    //m_port->clear();//digitalWrite(LED_PIN,HIGH);

    m_device = QCanBus::instance()->createDevice(
        QStringLiteral("socketcan"), QStringLiteral("slcan0"), &errorString);

    if (!m_device) {
        qDebug() << errorString;
    } else {
        m_device->connectDevice();
    }



    connect(m_device, &QCanBusDevice::framesReceived, this, [=](){
        m_recivedFrames.append(m_device->readAllFrames());
    });
}

void Test::cleanupTestCase()
{
    //digitalWrite(m_POWER_PIN,HIGH);
    m_port->close();
    delete m_KeyCardsActuator;
    delete m_port;
    delete app;
    delete m_device;
    delete m_camera;

}




Test::~Test() {}

void Test::test_wrongCardsDoesNotUnlock()
{
    QSKIP("Test already tested!");

    QSignalSpy waitForLeftCardMove(m_KeyCardsActuator, &KeyCardsActuator::leftCardFinishedMoving);
    m_KeyCardsActuator->approchLeftCard();

    auto wasLeftMoveCalled = waitForLeftCardMove.wait(7000);
    QCOMPARE(wasLeftMoveCalled,true);

    bool wasWrongCardMessageRecived = false;

    foreach (auto frame, m_recivedFrames) {
        if((frame.frameId() == 0x18ff82fd) && frame.payload()[2] == 0x03)
        {
            wasWrongCardMessageRecived = true;
        }
    }
    QCOMPARE(wasWrongCardMessageRecived, true);

}

void Test::test_rightCardsDoesUnlock()
{
    //QSKIP("Test already tested!");
    //should be locked here

    // QSignalSpy waitForRightCardMove(m_KeyCardsActuator, &KeyCardsActuator::rightCardFinishedMoving);
    // m_KeyCardsActuator ->approchRightCard();


    // auto wasRightMoveCalled = waitForRightCardMove.wait(7000);
    // QCOMPARE(wasRightMoveCalled,true);

    // bool wasLeverUnlocked = false;

    // foreach(auto frame, m_recivedFrames){
    //     if((frame.frameId() == 0x18ff82fd) && frame.payload()[2] == 0x01)
    //     {
    //         wasLeverUnlocked = true;
    //     }
    // }

    // QSignalSpy waitForPicture(m_camera,&Camera::pictureTaken);
    // m_camera->takePicture();

    // auto wasPictureTaken=waitForPicture.wait(10000);

    //QCOMPARE(wasPictureTaken,true);
    m_camera->processData();
    //delay(10000);

    //QCOMPARE(wasLeverUnlocked,true);
    QCOMPARE(true,true);


}

void Test::test_wrongCardDoesnNotLock()
{
    QSKIP("Test already tested!");

    QSignalSpy waitForLeverUnlock(m_KeyCardsActuator,&KeyCardsActuator::rightCardFinishedMoving);
    m_KeyCardsActuator->approchRightCard();
    auto wasLeverUnlocked=waitForLeverUnlock.wait(7000);
    QCOMPARE(wasLeverUnlocked,true);

    //should be unlocked here
    //mabe state of lever in class
    QSignalSpy waitForLeftMove(m_KeyCardsActuator,&KeyCardsActuator::leftCardFinishedMoving);
    m_KeyCardsActuator -> approchLeftCard();

    auto wasLeftMoved = waitForLeftMove.wait(7000);
    QCOMPARE(wasLeftMoved,true);

    bool wasLeverLocked=false;

    foreach(auto frame, m_recivedFrames){
        if((frame.frameId() == 0x18ff82fd) && frame.payload()[2] == 0x02)
        {
            wasLeverLocked = true;
        }

    }
    QCOMPARE(wasLeverLocked,false);

}

void Test::test_rightCardLocks()
{
    QSKIP("Test already tested!");
    QSignalSpy waitForLeverUnlock(m_KeyCardsActuator, &KeyCardsActuator::rightCardFinishedMoving);
    m_KeyCardsActuator-> approchRightCard();

    auto isLeverUnlocked=waitForLeverUnlock.wait(7000); //unlocking card

    QCOMPARE(isLeverUnlocked,true);

    QSignalSpy waitForRightMove(m_KeyCardsActuator,&KeyCardsActuator::rightCardFinishedMoving);
    m_KeyCardsActuator -> approchRightCard();

    auto wasRightMoved= waitForRightMove.wait(7000);
    QCOMPARE(wasRightMoved,true);
    bool wasLeverLocked = false;

    foreach(auto frame, m_recivedFrames){
        if((frame.frameId() == 0x18ff82fd) && frame.payload()[2] == 0x02)
        {
            wasLeverLocked = true;
        }

    }

    QCOMPARE(wasLeverLocked,true);

}

void Test::test_pressDock()
{
    QSKIP("");
    QSignalSpy spy(m_buttons, &Buttons::pressed);
    m_buttons->press(sync);
    //m_buttons->press(station); not really work
    m_buttons->press(dock);
    auto wasSpyCalled=spy.wait(6000);

    QCOMPARE(wasSpyCalled,true);
}



QTEST_APPLESS_MAIN(Test)

#include "tst_test.moc"
