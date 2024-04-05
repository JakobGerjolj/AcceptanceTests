#include <QtTest>
#include <QSignalSpy>
#include "/home/jakob/testBenchLibrary/keycardsactuator.h"

class Test : public QObject
{
    Q_OBJECT

public:
    Test();
    ~Test();

private slots:
    void test_case1();
    void initTestCase();
    void cleanupTestCase();

private:
    QSerialPort *m_port{nullptr};
    KeyCardsActuator *m_KeyCardsActuator{nullptr};
};

Test::Test(){}

void Test::initTestCase()
{
    m_port = new QSerialPort("/dev/ttyACM0");
    m_port->setBaudRate(QSerialPort::Baud115200);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setFlowControl(QSerialPort::NoFlowControl);
    if(!m_port->open(QIODevice::ReadWrite))
        qDebug() << m_port->errorString();

    m_KeyCardsActuator = new KeyCardsActuator(m_port);


}

void Test::cleanupTestCase()
{
    delete m_KeyCardsActuator;
    delete m_port;
}




Test::~Test() {
}

void Test::test_case1()
{
    QSignalSpy spy(m_KeyCardsActuator, &KeyCardsActuator::leftCardFinishedMoving);

    m_KeyCardsActuator->approchLeftCard();


    spy.wait(4000);

    // auto wasSpyCalled = spy.wait(7000);

    // QCOMPARE(wasSpyCalled, true);

}



QTEST_APPLESS_MAIN(Test)

#include "tst_test.moc"
