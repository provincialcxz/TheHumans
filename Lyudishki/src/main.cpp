#include <QApplication>
#include <QFontDatabase>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include "app/AppContext.h"
#include "ui/windows/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Lyudishki");
    app.setOrganizationName("Lyudishki");

    // Load font
    int fontId = QFontDatabase::addApplicationFont(":/fonts/JetBrainsMono-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/JetBrainsMono-Bold.ttf");
    if (fontId >= 0) {
        QStringList families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.isEmpty()) {
            QFont defaultFont(families.first(), 11);
            app.setFont(defaultFont);
        }
    }

    // App icon (for Linux/Windows window icon)
    app.setWindowIcon(QIcon(":/icons/app_icon.png"));

    // Apply QSS
    QFile qssFile(":/styles/mrrobot.qss");
    if (qssFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(qssFile.readAll());
        qssFile.close();
    }

    // DB path: ~/Documents/Lyudishki/
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                      + "/Lyudishki";
    QDir().mkpath(dataDir);
    QString dbPath = dataDir + "/lyudishki.db";

    AppContext ctx(dbPath, dataDir);
    if (!ctx.initialize()) {
        qCritical("Failed to initialize database");
        return 1;
    }

    MainWindow w(ctx);
    w.resize(1100, 700);
    w.show();

    return app.exec();
}
