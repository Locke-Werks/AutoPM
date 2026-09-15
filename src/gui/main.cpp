#include "core/workspace.h"
#include "mainwindow.h"
#include "theme.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>
#include <QStandardPaths>

namespace {

// Definitions live next to the executable once installed, and a few levels up
// in a build tree. Take the first one that exists.
QString findDefinitions(const QString& executableDir) {
    for (const std::string& candidate :
         pm::definitionSearchPaths(executableDir.toStdString())) {
        const QString path = QDir::cleanPath(QString::fromStdString(candidate));
        if (QDir(path).exists()) return path;
    }
    return QString();
}

QString findAssets(const QString& executableDir) {
    QDir dir(executableDir);
    for (int up = 0; up < 4; ++up) {
        if (QDir(dir.filePath("assets")).exists()) return dir.filePath("assets");
        if (!dir.cdUp()) break;
    }
    return executableDir + "/assets";
}

// Record #1: this project's own charter, so the tool is never empty on first
// run and the first thing it shows is a real project managed in it.
void seedFirstRun(const QString& assetsDir, const QString& projectsDir) {
    QDir projects(projectsDir);
    if (!projects.entryList({"*.pmproj"}, QDir::Files).isEmpty()) return;

    QDir seeds(assetsDir + "/seed");
    if (!seeds.exists()) return;
    for (const QString& file : seeds.entryList({"*.pmproj"}, QDir::Files))
        QFile::copy(seeds.filePath(file), projects.filePath(file));
}

} // namespace

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setApplicationName("AutoPM");
    app.setOrganizationName("Locke Werks");
    app.setApplicationVersion("0.1.0");

    const QString executableDir = QCoreApplication::applicationDirPath();
    const QString assetsDir = findAssets(executableDir);
    const QString definitionsDir = findDefinitions(executableDir);

    theme::loadFonts(assetsDir);

    // --fonts <file> reports which faces resolved and exits. A machine without
    // the house faces falls back silently, and this is how you find out.
    const QStringList early = QCoreApplication::arguments();
    const int fontsAt = early.indexOf("--fonts");
    if (fontsAt >= 0 && fontsAt + 1 < early.size()) {
        QFile report(early[fontsAt + 1]);
        if (report.open(QIODevice::WriteOnly | QIODevice::Text))
            report.write(theme::resolvedFaces().toUtf8());
        return 0;
    }
    theme::setAccent(QColor("#b76bff"));
    app.setFont(theme::bodyFont(14));
    app.setStyleSheet(theme::styleSheet());
    app.setWindowIcon(QIcon(assetsDir + "/autopm.ico"));

    if (definitionsDir.isEmpty()) {
        QMessageBox::critical(nullptr, "AutoPM",
                              "AutoPM could not find its definitions folder.\n\n"
                              "It should sit next to AutoPM.exe. Every screen in the app is "
                              "built from those files, so there is nothing to show without "
                              "them.");
        return 1;
    }

    const QString projectsDir =
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/AutoPM";
    QDir().mkpath(projectsDir);
    seedFirstRun(assetsDir, projectsDir);

    MainWindow window(definitionsDir, projectsDir);
    theme::applyDarkFrame(&window);
    window.show();

    // --screen <id> opens straight onto one screen. Handy when you live on one.
    const QStringList arguments = app.arguments();
    const int at = arguments.indexOf("--screen");
    if (at >= 0 && at + 1 < arguments.size()) window.showScreenById(arguments[at + 1]);

    // --walk [screen] opens the walkthrough instead of the normal panes.
    const int walk = arguments.indexOf("--walk");
    if (walk >= 0) {
        const QString screen = walk + 1 < arguments.size() && !arguments[walk + 1].startsWith("--")
                                   ? arguments[walk + 1]
                                   : QString();
        window.startWalkthrough(screen);
    }
    return app.exec();
}
