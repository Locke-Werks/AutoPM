#include "screenview.h"

#include "boardview.h"
#include "chrome.h"
#include "fieldeditors.h"
#include "planviews.h"
#include "tableeditor.h"
#include "core/coach.h"
#include "theme.h"

#include <QButtonGroup>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QStackedWidget>
#include <QVBoxLayout>

ScreenView::ScreenView(const pm::Screen& screen, const std::shared_ptr<pm::Record>& record,
                       const pm::Definitions* definitions, QWidget* parent)
    : QWidget(parent), screen_(screen), record_(record), definitions_(definitions) {
    column_ = new QVBoxLayout(this);
    column_->setContentsMargins(38, 30, 34, 44);
    column_->setSpacing(16);

    column_->addWidget(buildHeader());
    column_->addSpacing(6);
    for (const pm::Field& field : screen_.fields)
        column_->addWidget(buildFieldCard(field));
    column_->addStretch(1);
}

std::string ScreenView::keyFor(const pm::Field& field) const {
    return screen_.id + "." + field.id;
}

void ScreenView::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), theme::voidBg());
    chrome::paintBloom(painter, QRect(0, 0, width(), 320));
}

QWidget* ScreenView::buildHeader() {
    auto* header = new QWidget(this);
    auto* layout = new QVBoxLayout(header);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    QString eyebrow = QString::fromStdString(screen_.phase).toUpper();
    if (!screen_.process.empty())
        eyebrow += "  ·  " + QString::fromStdString(screen_.process).toUpper();
    layout->addWidget(new chrome::Eyebrow(eyebrow, header));

    auto* titleRow = new QHBoxLayout;
    auto* title = new chrome::Heading(QString::fromStdString(screen_.title), 34, header);
    titleRow->addWidget(title, 1);

    auto* teach = chrome::button("walk me through this", "primary", header);
    teach->setToolTip("One question at a time, with what it is for and what a good answer "
                      "looks like.");
    connect(teach, &QPushButton::clicked, this, &ScreenView::walkThrough);
    titleRow->addWidget(teach, 0, Qt::AlignTop);
    layout->addLayout(titleRow);

    const QString intro = QString::fromStdString(screen_.intro).trimmed();
    if (!intro.isEmpty()) {
        layout->addWidget(chrome::bodyText(intro, theme::textSecondary(), 14));
    }
    layout->addSpacing(4);
    layout->addWidget(chrome::rule(header));
    return header;
}

QWidget* ScreenView::buildRichView(const pm::Field& field, TableEditor* grid) {
    RichView* view = nullptr;
    if (field.view == "board")         view = new BoardView(field, this);
    else if (field.view == "tree")     view = new TreeView(field, this);
    else if (field.view == "timeline") view = new TimelineView(field, this);
    else if (field.view == "matrix")   view = new MatrixView(field, this);
    else if (field.view == "log")      view = new LogView(field, this);
    else if (field.view == "planning") {
        auto* planning = new PlanningView(field, this);
        // The planning tool moves cards that belong to another screen, so it
        // needs that screen's field definition to edit one.
        if (const pm::Field* cards = definitions_ ? definitions_->fieldByKey(field.reads) : nullptr)
            planning->setCardField(cards);
        view = planning;
    }
    if (!view) return nullptr;

    view->setRecord(record_);
    connect(view, &RichView::recordChanged, this, [this] { emit recordEdited(); });

    const std::string key = keyFor(field);
    if (const std::vector<pm::Row>* rows = record_->rows(key)) view->setRows(*rows);

    connect(view, &RichView::rowsChanged, this, [this, key, grid](const std::vector<pm::Row>& rows) {
        record_->mutableEntry(key).rows = rows;
        record_->markDirty();
        QSignalBlocker block(grid);
        grid->setRows(rows);
        emit recordEdited();
    });

    // Double-clicking an entry in any of these views opens the full editor,
    // because a grid cell cannot show a paragraph.
    connect(view, &RichView::rowActivated, this, [this, key, grid, field, view](int index) {
        std::vector<pm::Row> rows = record_->mutableEntry(key).rows;
        if (index < 0 || index >= static_cast<int>(rows.size())) return;
        CardDialog dialog(field, rows[static_cast<size_t>(index)], this);
        if (dialog.exec() != QDialog::Accepted) return;
        rows[static_cast<size_t>(index)] = dialog.row();
        record_->mutableEntry(key).rows = rows;
        record_->markDirty();
        QSignalBlocker block(grid);
        grid->setRows(rows);
        view->setRows(rows);
        emit recordEdited();
    });
    return view;
}

QWidget* ScreenView::buildFieldCard(const pm::Field& field) {
    auto* card = new chrome::Card(this);
    const std::string key = keyFor(field);

    auto* head = new QHBoxLayout;
    head->setSpacing(9);

    auto* name = chrome::label(QString::fromStdString(field.label), 11, theme::textLabel());
    head->addWidget(name);

    if (field.origin == "yours") {
        auto* badge = new chrome::Badge("your own", card);
        badge->setColour(theme::warn());
        badge->setToolTip("Not in the standard. This field is your own.");
        head->addWidget(badge);
    }
    head->addStretch(1);

    auto* provenance = new chrome::ProvenanceControl(card);
    const pm::Entry* existing = record_->entry(key);
    if (existing)
        provenance->set(existing->provenance, QString::fromStdString(existing->evidence));
    connect(provenance, &chrome::ProvenanceControl::changed, this, [this, key, provenance] {
        pm::Entry& entry = record_->mutableEntry(key);
        entry.provenance = provenance->provenance();
        entry.evidence = provenance->evidence().toStdString();
        record_->markDirty();
        emit recordEdited();
    });
    head->addWidget(provenance);
    card->body()->addLayout(head);

    auto* notes = chrome::bodyText(QString(), theme::warn(), 12);
    notes->setVisible(false);
    const auto refreshNotes = [this, field, key, notes] {
        const std::vector<std::string> failed = pm::reviewField(field, record_->entry(key));
        if (failed.empty()) { notes->setVisible(false); return; }
        QStringList lines;
        for (const std::string& message : failed)
            lines << "— " + QString::fromStdString(message);
        notes->setText(lines.join("\n"));
        notes->setVisible(true);
    };

    if (!field.isTable()) {
        QWidget* editor = editors::makeEditor(QString::fromStdString(field.type), field.options,
                                              QString::fromStdString(field.placeholder), card);
        editors::setEditorValue(editor, QString::fromStdString(record_->value(key)));
        editors::connectChanged(editor, this, [this, key, editor, refreshNotes] {
            record_->setValue(key, editors::editorValue(editor).toStdString());
            record_->markDirty();
            refreshNotes();
            emit recordEdited();
        });
        editor->installEventFilter(this);
        focusOwners_.insert(editor, QString::fromStdString(field.id));
        card->body()->addWidget(editor);
        card->body()->addWidget(notes);
        refreshNotes();
        return card;
    }

    auto* grid = new TableEditor(field, card);
    if (const std::vector<pm::Row>* rows = record_->rows(key)) grid->setRows(*rows);
    grid->installEventFilter(this);
    focusOwners_.insert(grid, QString::fromStdString(field.id));

    QWidget* rich = buildRichView(field, grid);

    connect(grid, &TableEditor::edited, this, [this, key, grid, rich, refreshNotes] {
        const std::vector<pm::Row> rows = grid->rows();
        record_->mutableEntry(key).rows = rows;
        record_->markDirty();
        if (auto* view = qobject_cast<RichView*>(rich)) {
            QSignalBlocker block(view);
            view->setRows(rows);
        }
        refreshNotes();
        emit recordEdited();
    });
    connect(grid, &TableEditor::columnFocused, this,
            [this, field](const QString& columnId) {
                emit columnFocused(QString::fromStdString(field.id), columnId);
            });
    connect(grid, &TableEditor::rowActivated, this, [this, key, field, grid, rich](int index) {
        std::vector<pm::Row> rows = record_->mutableEntry(key).rows;
        if (index < 0 || index >= static_cast<int>(rows.size())) return;
        CardDialog dialog(field, rows[static_cast<size_t>(index)], this);
        if (dialog.exec() != QDialog::Accepted) return;
        rows[static_cast<size_t>(index)] = dialog.row();
        record_->mutableEntry(key).rows = rows;
        record_->markDirty();
        QSignalBlocker block(grid);
        grid->setRows(rows);
        if (auto* view = qobject_cast<RichView*>(rich)) view->setRows(rows);
        emit recordEdited();
    });

    if (!rich) {
        card->body()->addWidget(grid);
        card->body()->addWidget(notes);
        refreshNotes();
        return card;
    }

    // The rich view leads and the grid is one click away, because the view is
    // what makes the data legible and the grid is what makes it editable.
    auto* stack = new QStackedWidget(card);
    stack->addWidget(rich);
    stack->addWidget(grid);

    auto* toggle = new QHBoxLayout;
    toggle->setSpacing(6);
    auto* viewButton = chrome::button(QString::fromStdString(field.view), "primary", card);
    auto* gridButton = chrome::button("table", QString(), card);
    viewButton->setCheckable(true);
    gridButton->setCheckable(true);
    viewButton->setChecked(true);
    auto* group = new QButtonGroup(card);
    group->addButton(viewButton, 0);
    group->addButton(gridButton, 1);
    group->setExclusive(true);
    connect(group, &QButtonGroup::idClicked, this, [stack, viewButton, gridButton](int id) {
        stack->setCurrentIndex(id);
        viewButton->setProperty("house", id == 0 ? "primary" : "");
        gridButton->setProperty("house", id == 1 ? "primary" : "");
        for (QWidget* w : {static_cast<QWidget*>(viewButton), static_cast<QWidget*>(gridButton)}) {
            w->style()->unpolish(w);
            w->style()->polish(w);
        }
    });
    toggle->addStretch(1);
    toggle->addWidget(viewButton);
    toggle->addWidget(gridButton);

    card->body()->addLayout(toggle);
    card->body()->addWidget(stack);
    card->body()->addWidget(notes);
    refreshNotes();
    return card;
}

bool ScreenView::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::FocusIn || event->type() == QEvent::MouseButtonPress) {
        auto it = focusOwners_.find(watched);
        if (it != focusOwners_.end()) emit fieldFocused(it.value());
    }
    return QWidget::eventFilter(watched, event);
}

void ScreenView::reload() {
    for (auto it = focusOwners_.begin(); it != focusOwners_.end(); ++it) {
        const pm::Field* field = screen_.field(it.value().toStdString());
        if (!field) continue;
        const std::string key = keyFor(*field);
        if (auto* grid = qobject_cast<TableEditor*>(it.key())) {
            if (const std::vector<pm::Row>* rows = record_->rows(key)) grid->setRows(*rows);
        } else if (auto* editor = qobject_cast<QWidget*>(it.key())) {
            editors::setEditorValue(editor, QString::fromStdString(record_->value(key)));
        }
    }
}
