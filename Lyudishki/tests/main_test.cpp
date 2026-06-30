#include <QApplication>
#include <QtTest>

extern QObject* createTestGroupRepository();
extern QObject* createTestPersonRepository();
extern QObject* createTestServices();

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    int status = 0;

    auto runTest = [&](QObject *test) {
        status |= QTest::qExec(test, argc, argv);
        delete test;
    };

    runTest(createTestGroupRepository());
    runTest(createTestPersonRepository());
    runTest(createTestServices());

    return status;
}
