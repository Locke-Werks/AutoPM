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
    void showScreenById(const QString& screenId);
    void startWalkthrough(const QString& screenId = QString());
    void chooseAccent();

protected:
    void closeEvent(QCloseEvent* event) override;

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
