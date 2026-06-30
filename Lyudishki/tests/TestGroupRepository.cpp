#include <QtTest>
#include <QSqlDatabase>
#include "data/db/DatabaseManager.h"
#include "data/db/MigrationManager.h"
#include "data/repositories/SqliteGroupRepository.h"

class TestGroupRepository : public QObject {
    Q_OBJECT
private slots:
    void testSeedGroups()
    {
        DatabaseManager dbm(":memory:");
        QVERIFY(dbm.isOpen());
        MigrationManager mm(dbm.database());
        QVERIFY(mm.migrate());

        SqliteGroupRepository repo(dbm.database());
        auto groups = repo.getAll();
        QCOMPARE(groups.size(), 4);
        QCOMPARE(groups[0].name, "Родственники");
        QCOMPARE(groups[1].name, "Друзья");
        QCOMPARE(groups[2].name, "Работа");
        QCOMPARE(groups[3].name, "Случайные");
    }

    void testCrud()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        SqliteGroupRepository repo(dbm.database());
        Group g;
        g.name = "Тест";
        int id = repo.add(g);
        QVERIFY(id > 0);

        auto fetched = repo.getById(id);
        QCOMPARE(fetched.name, "Тест");

        fetched.name = "Обновлено";
        QVERIFY(repo.update(fetched));

        auto updated = repo.getById(id);
        QCOMPARE(updated.name, "Обновлено");

        QVERIFY(repo.remove(id));
        auto removed = repo.getById(id);
        QCOMPARE(removed.id, 0);
    }
};

QObject* createTestGroupRepository() { return new TestGroupRepository(); }

#include "TestGroupRepository.moc"
