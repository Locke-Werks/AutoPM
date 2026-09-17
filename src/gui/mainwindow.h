// The window: a lifecycle rail on the left, the screen in the middle, the
// "why" panel on the right.
#pragma once

#include "core/workspace.h"

#include <QHash>
#include <QMainWindow>
#include <memory>

class QLabel;
class QPushButton;
class QStackedWidget;
class QVBoxLayout;
class Dashboard;
class GuidedView;
class HelpPanel;
class ScreenView;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(const QString& definitionsDir, const QString& projectsDir,
               QWidget* parent = nullptr);

    bool openProject(const QString& path);
    // By name or id, for the command line. False when there is no such project.
    bool openProjectNamed(const QString& wanted);
    void showScreenById(const QString& screenId);
    // A screen, scrolled to one entry on it.
    void showEntry(const QString& screenId, const QString& fieldId);
    void startWalkthrough(const QString& screenId = QString());
    void chooseAccent();

protected:
    void closeEvent(QCloseEvent* event) override;
    // The MCP server writes the same file, so the window checks for a newer
    // copy whenever it comes back to the front.
    bool event(QEvent* event) override;

private:
    void buildChrome();
    void buildRail();
    void rebuildPages();
    void showPage(int index);

    void newProject();
    void switchProject();
    void save();
    bool confirmDiscard();
    void markDirty();
    void refreshStatus();
    void openRecordsFolder();
    void reloadIfChangedElsewhere();

    pm::Workspace workspace_;
    std::shared_ptr<pm::Record> record_;

    QStackedWidget* shell_ = nullptr;   // the three panes, or the walkthrough
    GuidedView* guided_ = nullptr;
    QWidget* rail_ = nullptr;
    QVBoxLayout* railItems_ = nullptr;
    QPushButton* projectButton_ = nullptr;
    QStackedWidget* pages_ = nullptr;
    HelpPanel* help_ = nullptr;
    Dashboard* dashboard_ = nullptr;
    QLabel* status_ = nullptr;
    QPushButton* saveButton_ = nullptr;

    QHash<QString, int> pageForScreen_;
    QHash<QString, ScreenView*> viewForScreen_;
    QList<QPushButton*> navButtons_;
};
