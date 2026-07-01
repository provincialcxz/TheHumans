#include <QtTest>
#include "ui/models/PersonListModel.h"
#include "ui/models/PersonSortFilterProxy.h"

class TestPersonSortFilterProxy : public QObject {
    Q_OBJECT
private slots:
    void testForgottenOnlyFilter()
    {
        PersonListModel model;
        PersonSortFilterProxy proxy;
        proxy.setSourceModel(&model);

        Person recent;
        recent.id = 1;
        recent.firstName = "Recent";
        recent.lastContactDate = QDate::currentDate().addDays(-10);

        Person stale;
        stale.id = 2;
        stale.firstName = "Stale";
        stale.lastContactDate = QDate::currentDate().addDays(-200);

        Person never;
        never.id = 3;
        never.firstName = "Never";
        // lastContactDate left invalid: never logged counts as forgotten too.

        model.setPeople({recent, stale, never});
        QCOMPARE(proxy.rowCount(), 3);

        proxy.setForgottenOnly(true);
        QCOMPARE(proxy.rowCount(), 2);

        proxy.setForgottenOnly(false);
        QCOMPARE(proxy.rowCount(), 3);
    }
};

QObject* createTestPersonSortFilterProxy() { return new TestPersonSortFilterProxy(); }

#include "TestPersonSortFilterProxy.moc"
