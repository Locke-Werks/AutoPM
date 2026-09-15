#include "mainwindow.h"

#include "accentdialog.h"
#include "chrome.h"
#include "dashboard.h"
#include "guided.h"
#include "helppanel.h"
#include "screenview.h"
#include "theme.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QScrollArea>
#include <QShortcut>
#include <QStackedWidget>
#include <QStatusBar>
#include <QUrl>
#include <QVBoxLayout>

namespace {

// A rail entry: flat, with an accent bar down the left edge when it is the
// screen you are on.
class NavButton : public QPushButton {
public:
    NavButton(const QString& text, bool isPhaseChild, QWidget* parent = nullptr)
        : QPushButton(text, parent), child_(isPhaseChild) {
        setCheckable(true);
        setCursor(Qt::PointingHandCursor);
        setFont(theme::bodyFont(13));
        setFixedHeight(32);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const bool on = isChecked();
        const bool hot = underMouse();

        if (on) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(theme::accentAt(26));
            painter.drawRoundedRect(QRectF(6, 1, width() - 8, height() - 2), 5, 5);
            painter.setBrush(theme::accent());
            painter.drawRoundedRect(QRectF(0, 6, 2.5, height() - 12), 1.2, 1.2);
        } else if (hot) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(theme::accentAt(16));
            painter.drawRoundedRect(QRectF(6, 1, width() - 8, height() - 2), 5, 5);
        }

        painter.setPen(on ? theme::textPrimary() : (hot ? theme::textBody() : theme::textSecondary()));
        painter.setFont(font());
        const int left = child_ ? 20 : 14;
        painter.drawText(QRect(left, 0, width() - left - 10, height()),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         painter.fontMetrics().elidedText(text(), Qt::ElideRight,
                                                          width() - left - 10));
    }

private:
    bool child_ = false;
};

QString elide(const QString& text, int length) {
    return text.size() <= length ? text : text.left(length - 1) + "…";
}

} // namespace

MainWindow::MainWindow(const QString& definitionsDir, const QString& projectsDir, QWidget* parent)
    : QMainWindow(parent) {
    std::string error;
    if (!workspace_.open(definitionsDir.toStdString(), projectsDir.toStdString(), error)) {
        QMessageBox::critical(this, "AutoPM",
                              QString("AutoPM could not load its screen definitions.\n\n%1")
                                  .arg(QString::fromStdString(error)));
        return;
    }
    buildChrome();

    const std::vector<pm::ProjectSummary> projects = workspace_.list();
    if (!projects.empty()) openProject(QString::fromStdString(projects.front().path));
    else newProject();
}

void MainWindow::buildChrome() {
    setWindowTitle("AutoPM");
    resize(1500, 950);
    setMinimumSize(1100, 700);

    shell_ = new QStackedWidget(this);

    auto* central = new QWidget(shell_);
    auto* row = new QHBoxLayout(central);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(0);

    // ── rail ──
    rail_ = new QWidget(central);
    rail_->setFixedWidth(248);
    rail_->setAutoFillBackground(true);
    QPalette railPalette = rail_->palette();
    railPalette.setColor(QPalette::Window, theme::surface());
    rail_->setPalette(railPalette);

    auto* railLayout = new QVBoxLayout(rail_);
    railLayout->setContentsMargins(12, 18, 12, 14);
    railLayout->setSpacing(4);

    auto* wordmark = new chrome::Heading("AutoPM", 23, rail_);
    wordmark->setContentsMargins(8, 0, 0, 0);
    railLayout->addWidget(wordmark);
    auto* tagline = chrome::label("the work, and why", 9, theme::accent());
    tagline->setContentsMargins(8, 0, 0, 0);
    railLayout->addWidget(tagline);
    railLayout->addSpacing(16);

    projectButton_ = new QPushButton("No project", rail_);
    projectButton_->setCursor(Qt::PointingHandCursor);
    projectButton_->setFont(theme::bodyFont(13));
    projectButton_->setProperty("house", "primary");
    projectButton_->setMinimumHeight(38);
    connect(projectButton_, &QPushButton::clicked, this, &MainWindow::switchProject);
    railLayout->addWidget(projectButton_);
    railLayout->addSpacing(14);

    railItems_ = new QVBoxLayout;
    railItems_->setSpacing(2);
    railLayout->addLayout(railItems_);
    railLayout->addStretch(1);

    auto* folder = chrome::button("records folder", "quiet", rail_);
    connect(folder, &QPushButton::clicked, this, &MainWindow::openRecordsFolder);
    railLayout->addWidget(folder);

    // ── pages ──
    pages_ = new QStackedWidget(central);

    // ── help ──
    help_ = new HelpPanel(central);

    row->addWidget(rail_);
    row->addWidget(pages_, 1);
    row->addWidget(help_);

    guided_ = new GuidedView(workspace_.definitions(), shell_);
    connect(guided_, &GuidedView::recordEdited, this, &MainWindow::markDirty);
    connect(guided_, &GuidedView::finished, this, [this] {
        shell_->setCurrentIndex(0);
        rebuildPages();
        buildRail();
        showPage(0);
    });

    shell_->addWidget(central);
    shell_->addWidget(guided_);
    setCentralWidget(shell_);

    status_ = new QLabel(this);
    status_->setFont(theme::bodyFont(12));
    QPalette statusPalette = status_->palette();
    statusPalette.setColor(QPalette::WindowText, theme::textFaint());
    status_->setPalette(statusPalette);

    saveButton_ = chrome::button("save", "primary", this);
    connect(saveButton_, &QPushButton::clicked, this, &MainWindow::save);

    statusBar()->addWidget(status_, 1);
    statusBar()->addPermanentWidget(saveButton_);
    statusBar()->setStyleSheet(QString("QStatusBar { background: %1; border-top: 1px solid %2; }"
                                       " QStatusBar::item { border: 0; }")
                                   .arg(theme::surface().name(), chrome::hairlineCss()));

    new QShortcut(QKeySequence::Save, this, [this] { save(); });
    new QShortcut(QKeySequence::New, this, [this] { newProject(); });
}

bool MainWindow::openProject(const QString& path) {
    std::string error;
    auto record = workspace_.load(path.toStdString(), error);
    if (!record) {
        QMessageBox::warning(this, "AutoPM",
                             QString("Could not open that record.\n\n%1")
                                 .arg(QString::fromStdString(error)));
        return false;
    }
    record_ = record;
    if (guided_) guided_->setRecord(record_);
    if (!record_->accent.empty()) theme::setAccent(QColor(QString::fromStdString(record_->accent)));
    qApp->setStyleSheet(theme::styleSheet());

    rebuildPages();
    buildRail();
    refreshStatus();
    return true;
}

void MainWindow::rebuildPages() {
    while (pages_->count() > 0) {
        QWidget* page = pages_->widget(0);
        pages_->removeWidget(page);
        page->deleteLater();
    }
    pageForScreen_.clear();
    viewForScreen_.clear();

    const auto wrap = [this](QWidget* content) {
        auto* scroll = new QScrollArea(pages_);
        scroll->setWidgetResizable(true);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setWidget(content);
        return scroll;
    };

    dashboard_ = new Dashboard(workspace_.definitions(), record_, pages_);
    connect(dashboard_, &Dashboard::openScreen, this, &MainWindow::showScreenById);
    connect(dashboard_, &Dashboard::walkThrough, this,
            [this](const QString& screenId) { startWalkthrough(screenId); });
    pages_->addWidget(wrap(dashboard_));

    for (const pm::Screen& screen : workspace_.definitions().screens()) {
        auto* view = new ScreenView(screen, record_, pages_);
        const QString screenId = QString::fromStdString(screen.id);

        connect(view, &ScreenView::fieldFocused, this, [this, screenId](const QString& fieldId) {
            const pm::Screen* screen = workspace_.definitions().screen(screenId.toStdString());
            if (!screen) return;
            if (const pm::Field* field = screen->field(fieldId.toStdString()))
                help_->showField(*screen, *field);
        });
        connect(view, &ScreenView::columnFocused, this,
                [this, screenId](const QString& fieldId, const QString& columnId) {
                    const pm::Screen* screen = workspace_.definitions().screen(screenId.toStdString());
                    if (!screen) return;
                    const pm::Field* field = screen->field(fieldId.toStdString());
                    if (!field) return;
                    if (const pm::Column* column = field->column(columnId.toStdString()))
                        help_->showColumn(*screen, *field, *column);
                });
        connect(view, &ScreenView::recordEdited, this, &MainWindow::markDirty);
        connect(view, &ScreenView::walkThrough, this,
                [this, screenId] { startWalkthrough(screenId); });

        pageForScreen_.insert(screenId, pages_->count());
        viewForScreen_.insert(screenId, view);
        pages_->addWidget(wrap(view));
    }
    showPage(0);
}

void MainWindow::buildRail() {
    while (QLayoutItem* item = railItems_->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    navButtons_.clear();

    auto* overview = new NavButton("Overview", false, rail_);
    overview->setChecked(true);
    connect(overview, &QPushButton::clicked, this, [this] { showPage(0); });
    railItems_->addWidget(overview);
    navButtons_ << overview;
    railItems_->addSpacing(10);

    QString lastPhase;
    for (const pm::Screen& screen : workspace_.definitions().screens()) {
        const QString phase = QString::fromStdString(screen.phase);
        if (phase != lastPhase) {
            lastPhase = phase;
            auto* header = chrome::label(phase, 9, theme::textFaint());
            header->setContentsMargins(14, 12, 6, 3);
            header->setWordWrap(true);
            railItems_->addWidget(header);
        }
        auto* item = new NavButton(QString::fromStdString(screen.title), true, rail_);
        const QString screenId = QString::fromStdString(screen.id);
        connect(item, &QPushButton::clicked, this, [this, screenId] { showScreenById(screenId); });
        railItems_->addWidget(item);
        navButtons_ << item;
    }

    projectButton_->setText(elide(QString::fromStdString(record_->name), 24));
}

void MainWindow::showPage(int index) {
    if (index < 0 || index >= pages_->count()) return;
    pages_->setCurrentIndex(index);

    for (int i = 0; i < navButtons_.size(); ++i)
        navButtons_[i]->setChecked(i == index);

    if (index == 0) {
        dashboard_->refresh();
        // Named members, not an aggregate initialiser: adding a field to
        // Screen must not silently shift a string into the wrong slot.
        pm::Screen overview;
        overview.id = "overview";
        overview.title = "Overview";
        overview.phase = "This project";
        overview.intro =
            "Everything the record claims, and everything it does not claim yet.\n\n"
            "The lifecycle list shows how much of each screen is filled in. The gaps list "
            "underneath is the interesting half: each blank field comes with the sentence "
            "explaining what goes wrong when nobody fills it in.";
        overview.source = "AutoPM's own screen, not a PMBOK artifact.";
        help_->showScreen(overview);
        return;
    }
    const auto& screens = workspace_.definitions().screens();
    if (index - 1 < static_cast<int>(screens.size())) help_->showScreen(screens[index - 1]);
}

void MainWindow::startWalkthrough(const QString& screenId) {
    if (!record_) return;
    guided_->setRecord(record_);
    guided_->start(screenId);
    shell_->setCurrentIndex(1);
}

void MainWindow::showScreenById(const QString& screenId) {
    const auto it = pageForScreen_.find(screenId);
    if (it != pageForScreen_.end()) showPage(it.value());
}

void MainWindow::newProject() {
    if (!confirmDiscard()) return;
    bool accepted = false;
    const QString name = QInputDialog::getText(this, "New project",
                                               "Project name\n\nThe charter comes next: a project "
                                               "is not authorized until its sponsor signs one.",
                                               QLineEdit::Normal, QString(), &accepted);
    if (!accepted || name.trimmed().isEmpty()) return;

    std::string error;
    auto record = workspace_.create(name.trimmed().toStdString(), "#b76bff", error);
    if (!record) {
        QMessageBox::warning(this, "AutoPM", QString::fromStdString(error));
        return;
    }
    openProject(QString::fromStdString(record->path));
}

void MainWindow::switchProject() {
    QMenu menu(this);
    for (const pm::ProjectSummary& summary : workspace_.list()) {
        QAction* action = menu.addAction(QString("%1    %2")
                                             .arg(QString::fromStdString(summary.name))
                                             .arg(QString::fromStdString(summary.modified)));
        const QString path = QString::fromStdString(summary.path);
        action->setCheckable(true);
        action->setChecked(record_ && QString::fromStdString(record_->path) == path);
        connect(action, &QAction::triggered, this, [this, path] {
            if (!confirmDiscard()) return;
            openProject(path);
        });
    }
    menu.addSeparator();
    connect(menu.addAction("Project colour…"), &QAction::triggered, this, &MainWindow::chooseAccent);
    connect(menu.addAction("New project…"), &QAction::triggered, this, &MainWindow::newProject);
    connect(menu.addAction("Open the records folder"), &QAction::triggered, this,
            &MainWindow::openRecordsFolder);
    menu.exec(projectButton_->mapToGlobal(QPoint(0, projectButton_->height() + 4)));
}

void MainWindow::save() {
    if (!record_) return;
    std::string error;
    if (!record_->save(record_->path, error)) {
        QMessageBox::warning(this, "AutoPM",
                             QString("Could not save the record.\n\n%1")
                                 .arg(QString::fromStdString(error)));
        return;
    }
    dashboard_->refresh();
    refreshStatus();
}

void MainWindow::markDirty() {
    refreshStatus();
}

void MainWindow::refreshStatus() {
    if (!record_) return;
    const bool dirty = record_->dirty();
    setWindowTitle(QString("%1 — AutoPM%2")
                       .arg(QString::fromStdString(record_->name))
                       .arg(dirty ? " •" : ""));
    status_->setText(QString("%1%2")
                         .arg(QString::fromStdString(record_->path))
                         .arg(dirty ? "     unsaved changes" : "     saved"));
    saveButton_->setEnabled(dirty);
    saveButton_->setText(dirty ? "SAVE" : "SAVED");
}

bool MainWindow::confirmDiscard() {
    if (!record_ || !record_->dirty()) return true;
    const auto answer = QMessageBox::question(
        this, "Unsaved changes",
        QString("%1 has changes that are not written to its record file yet.")
            .arg(QString::fromStdString(record_->name)),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if (answer == QMessageBox::Cancel) return false;
    if (answer == QMessageBox::Save) save();
    return true;
}

void MainWindow::chooseAccent() {
    if (!record_) return;
    AccentDialog dialog(QString::fromStdString(record_->name), theme::accent(), this);
    if (dialog.exec() != QDialog::Accepted) return;
    if (dialog.chosen() == theme::accent()) return;

    record_->accent = dialog.chosen().name().toStdString();
    record_->markDirty();
    theme::setAccent(dialog.chosen());
    qApp->setStyleSheet(theme::styleSheet());

    // The accent is painted, not style-sheeted, in most places, so the pages
    // are rebuilt rather than restyled.
    rebuildPages();
    buildRail();
    refreshStatus();
}

void MainWindow::openRecordsFolder() {
    QDesktopServices::openUrl(
        QUrl::fromLocalFile(QString::fromStdString(workspace_.projectsDir())));
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (confirmDiscard()) event->accept();
    else event->ignore();
}
