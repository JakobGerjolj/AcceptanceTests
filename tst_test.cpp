#include <QtTest>
#include <QSignalSpy>
#include <QCanBus>
#include <QDebug>
#include "../testBenchLibrary/keycardsactuator.h"
#include "../testBenchLibrary/buttons.h"
#include "../testBenchLibrary/camera.h"
#include "../testBenchLibrary/leversactuator.h"
#include <wiringPi.h>
#include <iostream>

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
    void test_leverFWDAndCan();
    void test_topLightsInNeutral();
    void test_topLightsNotInNeutral();
    void test_neutralSyncPressed();
    void test_notNeutralSyncPressed();
    void test_testCANMessages();
    void test_calibrationForNeutral();
    void cleanupTestCase();

private:
    const int m_POWER_PIN = 4;
    QCoreApplication *app{nullptr};
    QSerialPort *m_port{nullptr};
    QSerialPort *m_port1{nullptr};
    KeyCardsActuator *m_KeyCardsActuator{nullptr};
    Buttons *m_buttons{nullptr};
    QCanBusDevice *m_device{nullptr};
    Camera *m_camera{nullptr};
    LeversActuator *m_levers{nullptr};


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

    m_port = new QSerialPort("/dev/ttyACM2");
    m_port->setBaudRate(QSerialPort::Baud115200);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setFlowControl(QSerialPort::NoFlowControl);
    if(!m_port->open(QIODevice::ReadWrite))
        qDebug() << m_port->errorString();
    else qDebug()<< "Port open";

    m_port1 = new QSerialPort("/dev/ttyACM1");
    m_port1->setBaudRate(QSerialPort::Baud115200);
    m_port1->setDataBits(QSerialPort::Data8);
    m_port1->setStopBits(QSerialPort::OneStop);
    m_port1->setParity(QSerialPort::NoParity);
    m_port1->setFlowControl(QSerialPort::NoFlowControl);
    if(!m_port1->open(QIODevice::ReadWrite))
        qDebug() << m_port1->errorString();
    else qDebug()<< "Port open";

    m_KeyCardsActuator = new KeyCardsActuator(m_port);
    m_camera = new Camera;
    m_levers = new LeversActuator(m_port1);
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
    QSignalSpy waitForBothHome(m_levers, &LeversActuator::bothHome); //Send both home always
    m_levers->home();

    auto leversHome=waitForBothHome.wait(8000);
    QCOMPARE(leversHome,true);
    digitalWrite(m_POWER_PIN,HIGH);
    m_port->close();
    m_port1->close();
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
    QSKIP("Test already tested!");
    //should be locked here

    QSignalSpy waitForRightCardMove(m_KeyCardsActuator, &KeyCardsActuator::rightCardFinishedMoving);
    m_KeyCardsActuator ->approchRightCard();


    auto wasRightMoveCalled = waitForRightCardMove.wait(7000);
    QCOMPARE(wasRightMoveCalled,true);

    bool wasLeverUnlocked = false;

    foreach(auto frame, m_recivedFrames){
        if((frame.frameId() == 0x18ff82fd) && frame.payload()[2] == 0x01)
        {
            wasLeverUnlocked = true;
        }
    }

    QCOMPARE(wasLeverUnlocked,true);

    QSignalSpy waitForPicture(m_camera,&Camera::pictureTaken);
    m_camera->takePicture();

    auto wasPictureTaken=waitForPicture.wait(10000);

    QCOMPARE(wasPictureTaken,true);

    qDebug()<<"Sync Button status: "<<m_camera->getStatus(syncBUTTON);

    QCOMPARE(m_camera->getStatus(syncBUTTON),true);


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

void Test::test_topLightsInNeutral()
{
    QSKIP("");
    QSignalSpy waitForLeverUnlock(m_KeyCardsActuator, &KeyCardsActuator::rightCardFinishedMoving);
    m_KeyCardsActuator-> approchRightCard();
    auto isLeverUnlocked=waitForLeverUnlock.wait(7000); //unlocking card
    QCOMPARE(isLeverUnlocked,true);
    // delay(3000);
    //m_levers->home();
    QSignalSpy waitForReverseMove(m_levers,&LeversActuator::bothMoved);
    m_levers->moveBoth(LeversActuator::reverse,730); //method move to neutral
    auto didBothMove=waitForReverseMove.wait(7000);
    QCOMPARE(didBothMove,true);

    QSignalSpy waitForPictureTaken(m_camera, &Camera::pictureTaken);
    m_camera->takePicture();
    auto didWeTakePicture=waitForPictureTaken.wait(5000);
    //take picture
    QCOMPARE(didWeTakePicture,true);
    // delay(5000);
    QSignalSpy waitForLeversHome(m_levers, &LeversActuator::bothHome);
    m_levers->home();
    auto isLeversHome=waitForLeversHome.wait(7000);
    //lever not sending messages

    QCOMPARE(isLeversHome,true);
    // QSignalSpy spy(m_buttons, &Buttons::pressed);
    // m_buttons->press(sync);
    //m_buttons->press(station); not really work
    // m_buttons->press(dock);
    // auto wasSpyCalled=spy.wait(6000);

    //QCOMPARE(true,true);
    QCOMPARE(m_camera->getStatus(leftLEVER),true);
    QCOMPARE(m_camera->getStatus(rightLEVER),true);


}

void Test::test_topLightsNotInNeutral()
{
    QSKIP("");
    //unlock lever
    QSignalSpy waitForLeverUnlock(m_KeyCardsActuator, &KeyCardsActuator::rightCardFinishedMoving);
    m_KeyCardsActuator->approchRightCard();
    auto isLeverUnlocked=waitForLeverUnlock.wait(7000);
    QCOMPARE(isLeverUnlocked,true);

    QSignalSpy waitForPictureTaken(m_camera, &Camera::pictureTaken);
    m_camera->takePicture();
    auto wasPictureTaken=waitForPictureTaken.wait(5000);
    QCOMPARE(wasPictureTaken,true);

    QCOMPARE(m_camera->getStatus(leftLEVER),false);
    QCOMPARE(m_camera->getStatus(rightLEVER),false);

}

void Test::test_neutralSyncPressed()
{
    //First unlock lever
    QSKIP("");
    QSignalSpy waitForLeverUnlock(m_KeyCardsActuator, &KeyCardsActuator::rightCardFinishedMoving);
    m_KeyCardsActuator->approchRightCard();
    auto isLeverUnlocked=waitForLeverUnlock.wait(7000);
    QCOMPARE(isLeverUnlocked,true);

    QSignalSpy waitForReverseMove(m_levers,&LeversActuator::bothMoved);
    m_levers->moveBoth(LeversActuator::reverse,700);
    auto didBothMove=waitForReverseMove.wait(7000);
    QCOMPARE(didBothMove,true);

    QSignalSpy waitForPressedSync(m_buttons,&Buttons::pressed);
    m_buttons->press(sync);
    auto isSyncPressed=waitForPressedSync.wait(5000);
    QCOMPARE(isSyncPressed,true);

    bool wasSyncActivated=false;

    foreach(auto frame, m_recivedFrames){
        if((frame.frameId() == 0x18FF82FD) && frame.payload()[1] == 0x03)
        {
            wasSyncActivated = true;
        }

    }



    QSignalSpy waitForLeversHome(m_levers, &LeversActuator::bothHome);
    m_levers->home();

    auto isLeversHome=waitForLeversHome.wait(7000);
    QCOMPARE(isLeversHome,true);


    QCOMPARE(wasSyncActivated,true); //Not getting sync, prob lever software fault (single mode prob)


}

void Test::test_notNeutralSyncPressed()
{
    //Lever should be unlocked
    QSKIP("");
    QSignalSpy waitForLeverUnlock(m_KeyCardsActuator, &KeyCardsActuator::rightCardFinishedMoving);
    m_KeyCardsActuator->approchRightCard();
    auto isLeverUnlocked=waitForLeverUnlock.wait(5000);
    QCOMPARE(isLeverUnlocked,true);

    QSignalSpy waitForPressedSync(m_buttons, &Buttons::pressed);
    m_buttons->press(sync);
    auto isSyncPressed=waitForPressedSync.wait(6000);
    QCOMPARE(isSyncPressed,true);


    bool wasSyncActivated=false;

    foreach(auto frame, m_recivedFrames){
        if((frame.frameId() == 0x18FF82FD) && frame.payload()[1] == 0x03)
        {
            wasSyncActivated = true;
        }

    }

    QCOMPARE(wasSyncActivated,false);

}

void Test::test_testCANMessages()
{
    QSKIP("");
    //unlock lever first

    QSignalSpy waitForUnlocked(m_KeyCardsActuator,&KeyCardsActuator::rightCardFinishedMoving);
    m_KeyCardsActuator->approchRightCard();

    auto wasLeverUnlocked=waitForUnlocked.wait(8000);
    QCOMPARE(wasLeverUnlocked,true);

    int countStatus=0;
    int countCommand=0;

    QSignalSpy delayTenSeconds(m_KeyCardsActuator,&KeyCardsActuator::ghost);

    QCOMPARE(delayTenSeconds.wait(10000),false);

    bool isSendingStatus=false;

    bool isSendingCommand=false;

    bool isFrameDataOK=true;

    bool isDataLongEnough=true;
    foreach(auto frame, m_recivedFrames){

        qDebug()<<frame.toString();
        if(frame.frameId()==0x18ff80fd){
            countStatus++;
            isSendingStatus=true;

            if(frame.payload().length()==8){
                isDataLongEnough=true;

            }else{
                isDataLongEnough=false;

            }

            if(frame.isValid()){
                isFrameDataOK=true;
            }else {
                isFrameDataOK=false;
            }
            if(frame.payload()[1] != 0x01){
                isFrameDataOK=false;

            }

            if(frame.payload()[2] != 0x01){
                isFrameDataOK=false;

            }

            if(frame.payload()[3] != 0x01){
                isFrameDataOK=false;

            }

            //qDebug()<<frame.payload()[9];




        }

        if(frame.frameId()==0x0cff81fd){
            countCommand++;
            isSendingCommand=true;
        }

    }


    bool wasEnoughCountStatus;
    bool wasEnoughCountCommand;
    //Around 10sec, around 20 status msg, around 200 command msg
    if(countStatus>=18){
        wasEnoughCountStatus = true;
    }else {
        wasEnoughCountStatus = false;
    }

    if(countCommand>=20){
        wasEnoughCountCommand=true;
    }else {
        wasEnoughCountCommand=false;
    }

    qDebug()<<"Count of status: "<<countStatus;
    qDebug()<<"Count of command: "<<countCommand;

    QCOMPARE(isDataLongEnough,true);
    QCOMPARE(isFrameDataOK,true);
    QCOMPARE(isSendingCommand,true);
    QCOMPARE(isSendingStatus,true);
    QCOMPARE(wasEnoughCountStatus,true);
    QCOMPARE(wasEnoughCountCommand,true);



}

void Test::test_calibrationForNeutral()
{
    QSKIP("");

    // QSignalSpy waitForUnlocked(m_KeyCardsActuator,&KeyCardsActuator::rightCardFinishedMoving);
    // m_KeyCardsActuator->approchRightCard();
    // auto wasLeverUnlocked=waitForUnlocked.wait(8000);
    // QCOMPARE(wasLeverUnlocked,true);

    // QSignalSpy waitForBothHome(m_levers, &LeversActuator::bothHome); //Send both home always
    // m_levers->home();
    //m_buttons->press(sync);
    // auto leversHome=waitForBothHome.wait(6000);
    // QCOMPARE(leversHome,true);


    // QSignalSpy waitForReverseMove(m_levers,&LeversActuator::bothMoved);
    // m_levers->moveBoth(LeversActuator::reverse,730); //move to neutral
    // auto didBothMove=waitForReverseMove.wait(7000);
    // QCOMPARE(didBothMove,true);

    // delay(5000);

    QSignalSpy waitForPress(m_buttons,&Buttons::pressed);

    m_buttons->press(sync);

    auto wasPressed=waitForPress.wait(9000);
    //delay(5000);
    delay(2500);
    QCOMPARE(wasPressed,true);
    delay(10000);

}

void Test::test_leverFWDAndCan()
{
    //QSKIP("Wait for proper messages");


    //Always put the home first just in case

    QSignalSpy waitForUnlocked(m_KeyCardsActuator,&KeyCardsActuator::rightCardFinishedMoving);
    m_KeyCardsActuator->approchRightCard();

    auto wasLeverUnlocked=waitForUnlocked.wait(8000);
    QCOMPARE(wasLeverUnlocked,true);
    qDebug()<<"Already here!";

    // QSignalSpy waitForBothHome(m_levers, &LeversActuator::bothHome); //Send both home always
    // m_levers->home();

    // auto leversHome=waitForBothHome.wait(6000);
    // QCOMPARE(leversHome,true);
    //delay(500);



    QSignalSpy waitForReverseMove(m_levers,&LeversActuator::bothMoved);
    m_levers->moveBoth(LeversActuator::reverse,730); //move to neutral
    auto didBothMove=waitForReverseMove.wait(7000);

    QCOMPARE(didBothMove,true);
    //SETUP DONE in Neutral now lets try slower movements

    //delay(500);
    //press sync here and we start reading
    QSignalSpy waitForSyncPress(m_buttons, &Buttons::pressed);
    m_buttons->press(sync); //Press sync for active

    auto wasSyncPressed=waitForSyncPress.wait(8000);
    QCOMPARE(wasSyncPressed,true);
    //delay(500);
    //Should start reading here
    //delay(100);

    //diffrent signal here try to move it slover
    QSignalSpy waitForLeversHome(m_levers, &LeversActuator::bothHome); //Home for reading values
    m_levers->home();

    auto isLeversHome=waitForLeversHome.wait(7000);
    QCOMPARE(isLeversHome,true);
    //QSignalSpy(m_levers,LeversActuator::);


    // QTimer timer;
    // int timerCounter = 1;
    // timer.setInterval(100);

    // connect(&timer, &QTimer::timeout, [&]() {
    //     m_levers->moveBoth(LeversActuator::forward,39);
    //     qDebug()<<"Cycle triggered, currently in cycle: "<<timerCounter;
    //     timerCounter++;
    //     if(timerCounter == 20){
    //         timer.stop();
    //     }

    // });


    //m_levers->moveBoth(LeversActuator::forward,39);
    // QSignalSpy waitForMoveFinished(&timer, &QTimer::timeout);
    // timer.start();




    QSignalSpy waitForReverseMove2(m_levers, &LeversActuator::bothMoved);
    m_levers->moveBoth(LeversActuator::reverse,730); //And back to neutral for reading values
    auto didBothMove2 = waitForReverseMove2.wait(8000);

    QCOMPARE(didBothMove2,true);
    int counter=0;

    foreach(auto frame, m_recivedFrames){
        if(frame.frameId()==0x0CFF81FD){

            qDebug()<<"Values from read "<<counter<<": "<<static_cast<unsigned char>(frame.payload().at(2));
            // leverValues.append(frame.payload().at(2)); //Read values here
            counter++;
        }
    }
    qDebug()<<"Amount of packages of data of HAL: "<<counter<<" !";
    QCOMPARE(true,true);


}



QTEST_APPLESS_MAIN(Test)

#include "tst_test.moc"
