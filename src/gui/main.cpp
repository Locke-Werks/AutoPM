#include "core/workspace.h"
#include "mainwindow.h"
#include "spellcheck.h"
#include "theme.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QImageReader>
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

// A fictional demo project, filled in partway through Executing, so the tool is
// never empty on first run and every screen has a worked answer to read.
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
    app.setApplicationVersion("0.3.0");

    const QString executableDir = QCoreApplication::applicationDirPath();
    const QString assetsDir = findAssets(executableDir);
    const QString definitionsDir = findDefinitions(executableDir);

    spell::initialise();
    theme::loadFonts(assetsDir);

    // --fonts <file> reports which faces resolved and exits. A machine without
    // the house faces falls back silently, and this is how you find out.
    const QStringList early = QCoreApplication::arguments();
    const int fontsAt = early.indexOf("--fonts");
    if (fontsAt >= 0 && fontsAt + 1 < early.size()) {
        QFile report(early[fontsAt + 1]);
        if (report.open(QIODevice::WriteOnly | QIODevice::Text)) {
            report.write(theme::resolvedFaces().toUtf8());
            // The icon resolves through a plugin and can fail silently, which
            // leaves the shell drawing its own placeholder. Report it here too.
            const QIcon probe(assetsDir + "/autopm.ico");
            QStringList sizes;
            for (const QSize& size : probe.availableSizes())
                sizes << QString("%1").arg(size.width());
            report.write(QString("\nassets: %1\nicon file exists: %2\nicon loaded: %3\n"
                                 "icon sizes: %4\nimage formats: %5\n")
                             .arg(assetsDir)
                             .arg(QFile::exists(assetsDir + "/autopm.ico") ? "yes" : "no")
                             .arg(probe.isNull() ? "NO" : "yes")
                             .arg(sizes.isEmpty() ? "none" : sizes.join(", "))
                             .arg(QString::fromUtf8(QImageReader::supportedImageFormats().join(' ')))
                             .toUtf8());
        }
        return 0;
    }
    theme::setAccent(QColor("#b76bff"));
    app.setFont(theme::bodyFont(14));
    app.setStyleSheet(theme::styleSheet());
    QIcon mark(assetsDir + "/autopm.ico");
    // The .ico needs Qt's ICO plugin. If that did not deploy, the png still
    // works, and an icon from somewhere beats the shell's placeholder.
    if (mark.isNull()) mark = QIcon(assetsDir + "/autopm.png");
    if (!mark.isNull()) app.setWindowIcon(mark);

    if (definitionsDir.isEmpty()) {
        QMessageBox::critical(nullptr, "AutoPM",
                              "AutoPM could not find its definitions folder.\n\n"
                              "It should sit next to AutoPM.exe. Every screen in the app is "
                              "built from those files, so there is nothing to show without "
                              "them.");
        return 1;
    }

    // --records <dir> reads and writes somewhere other than Documents\AutoPM,
    // the same flag the MCP server takes. For trying things on a copy.
    const int recordsAt = early.indexOf("--records");
    const QString projectsDir =
        recordsAt >= 0 && recordsAt + 1 < early.size()
            ? QDir(early[recordsAt + 1]).absolutePath()
            : QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/AutoPM";
    QDir().mkpath(projectsDir);
    seedFirstRun(assetsDir, projectsDir);

    MainWindow window(definitionsDir, projectsDir);
    theme::applyDarkFrame(&window);
    window.show();

    // --screen <id> opens straight onto one screen. Handy when you live on one.
    const QStringList arguments = app.arguments();

    // --project <name> before anything else, so --screen and --walk land on
    // the project asked for rather than whichever was written to last.
    const int wanted = arguments.indexOf("--project");
    if (wanted >= 0 && wanted + 1 < arguments.size()) {
        if (!window.openProjectNamed(arguments[wanted + 1])) {
            QMessageBox::warning(&window, "AutoPM",
                                 QString("There is no project called \"%1\".")
                                     .arg(arguments[wanted + 1]));
        }
    }

    const int at = arguments.indexOf("--screen");
    if (at >= 0 && at + 1 < arguments.size()) window.showScreenById(arguments[at + 1]);

    // --colour jumps straight to the project's colour picker.
    if (arguments.contains("--colour") || arguments.contains("--color")) window.chooseAccent();

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
