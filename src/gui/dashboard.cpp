#include "dashboard.h"

#include "chrome.h"
#include "core/coach.h"
#include "theme.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QVBoxLayout>

namespace {

struct Counts {
    int fieldsTotal = 0;
    int fieldsFilled = 0;
    int rows = 0;
    int specified = 0;
    int agreed = 0;
    int unobjected = 0;
    int untagged = 0;
};

bool entryHasContent(const pm::Entry* entry) {
    return entry && !entry->isEmpty();
}

void tally(Counts& counts, pm::Provenance provenance) {
    switch (provenance) {
        case pm::Provenance::Specified:  ++counts.specified; break;
        case pm::Provenance::Agreed:     ++counts.agreed; break;
        case pm::Provenance::Unobjected: ++counts.unobjected; break;
        case pm::Provenance::Untagged:   ++counts.untagged; break;
    }
}

Counts countScreen(const pm::Screen& screen, const pm::Record& record) {
    Counts counts;
    for (const pm::Field& field : screen.fields) {
        ++counts.fieldsTotal;
        const pm::Entry* entry = record.entry(screen.id + "." + field.id);
        if (!entryHasContent(entry)) continue;
        ++counts.fieldsFilled;
        tally(counts, entry->provenance);
        counts.rows += static_cast<int>(entry->rows.size());
        for (const pm::Row& row : entry->rows) tally(counts, row.provenance);
    }
    return counts;
}

// A number with its label under it, in a traced-edge card.
chrome::Card* statCard(const QString& value, const QString& caption, const QColor& colour,
                       QWidget* parent) {
    auto* card = new chrome::Card(parent);
    card->body()->setContentsMargins(20, 17, 20, 16);
    card->body()->setSpacing(5);

    auto* number = new QLabel(value, card);
    number->setFont(theme::serifFont(30));
    QPalette palette = number->palette();
    palette.setColor(QPalette::WindowText, colour);
    number->setPalette(palette);

    auto* text = chrome::label(caption, 10, theme::textLabel());
    text->setWordWrap(true);

    card->body()->addWidget(number);
    card->body()->addWidget(text);
    card->setMinimumWidth(150);
    return card;
}

// A thin progress rail. Filled portion in the accent; the rest a hairline.
class Meter : public QWidget {
public:
    Meter(qreal fraction, QWidget* parent = nullptr) : QWidget(parent), fraction_(fraction) {
        setFixedHeight(4);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(190, 120, 255, 30));
        painter.drawRoundedRect(QRectF(0, 0, width(), height()), 2, 2);
        if (fraction_ <= 0) return;
        painter.setBrush(theme::accent());
        painter.drawRoundedRect(QRectF(0, 0, width() * qBound(0.0, fraction_, 1.0), height()), 2, 2);
    }

private:
    qreal fraction_;
};

} // namespace

Dashboard::Dashboard(const pm::Definitions& definitions, const std::shared_ptr<pm::Record>& record,
                     QWidget* parent)
    : QWidget(parent), definitions_(definitions), record_(record) {
    column_ = new QVBoxLayout(this);
    column_->setContentsMargins(38, 30, 34, 44);
    column_->setSpacing(16);
    refresh();
}

void Dashboard::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), theme::voidBg());
    chrome::paintBloom(painter, QRect(0, 0, width(), 360), 1.3);
}

void Dashboard::refresh() {
    while (QLayoutItem* item = column_->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    column_->addWidget(buildHeader());
    column_->addSpacing(8);
    column_->addWidget(buildNextStep());
    column_->addSpacing(8);
    column_->addWidget(buildStats());
    column_->addSpacing(8);
    column_->addWidget(buildPhases());
    column_->addSpacing(8);
    column_->addWidget(buildGaps());
    column_->addStretch(1);
}

QWidget* Dashboard::buildHeader() {
    auto* header = new QWidget(this);
    auto* layout = new QVBoxLayout(header);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    layout->addWidget(new chrome::Eyebrow("project record", header));
    layout->addWidget(new chrome::Heading(QString::fromStdString(record_->name), 38, header));

    auto* meta = new QHBoxLayout;
    meta->setSpacing(8);
    if (!record_->phase.empty()) {
        auto* phase = new chrome::Badge(QString::fromStdString(record_->phase), header);
        phase->setColour(theme::accent());
        meta->addWidget(phase);
    }
    auto* opened = new chrome::Badge(QString("opened %1").arg(QString::fromStdString(record_->created)),
                                     header);
    opened->setColour(theme::textLabel());
    meta->addWidget(opened);
    if (!record_->modified.empty()) {
        auto* saved = new chrome::Badge(
            QString("saved %1").arg(QString::fromStdString(record_->modified)), header);
        saved->setColour(theme::textFaint());
        meta->addWidget(saved);
    }
    meta->addStretch(1);
    layout->addLayout(meta);

    const QString summary = QString::fromStdString(record_->summary).trimmed();
    if (!summary.isEmpty()) {
        layout->addWidget(chrome::bodyText(summary, theme::textSecondary(), 14));
    }
    layout->addSpacing(4);
    layout->addWidget(chrome::rule(header));
    return header;
}

// The front page leads with the next thing to do, not with a score. The
// point of the tool is to walk someone through the job.
QWidget* Dashboard::buildNextStep() {
    auto* card = new chrome::Card(this);
    card->setHighlighted(true);
    card->body()->setSpacing(13);

    const std::string next = pm::nextScreen(definitions_, *record_);
    const pm::Screen* screen = next.empty() ? nullptr : definitions_.screen(next);

    if (!screen) {
        card->body()->addWidget(chrome::label("nothing outstanding", 11, theme::ok()));
        card->body()->addWidget(chrome::bodyText(
            "Every screen has an answer in every field. What is left is judgement: open any "
            "screen and read what the coach says about the answers you gave.",
            theme::textSecondary(), 14));
        auto* again = chrome::button("walk it again", QString(), card);
        connect(again, &QPushButton::clicked, this, [this] { emit walkThrough(QString()); });
        auto* row = new QHBoxLayout;
        row->addWidget(again);
        row->addStretch(1);
        card->body()->addLayout(row);
        return card;
    }

    const pm::Progress progress = pm::progressOf(*screen, *record_);
    card->body()->addWidget(chrome::label("what to do next", 11, theme::accent()));
    card->body()->addWidget(new chrome::Heading(QString::fromStdString(screen->title), 27, card));

    QString because = QString::fromStdString(screen->teaches);
    if (because.trimmed().isEmpty()) because = QString::fromStdString(screen->intro);
    card->body()->addWidget(chrome::bodyText(because, theme::textSecondary(), 14));

    card->body()->addWidget(chrome::label(
        QString("%1 of %2 fields answered  ·  %3")
            .arg(progress.answered)
            .arg(progress.fields)
            .arg(QString::fromStdString(screen->phase)),
        10, theme::textFaint()));

    auto* row = new QHBoxLayout;
    row->setSpacing(9);
    auto* walk = chrome::button("walk me through it", "primary", card);
    walk->setMinimumWidth(190);
    const QString screenId = QString::fromStdString(screen->id);
    connect(walk, &QPushButton::clicked, this, [this, screenId] { emit walkThrough(screenId); });
    auto* open = chrome::button("just open the screen", QString(), card);
    connect(open, &QPushButton::clicked, this, [this, screenId] { emit openScreen(screenId); });
    row->addWidget(walk);
    row->addWidget(open);
    row->addStretch(1);
    card->body()->addLayout(row);
    return card;
}

QWidget* Dashboard::buildStats() {
    Counts total;
    for (const pm::Screen& screen : definitions_.screens()) {
        const Counts counts = countScreen(screen, *record_);
        total.fieldsTotal += counts.fieldsTotal;
        total.fieldsFilled += counts.fieldsFilled;
        total.rows += counts.rows;
        total.specified += counts.specified;
        total.agreed += counts.agreed;
        total.unobjected += counts.unobjected;
        total.untagged += counts.untagged;
    }

    auto* holder = new QWidget(this);
    auto* grid = new QGridLayout(holder);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(12);

    const int percent = total.fieldsTotal == 0
                            ? 0
                            : total.fieldsFilled * 100 / total.fieldsTotal;
    const int decisions = total.specified + total.agreed;

    auto* decisionCard = statCard(QString::number(decisions), "decisions on evidence",
                                  theme::ok(), holder);
    decisionCard->setToolTip("Entries tagged specified or agreed. Only these count as decisions.");

    auto* looseCard = statCard(QString::number(total.unobjected), "proposed, unanswered",
                               total.unobjected > 0 ? theme::warn() : theme::textFaint(), holder);
    looseCard->setToolTip("Tagged unobjected: proposed and never answered. Not decisions. "
                          "Chase them or drop them.");

    // Four columns at full width, folding to two when the window narrows.
    QList<chrome::Card*> cards{
        statCard(QString("%1%").arg(percent), "record complete", theme::accent(), holder),
        statCard(QString::number(total.rows), "logged entries", theme::textPrimary(), holder),
        decisionCard,
        looseCard,
    };
    // Four across. They have a floor of 150px and the page is never
    // narrower than that times four, so this does not need measuring.
    const int columns = 4;
    for (int i = 0; i < cards.size(); ++i) {
        grid->addWidget(cards[i], i / columns, i % columns);
        grid->setColumnStretch(i % columns, 1);
    }
    return holder;
}

QWidget* Dashboard::buildPhases() {
    auto* card = new chrome::Card(this);
    card->body()->setSpacing(13);
    card->body()->addWidget(chrome::label("lifecycle", 11, theme::textLabel()));

    QString lastPhase;
    for (const pm::Screen& screen : definitions_.screens()) {
        const Counts counts = countScreen(screen, *record_);
        const qreal fraction = counts.fieldsTotal == 0
                                   ? 0.0
                                   : counts.fieldsFilled / static_cast<qreal>(counts.fieldsTotal);

        if (QString::fromStdString(screen.phase) != lastPhase) {
            lastPhase = QString::fromStdString(screen.phase);
            auto* phase = chrome::label(lastPhase, 10, theme::accent());
            card->body()->addSpacing(4);
            card->body()->addWidget(phase);
        }

        auto* line = new QWidget(card);
        auto* layout = new QHBoxLayout(line);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(12);

        auto* open = new QPushButton(QString::fromStdString(screen.title), line);
        open->setProperty("house", "quiet");
        open->setCursor(Qt::PointingHandCursor);
        open->setFont(theme::bodyFont(13));
        open->setMinimumWidth(120);
        open->setMaximumWidth(210);
        open->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        open->setStyleSheet("text-align: left;");
        const QString screenId = QString::fromStdString(screen.id);
        connect(open, &QPushButton::clicked, this, [this, screenId] { emit openScreen(screenId); });

        auto* meter = new Meter(fraction, line);
        auto* count = chrome::label(QString("%1 / %2").arg(counts.fieldsFilled).arg(counts.fieldsTotal),
                                    10, fraction >= 1.0 ? theme::ok() : theme::textFaint());
        count->setMinimumWidth(56);
        count->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        layout->addWidget(open);
        layout->addWidget(meter, 1);
        layout->addWidget(count);
        card->body()->addWidget(line);
    }
    return card;
}

QWidget* Dashboard::buildGaps() {
    auto* card = new chrome::Card(this);
    card->body()->setSpacing(11);
    card->body()->addWidget(chrome::label("what is missing, and what it costs", 11, theme::danger()));
    card->body()->addWidget(chrome::bodyText(
        "Every field carries the sentence explaining what goes wrong when nobody fills it "
        "in. These are the ones still blank.", theme::textFaint(), 12));

    int shown = 0;
    for (const pm::Screen& screen : definitions_.screens()) {
        for (const pm::Field& field : screen.fields) {
            const pm::Entry* entry = record_->entry(screen.id + "." + field.id);
            if (entryHasContent(entry)) continue;
            const QString consequence = QString::fromStdString(field.ifBlank).trimmed();
            if (consequence.isEmpty()) continue;
            if (++shown > 8) break;

            auto* line = new QWidget(card);
            auto* layout = new QVBoxLayout(line);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(3);

            auto* head = new QHBoxLayout;
            head->setSpacing(8);
            auto* where = new chrome::Badge(QString::fromStdString(screen.title), line);
            where->setColour(theme::textFaint());
            head->addWidget(where);
            head->addWidget(chrome::label(QString::fromStdString(field.label), 10,
                                          theme::textSecondary()));
            head->addStretch(1);
            layout->addLayout(head);
            layout->addWidget(chrome::bodyText(consequence, theme::textFaint(), 12));
            card->body()->addWidget(line);
        }
        if (shown > 8) break;
    }

    if (shown == 0) {
        card->body()->addWidget(chrome::bodyText(
            "Nothing is blank. Every field in every screen has an answer.", theme::ok(), 13));
    } else if (shown > 8) {
        card->body()->addWidget(chrome::bodyText("Showing the first eight.", theme::textFaint(), 12));
    }
    return card;
}
