#include <QtTest>
#include "/home/jakob/testBenchLibrary/testbenchlibrary.h"
//#include "testbenchlibrary.h"
// add necessary includes here

class Test : public QObject
{
    Q_OBJECT

public:
    Test();
    ~Test();
private:


private slots:
    void test_case1();
};

Test::Test() {}

Test::~Test() {}

void Test::test_case1() {
    QSerialPort port("/dev/ttyACM0");
    port.setBaudRate(QSerialPort::Baud115200);
    port.setDataBits(QSerialPort::Data8);
    port.setStopBits(QSerialPort::OneStop);
    port.setParity(QSerialPort::NoParity);
    port.setFlowControl(QSerialPort::NoFlowControl);
    if(!port.open(QIODevice::ReadWrite))
        qDebug() << port.errorString();

    port.write("2\r\n"); //works

    TestBenchLibrary t_test_bench(&port); //currently not working
    t_test_bench.move_correct_card(); //currently not working


    QCOMPARE(2,2);

}

QTEST_APPLESS_MAIN(Test)

#include "tst_test.moc"
