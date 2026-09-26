#include "mainwindow.h"

#include "accentdialog.h"
#include "chrome.h"
#include "dashboard.h"
#include "guided.h"
#include "helppanel.h"
#include "screenview.h"
#include "theme.h"

#include <algorithm>

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QScrollArea>
#include <QTimer>
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

// A project in the far-left list: its own colour as a dot, so the list reads
// at a glance before any name is, and the phase underneath.
class ProjectButton : public QPushButton {
public:
    ProjectButton(const pm::ProjectSummary& summary, QWidget* parent = nullptr)
        : QPushButton(QString::fromStdString(summary.name), parent),
          phase_(QString::fromStdString(summary.phase)),
          colour_(QString::fromStdString(summary.accent)) {
        if (!colour_.isValid()) colour_ = theme::textFaint();
        setCheckable(true);
        setCursor(Qt::PointingHandCursor);
        setFont(theme::bodyFont(13));
        setFixedHeight(phase_.isEmpty() ? 32 : 44);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setToolTip(text());
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const bool on = isChecked();
        const bool hot = underMouse();

        if (on || hot) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(theme::accentAt(on ? 26 : 16));
            painter.drawRoundedRect(QRectF(6, 1, width() - 8, height() - 2), 5, 5);
        }
        if (on) {
            painter.setBrush(colour_);
            painter.drawRoundedRect(QRectF(0, 6, 2.5, height() - 12), 1.2, 1.2);
        }

        const int left = 30;
        const int textWidth = width() - left - 10;
        const int nameTop = phase_.isEmpty() ? 0 : 5;
        const int nameHeight = phase_.isEmpty() ? height() : 20;

        painter.setPen(Qt::NoPen);
        painter.setBrush(colour_);
        painter.drawEllipse(QPointF(18, nameTop + nameHeight / 2.0), 4, 4);

        painter.setPen(on ? theme::textPrimary() : (hot ? theme::textBody() : theme::textSecondary()));
        painter.setFont(font());
        painter.drawText(QRect(left, nameTop, textWidth, nameHeight), Qt::AlignLeft | Qt::AlignVCenter,
                         painter.fontMetrics().elidedText(text(), Qt::ElideRight, textWidth));

        if (!phase_.isEmpty()) {
            painter.setPen(theme::textFaint());
            painter.setFont(theme::bodyFont(11));
            painter.drawText(QRect(left, nameTop + nameHeight, textWidth, 15),
                             Qt::AlignLeft | Qt::AlignVCenter,
                             painter.fontMetrics().elidedText(phase_, Qt::ElideRight, textWidth));
        }
    }

private:
    QString phase_;
    QColor colour_;
};

// The project column's ground, with a hairline between it and the rail. Both
// are the raised surface, and without the line they read as one wide rail.
class ProjectColumn : public QWidget {
public:
    using QWidget::QWidget;

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.fillRect(rect(), theme::surface());
        painter.fillRect(QRect(width() - 1, 0, 1, height()), theme::hairline());
    }
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
    resize(1720, 950);
    setMinimumSize(1320, 700);

    shell_ = new QStackedWidget(this);

    auto* central = new QWidget(shell_);
    auto* row = new QHBoxLayout(central);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(0);

    // ── projects ──
    auto* projects = new ProjectColumn(central);
    projects->setFixedWidth(220);
    auto* projectsLayout = new QVBoxLayout(projects);
    projectsLayout->setContentsMargins(8, 24, 9, 14);
    projectsLayout->setSpacing(4);

    auto* projectsHeader = chrome::label("Projects", 9, theme::textFaint());
    projectsHeader->setContentsMargins(14, 0, 6, 6);
    projectsLayout->addWidget(projectsHeader);

    auto* projectScroll = new QScrollArea(projects);
    projectScroll->setWidgetResizable(true);
    projectScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    projectScroll->setFrameShape(QFrame::NoFrame);
    auto* projectHolder = new QWidget(projectScroll);
    projectItems_ = new QVBoxLayout(projectHolder);
    projectItems_->setContentsMargins(0, 0, 0, 0);
    projectItems_->setSpacing(2);
    projectItems_->addStretch(1);
    projectScroll->setWidget(projectHolder);
    projectsLayout->addWidget(projectScroll, 1);

    auto* create = chrome::button("new project", "quiet", projects);
    connect(create, &QPushButton::clicked, this, &MainWindow::newProject);
    projectsLayout->addWidget(create);

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
    connect(projectButton_, &QPushButton::clicked, this, &MainWindow::projectMenu);
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

    row->addWidget(projects);
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
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageUp), this, [this] { stepProject(-1); });
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageDown), this, [this] { stepProject(1); });
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
    buildProjectList();
    refreshStatus();
    return true;
}

bool MainWindow::openProjectNamed(const QString& wanted) {
    for (const pm::ProjectSummary& summary : workspace_.list()) {
        if (QString::fromStdString(summary.id) != wanted &&
            QString::fromStdString(summary.name).compare(wanted, Qt::CaseInsensitive) != 0)
            continue;
        return openProject(QString::fromStdString(summary.path));
    }
    return false;
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
    connect(dashboard_, &Dashboard::openEntry, this, &MainWindow::showEntry);
    connect(dashboard_, &Dashboard::recordEdited, this, &MainWindow::markDirty);
    connect(dashboard_, &Dashboard::walkThrough, this,
            [this](const QString& screenId) { startWalkthrough(screenId); });
    pages_->addWidget(wrap(dashboard_));

    // Empty pages, filled on first visit. Building every screen up front cost
    // most of a second per project switch, for screens that mostly go unseen.
    for (const pm::Screen& screen : workspace_.definitions().screens()) {
        pageForScreen_.insert(QString::fromStdString(screen.id), pages_->count());
        pages_->addWidget(wrap(new QWidget));
    }
    showPage(0);
}

ScreenView* MainWindow::ensureView(int index) {
    const auto& screens = workspace_.definitions().screens();
    if (index < 1 || index - 1 >= static_cast<int>(screens.size())) return nullptr;
    const pm::Screen& screen = screens[index - 1];
    const QString screenId = QString::fromStdString(screen.id);
    if (ScreenView* existing = viewForScreen_.value(screenId)) return existing;

    auto* scroll = qobject_cast<QScrollArea*>(pages_->widget(index));
    if (!scroll) return nullptr;

    auto* view = new ScreenView(screen, record_, &workspace_.definitions(), scroll);
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

    viewForScreen_.insert(screenId, view);
    scroll->setWidget(view);   // deletes the placeholder
    return view;
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

// By name, not by last edit. The records are sorted newest first everywhere
// else, but a list you click through by position should not reorder itself
// every time you save.
void MainWindow::buildProjectList() {
    while (projectItems_->count() > 1) {
        QLayoutItem* item = projectItems_->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    projectPaths_.clear();

    std::vector<pm::ProjectSummary> projects = workspace_.list();
    std::stable_sort(projects.begin(), projects.end(),
                     [](const pm::ProjectSummary& a, const pm::ProjectSummary& b) {
                         return QString::fromStdString(a.name).compare(
                                    QString::fromStdString(b.name), Qt::CaseInsensitive) < 0;
                     });

    const QString current = record_ ? QString::fromStdString(record_->path) : QString();
    for (const pm::ProjectSummary& summary : projects) {
        const QString path = QString::fromStdString(summary.path);
        auto* item = new ProjectButton(summary);
        item->setChecked(QFileInfo(path) == QFileInfo(current));
        connect(item, &QPushButton::clicked, this, [this, item, path] {
            // A checkable button unchecks itself on click; the list only
            // changes when the switch actually happens.
            item->setChecked(record_ && QFileInfo(path) == QFileInfo(QString::fromStdString(record_->path)));
            switchTo(path);
        });
        projectItems_->insertWidget(projectItems_->count() - 1, item);
        projectPaths_ << path;
    }
}

void MainWindow::showPage(int index) {
    if (index < 0 || index >= pages_->count()) return;
    ensureView(index);
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

void MainWindow::showEntry(const QString& screenId, const QString& fieldId) {
    showScreenById(screenId);
    if (fieldId.isEmpty()) return;

    const auto view = viewForScreen_.find(screenId);
    const auto page = pageForScreen_.find(screenId);
    if (view == viewForScreen_.end() || page == pageForScreen_.end()) return;

    QWidget* card = view.value()->fieldCard(fieldId);
    auto* scroll = qobject_cast<QScrollArea*>(pages_->widget(page.value()));
    if (!card || !scroll) return;

    // After the page has been laid out. Scrolling to a widget whose geometry is
    // still the one it had on the previous page lands somewhere arbitrary.
    QTimer::singleShot(0, this, [scroll, card] {
        scroll->ensureWidgetVisible(card, 0, 60);
    });
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

// Settings for the open project only. Choosing a project is the list's job.
void MainWindow::projectMenu() {
    if (!record_) return;
    QMenu menu(this);
    connect(menu.addAction("Colour…"), &QAction::triggered, this, &MainWindow::chooseAccent);
    connect(menu.addAction("Rename…"), &QAction::triggered, this, &MainWindow::renameProject);
    connect(menu.addAction("Open the records folder"), &QAction::triggered, this,
            &MainWindow::openRecordsFolder);
    menu.exec(projectButton_->mapToGlobal(QPoint(0, projectButton_->height() + 4)));
}

// The record file is named by id, not by name, so a rename touches nothing
// on disk until the next save writes the new name into it.
void MainWindow::renameProject() {
    if (!record_) return;
    bool accepted = false;
    const QString current = QString::fromStdString(record_->name);
    const QString name = QInputDialog::getText(this, "Rename project", "Project name",
                                               QLineEdit::Normal, current, &accepted).trimmed();
    if (!accepted || name.isEmpty() || name == current) return;

    record_->name = name.toStdString();
    record_->markDirty();
    // The overview's heading is the name, painted once when the page is built.
    rebuildPages();
    buildRail();
    refreshStatus();
}

// Staying on the same screen is what makes comparing two projects quick: the
// risks of one, then the risks of the next, without a trip through Overview.
void MainWindow::switchTo(const QString& path) {
    if (record_ && QFileInfo(path) == QFileInfo(QString::fromStdString(record_->path))) return;
    if (!confirmDiscard()) return;
    const int page = pages_ ? pages_->currentIndex() : 0;
    if (openProject(path)) showPage(page);
}

void MainWindow::stepProject(int delta) {
    if (projectPaths_.isEmpty() || shell_->currentIndex() != 0) return;
    int at = -1;
    if (record_) {
        const QFileInfo current(QString::fromStdString(record_->path));
        for (int i = 0; i < projectPaths_.size(); ++i)
            if (QFileInfo(projectPaths_[i]) == current) at = i;
    }
    if (at < 0) at = delta > 0 ? -1 : 0;
    const int count = static_cast<int>(projectPaths_.size());
    switchTo(projectPaths_[((at + delta) % count + count) % count]);
}

bool MainWindow::event(QEvent* event) {
    if (event->type() == QEvent::WindowActivate) {
        reloadIfChangedElsewhere();
        // The MCP server can create projects too.
        if (projectItems_) buildProjectList();
    }
    return QMainWindow::event(event);
}

// Somebody else wrote the record while this window had it open. With no edits
// of our own there is nothing to weigh up, so take theirs.
void MainWindow::reloadIfChangedElsewhere() {
    if (!record_ || !record_->changedOnDisk()) return;
    if (record_->dirty()) {
        statusBar()->showMessage(
            "This record was changed by something else while you had unsaved edits. Saving will "
            "ask you which copy to keep.", 12000);
        return;
    }
    std::string error;
    const int page = pages_ ? pages_->currentIndex() : 0;
    if (!record_->reload(error)) return;
    rebuildPages();
    buildRail();
    showPage(page);
    refreshStatus();
    statusBar()->showMessage("Reloaded: this record was changed by something else.", 6000);
}

void MainWindow::save() {
    if (!record_) return;
    std::string error;

    // Never write a copy loaded before somebody else's changes. Doing that
    // destroys their work with no warning, which is exactly how a decision log
    // loses entries.
    if (record_->changedOnDisk()) {
        QMessageBox box(this);
        box.setWindowTitle("Changed elsewhere");
        box.setText("This record has been written by something else since you opened it.");
        box.setInformativeText(
            "The MCP server and this window hold the same file. Saving now would overwrite "
            "whatever it wrote.");
        QPushButton* keepTheirs = box.addButton("Reload theirs, lose mine", QMessageBox::DestructiveRole);
        QPushButton* keepMine = box.addButton("Overwrite with mine", QMessageBox::AcceptRole);
        box.addButton(QMessageBox::Cancel);
        box.exec();

        if (box.clickedButton() == keepTheirs) {
            const int page = pages_ ? pages_->currentIndex() : 0;
            if (record_->reload(error)) {
                rebuildPages();
                buildRail();
                showPage(page);
                refreshStatus();
            }
            return;
        }
        if (box.clickedButton() != keepMine) return;
    }
    if (!record_->save(record_->path, error)) {
        QMessageBox::warning(this, "AutoPM",
                             QString("Could not save the record.\n\n%1")
                                 .arg(QString::fromStdString(error)));
        return;
    }
    dashboard_->refresh();
    buildProjectList();   // the phase under the name may have moved
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
