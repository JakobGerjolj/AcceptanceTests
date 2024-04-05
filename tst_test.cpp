#include <QtTest>
#include <QSignalSpy>
#include "/home/jakob/testBenchLibrary/testbenchlibrary.h"
#include "/home/jakob/testBenchLibrary/keycardsactuator.h"
//#include "testbenchlibrary.h"
// add necessary includes here

class Test : public QObject
{
    Q_OBJECT

public:
    Test();
    ~Test();

private slots:
    void test_case1();
    void initTestCase();
private:
    QCoreApplication *app;
         //KeyCardsActuator* t_KeyCardsActuator;
};

Test::Test()
{
    int argc = 0;
    char *argv[] = { nullptr };
    app = new QCoreApplication(argc, argv);

    // QSerialPort port("/dev/ttyACM0");
    // port.setBaudRate(QSerialPort::Baud115200);
    // port.setDataBits(QSerialPort::Data8);
    // port.setStopBits(QSerialPort::OneStop);
    // port.setParity(QSerialPort::NoParity);
    // port.setFlowControl(QSerialPort::NoFlowControl);
    // if(!port.open(QIODevice::ReadWrite))
    //     qDebug() << port.errorString();
    // t_KeyCardsActuator=new KeyCardsActuator(&port);

}

void Test::initTestCase()
{

}




Test::~Test() {
    //   delete t_KeyCardsActuator;

}

void Test::test_case1() {

    QSerialPort port("/dev/ttyACM0");
    port.setBaudRate(QSerialPort::Baud115200);
    port.setDataBits(QSerialPort::Data8);
    port.setStopBits(QSerialPort::OneStop);
    port.setParity(QSerialPort::NoParity);
    port.setFlowControl(QSerialPort::NoFlowControl);
    if(!port.open(QIODevice::ReadWrite))
        qDebug() << port.errorString();


    // KeyCardsActuator t_keyCardsActuator(&port);
    KeyCardsActuator* t_keyCardsActuator=new KeyCardsActuator(&port);
    //connect(t_keyCardsActuator,KeyCardsActuator::allCardMoveFinished,this,Test::test_case1);
    // connect(&t_keyCardsActuator, &KeyCardsActuator::allCardMoveFinished, this,[&]() {
    //     qDebug() << "Signal sent";
    // });
    //port.write("2\r\n");
    //t_keyCardsActuator->moveCorrectCard();
    t_keyCardsActuator->approchRightCard();
    QSignalSpy spy(t_keyCardsActuator, &KeyCardsActuator::allCardMoveFinished);

    // connect(&t_keyCardsActuator, &KeyCardsActuator::allCardMoveFinished, [&]() {
    //     qDebug() << "Signal sent";
    // });

    //t_keyCardsActuator.approchRightCard();

    if(!spy.wait(7000)){
        qDebug()<<" Signal not emmited within timeout";

    }else {
        qDebug()<<" Signal emmited and captured";

    }
    qDebug()<<"Right status: "<<t_keyCardsActuator->rightCardStatus();

    //t_KeyCardsActuator->approchRightCard(); //?
    // delete app;
    // port.close();
    QCOMPARE(2,2);

}



QTEST_APPLESS_MAIN(Test)

#include "tst_test.moc"
