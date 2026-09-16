#include "guided.h"

#include "boardview.h"
#include "chrome.h"
#include "fieldeditors.h"
#include "tableeditor.h"
#include "theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {

QString qs(const std::string& s) { return QString::fromStdString(s); }

// The thin rail across the top of the walkthrough: one tick per step in the
// current screen, filled up to where you are.
class StepRail : public QWidget {
public:
    explicit StepRail(QWidget* parent = nullptr) : QWidget(parent) {
        setFixedHeight(3);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    void set(int total, int at) {
        total_ = qMax(1, total);
        at_ = at;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);

        const qreal gap = 3;
        const qreal each = (width() - gap * (total_ - 1)) / total_;
        for (int i = 0; i < total_; ++i) {
            painter.setBrush(i <= at_ ? theme::accent() : theme::hairline());
            painter.drawRoundedRect(QRectF(i * (each + gap), 0, each, height()), 1.5, 1.5);
        }
    }

private:
    int total_ = 1;
    int at_ = 0;
};

// A block of teaching: a small coloured label and a paragraph under it.
QWidget* lesson(const QString& label, const QString& text, const QColor& colour, QWidget* parent) {
    if (text.trimmed().isEmpty()) return nullptr;
    auto* holder = new QWidget(parent);
    auto* layout = new QVBoxLayout(holder);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(7);
    layout->addWidget(chrome::label(label, 10, colour));
    layout->addWidget(chrome::bodyText(text, theme::textSecondary(), 14));
    return holder;
}

} // namespace

GuidedView::GuidedView(const pm::Definitions& definitions, QWidget* parent)
    : QWidget(parent), definitions_(definitions) {
    auto* column = new QVBoxLayout(this);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(0);

    // ── top bar: where you are ──
    auto* top = new QWidget(this);
    auto* topLayout = new QVBoxLayout(top);
    topLayout->setContentsMargins(46, 22, 46, 16);
    topLayout->setSpacing(12);

    auto* line = new QHBoxLayout;
    line->setSpacing(12);
    phase_ = new chrome::Eyebrow(QString(), top);
    counter_ = chrome::label(QString(), 10, theme::textFaint());
    leave_ = chrome::button("leave the walkthrough", "quiet", top);
    connect(leave_, &QPushButton::clicked, this, &GuidedView::finished);
    line->addWidget(phase_);
    line->addStretch(1);
    line->addWidget(counter_);
    line->addWidget(leave_);
    topLayout->addLayout(line);

    rail_ = new StepRail(top);
    topLayout->addWidget(rail_);
    column->addWidget(top);

    // ── the step itself ──
    scroll_ = new QScrollArea(this);
    scroll_->setWidgetResizable(true);
    scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_->setFrameShape(QFrame::NoFrame);
    column->addWidget(scroll_, 1);

    // ── the way forward ──
    auto* bottom = new QWidget(this);
    auto* bottomLayout = new QHBoxLayout(bottom);
    bottomLayout->setContentsMargins(46, 14, 46, 22);
    bottomLayout->setSpacing(10);

    back_ = chrome::button("back", QString(), bottom);
    next_ = chrome::button("next", "primary", bottom);
    next_->setMinimumWidth(150);
    connect(back_, &QPushButton::clicked, this, &GuidedView::goBack);
    connect(next_, &QPushButton::clicked, this, &GuidedView::goNext);

    bottomLayout->addWidget(back_);
    bottomLayout->addStretch(1);
    bottomLayout->addWidget(next_);
    column->addWidget(bottom);
}

void GuidedView::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), theme::voidBg());
    chrome::paintBloom(painter, QRect(0, 0, width(), 420), 1.2);
}

void GuidedView::setRecord(const std::shared_ptr<pm::Record>& record) {
    record_ = record;
}

void GuidedView::start(const QString& screenId) {
    if (!record_) return;

    int from = 0;
    const auto& screens = definitions_.screens();
    if (screenId.isEmpty()) {
        // Pick up where the record actually is rather than always at step one.
        const std::string next = pm::nextScreen(definitions_, *record_);
        for (size_t i = 0; i < screens.size(); ++i)
            if (screens[i].id == next) { from = static_cast<int>(i); break; }
    } else {
        for (size_t i = 0; i < screens.size(); ++i)
            if (qs(screens[i].id) == screenId) { from = static_cast<int>(i); break; }
    }
    buildSteps(from);
    showStep(0);
}

void GuidedView::buildSteps(int fromScreen) {
    steps_.clear();
    const auto& screens = definitions_.screens();
    for (int s = fromScreen; s < static_cast<int>(screens.size()); ++s) {
        steps_.push_back({s, -1});
        for (int f = 0; f < static_cast<int>(screens[s].fields.size()); ++f)
            steps_.push_back({s, f});
        steps_.push_back({s, -2});
    }
}

void GuidedView::showStep(int index) {
    if (steps_.empty()) return;
    at_ = qBound(0, index, static_cast<int>(steps_.size()) - 1);
    const Step step = steps_[static_cast<size_t>(at_)];
    const pm::Screen& screen = definitions_.screens()[static_cast<size_t>(step.screen)];

    QWidget* page = nullptr;
    if (step.field == -1)      page = buildLesson(screen);
    else if (step.field == -2) page = buildRecap(screen);
    else                       page = buildField(screen, screen.fields[static_cast<size_t>(step.field)]);

    // One centred reading column. Text measured at 1400px and drawn at 800
    // loses its last line every time.
    page->setMinimumWidth(640);
    page->setMaximumWidth(880);
    auto* centred = new QWidget;
    auto* row = new QHBoxLayout(centred);
    row->setContentsMargins(0, 0, 0, 0);
    // The page carries the stretch, or the two spacers squeeze the column down
    // to whatever width the wrapped text would prefer, which is very little.
    row->addStretch(1);
    row->addWidget(page, 8);
    row->addStretch(1);

    if (QWidget* old = scroll_->takeWidget()) old->deleteLater();
    scroll_->setWidget(centred);

    // Position within this screen, not within the whole record: the screen is
    // the unit a PM actually works in.
    int first = at_, last = at_;
    while (first > 0 && steps_[static_cast<size_t>(first - 1)].screen == step.screen) --first;
    while (last + 1 < static_cast<int>(steps_.size()) &&
           steps_[static_cast<size_t>(last + 1)].screen == step.screen) ++last;
    static_cast<StepRail*>(rail_)->set(last - first + 1, at_ - first);

    phase_->setText(QString("// %1  ·  %2").arg(qs(screen.phase).toUpper(), qs(screen.title).toUpper()));
    if (step.field >= 0)
        counter_->setText(QString("step %1 of %2").arg(at_ - first).arg(last - first - 1));
    else
        counter_->setText(step.field == -1 ? "the lesson" : "what you just did");

    back_->setEnabled(at_ > 0);
    const bool lastStep = at_ == static_cast<int>(steps_.size()) - 1;
    next_->setText(lastStep ? "FINISH" : (step.field == -1 ? "START" : "NEXT"));
}

void GuidedView::goBack() { showStep(at_ - 1); }

void GuidedView::goNext() {
    if (at_ == static_cast<int>(steps_.size()) - 1) {
        emit finished();
        return;
    }
    showStep(at_ + 1);
}

QWidget* GuidedView::buildLesson(const pm::Screen& screen) {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(46, 26, 46, 40);
    layout->setSpacing(22);

    layout->addWidget(new chrome::Heading(qs(screen.title), 44, page));

    if (!screen.process.empty()) {
        auto* badge = new chrome::Badge(qs(screen.process), page);
        badge->setColour(theme::accent());
        auto* row = new QHBoxLayout;
        row->addWidget(badge);
        row->addStretch(1);
        layout->addLayout(row);
    }

    if (QWidget* block = lesson("what this is", qs(screen.intro), theme::textLabel(), page))
        layout->addWidget(block);
    if (QWidget* block = lesson("what you should be able to explain after this",
                                qs(screen.teaches), theme::accent(), page))
        layout->addWidget(block);
    if (QWidget* block = lesson("before you start", qs(screen.before), theme::warn(), page))
        layout->addWidget(block);

    // The order is the lesson. Say what is out of order and why it matters,
    // then let the PM decide.
    const std::vector<std::string> unmet =
        pm::unmetPrerequisites(screen, definitions_, *record_);
    if (!unmet.empty()) {
        QStringList names;
        for (const std::string& name : unmet) names << qs(name);
        auto* card = new chrome::Card(page);
        card->body()->addWidget(chrome::label("out of order", 10, theme::danger()));
        card->body()->addWidget(chrome::bodyText(
            QString("%1 %2 not finished yet. Work done ahead of it is work you may "
                    "have to redo, because this screen depends on decisions that "
                    "have not been made. You can carry on: just do it knowing that.")
                .arg(names.join(" and "), names.size() == 1 ? "is" : "are"),
            theme::textSecondary(), 14));
        layout->addWidget(card);
    }

    if (QWidget* block = lesson("where it comes from", qs(screen.source), theme::textFaint(), page))
        layout->addWidget(block);

    layout->addStretch(1);
    return page;
}

QWidget* GuidedView::teachingBlock(const pm::Field& field, QWidget* parent) {
    auto* card = new chrome::Card(parent);
    card->body()->setSpacing(16);

    if (QWidget* block = lesson("why a pm fills this in", qs(field.why), theme::accent(), card))
        card->body()->addWidget(block);
    if (QWidget* block = lesson("what breaks if you leave it blank", qs(field.ifBlank),
                                theme::danger(), card))
        card->body()->addWidget(block);
    if (QWidget* block = lesson("what a good answer looks like", qs(field.example),
                                theme::ok(), card))
        card->body()->addWidget(block);
    if (QWidget* block = lesson("where it comes from", qs(field.source), theme::textFaint(), card))
        card->body()->addWidget(block);
    return card;
}

QWidget* GuidedView::buildField(const pm::Screen& screen, const pm::Field& field) {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(46, 26, 46, 40);
    layout->setSpacing(20);

    const std::string key = screen.id + "." + field.id;

    // The question, not the form label.
    const QString question = field.prompt.empty() ? qs(field.label) : qs(field.prompt);
    layout->addWidget(new chrome::Heading(question, 32, page));

    auto* tags = new QHBoxLayout;
    tags->setSpacing(8);
    auto* origin = new chrome::Badge(field.origin == "yours" ? "your own" : "standard", page);
    origin->setColour(field.origin == "yours" ? theme::warn() : theme::accent());
    tags->addWidget(origin);
    auto* name = new chrome::Badge(qs(field.label), page);
    name->setColour(theme::textFaint());
    tags->addWidget(name);
    tags->addStretch(1);
    layout->addLayout(tags);

    layout->addWidget(teachingBlock(field, page));

    // ── the answer ──
    auto* answer = new chrome::Card(page);
    answer->setHighlighted(true);
    answer->body()->addWidget(chrome::label("your answer", 10, theme::textLabel()));

    // The coaching line under the editor, refreshed as the answer changes.
    auto* notes = chrome::bodyText(QString(), theme::warn(), 13);
    notes->setVisible(false);

    const auto refreshNotes = [this, &field, key, notes] {
        const std::vector<std::string> failed =
            pm::reviewField(field, record_->entry(key), record_.get());
        if (failed.empty()) {
            notes->setVisible(false);
            return;
        }
        QStringList lines;
        for (const std::string& message : failed) lines << "— " + qs(message);
        notes->setText(lines.join("\n"));
        notes->setVisible(true);
    };

    if (!field.isTable()) {
        QWidget* editor = editors::makeEditor(qs(field.type), field.options,
                                              qs(field.placeholder), answer);
        if (auto* growing = qobject_cast<editors::GrowingTextEdit*>(editor))
            growing->setMinimumLines(4);
        editors::setEditorValue(editor, qs(record_->value(key)));
        editors::connectChanged(editor, this, [this, key, editor, refreshNotes] {
            record_->setValue(key, editors::editorValue(editor).toStdString());
            record_->markDirty();
            refreshNotes();
            emit recordEdited();
        });
        answer->body()->addWidget(editor);
    } else {
        auto* grid = new TableEditor(field, answer);
        if (const std::vector<pm::Row>* rows = record_->rows(key)) grid->setRows(*rows);
        connect(grid, &TableEditor::edited, this, [this, key, grid, refreshNotes] {
            record_->mutableEntry(key).rows = grid->rows();
            record_->markDirty();
            refreshNotes();
            emit recordEdited();
        });
        connect(grid, &TableEditor::rowActivated, this, [this, key, field, grid, refreshNotes](int index) {
            std::vector<pm::Row> rows = record_->mutableEntry(key).rows;
            if (index < 0 || index >= static_cast<int>(rows.size())) return;
            CardDialog dialog(field, rows[static_cast<size_t>(index)], this);
            if (dialog.exec() != QDialog::Accepted) return;
            rows[static_cast<size_t>(index)] = dialog.row();
            record_->mutableEntry(key).rows = rows;
            record_->markDirty();
            grid->setRows(rows);
            refreshNotes();
            emit recordEdited();
        });
        answer->body()->addWidget(grid);
    }

    answer->body()->addWidget(notes);
    layout->addWidget(answer);

    // ── provenance: the habit the whole tool is built around ──
    auto* source = new chrome::Card(page);
    source->body()->addWidget(chrome::label("where this answer came from", 10, theme::textLabel()));
    source->body()->addWidget(chrome::bodyText(
        "Tag it. Specified means someone said it and you can quote them. Agreed means they "
        "approved your summary. Unobjected means you proposed it and nobody answered — which "
        "is not a decision, however much it feels like one.",
        theme::textFaint(), 13));

    auto* provenance = new chrome::ProvenanceControl(source);
    if (const pm::Entry* entry = record_->entry(key))
        provenance->set(entry->provenance, qs(entry->evidence));
    connect(provenance, &chrome::ProvenanceControl::changed, this, [this, key, provenance] {
        pm::Entry& entry = record_->mutableEntry(key);
        entry.provenance = provenance->provenance();
        entry.evidence = provenance->evidence().toStdString();
        record_->markDirty();
        emit recordEdited();
    });
    source->body()->addWidget(provenance);
    layout->addWidget(source);

    refreshNotes();
    layout->addStretch(1);
    return page;
}

QWidget* GuidedView::buildRecap(const pm::Screen& screen) {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(46, 26, 46, 40);
    layout->setSpacing(20);

    const pm::Progress progress = pm::progressOf(screen, *record_);

    layout->addWidget(new chrome::Eyebrow("done with this screen", page));
    layout->addWidget(new chrome::Heading(qs(screen.title), 40, page));

    auto* score = new chrome::Card(page);
    score->body()->addWidget(chrome::label(
        QString("%1 of %2 answered").arg(progress.answered).arg(progress.fields), 11,
        progress.complete() ? theme::ok() : theme::warn()));

    const std::vector<pm::Note> notes = pm::reviewScreen(screen, *record_);
    if (notes.empty() && progress.complete()) {
        score->body()->addWidget(chrome::bodyText(
            "Nothing thin, nothing missing. This artifact would stand up in a review.",
            theme::ok(), 14));
    } else {
        score->body()->addWidget(chrome::bodyText(
            "None of this stops you. It is what someone reviewing this artifact would "
            "ask about.", theme::textFaint(), 13));
        for (const pm::Note& note : notes) {
            const pm::Field* field = screen.field(note.fieldId);
            auto* row = new QWidget(score);
            auto* rowLayout = new QVBoxLayout(row);
            rowLayout->setContentsMargins(0, 6, 0, 0);
            rowLayout->setSpacing(3);
            rowLayout->addWidget(chrome::label(field ? qs(field->label) : qs(note.fieldId), 10,
                                               theme::warn()));
            rowLayout->addWidget(chrome::bodyText(qs(note.message), theme::textSecondary(), 13));
            score->body()->addWidget(row);
        }
    }
    layout->addWidget(score);

    if (QWidget* block = lesson("what this sets up", qs(screen.after), theme::accent(), page))
        layout->addWidget(block);
    if (QWidget* block = lesson("what you should now be able to explain", qs(screen.teaches),
                                theme::textLabel(), page))
        layout->addWidget(block);

    layout->addStretch(1);
    return page;
}
