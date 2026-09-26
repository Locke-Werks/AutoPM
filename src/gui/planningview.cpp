// Sprint planning: pulling work out of the backlog into an iteration, and
// finding out whether you have pulled too much.
//
// It moves the board's own cards rather than keeping a second list, because a
// sprint that does not contain the actual work is a spreadsheet.
#include "boardview.h"

#include "chrome.h"
#include "theme.h"

#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QSet>
#include <QVBoxLayout>

namespace {

// Committed against capacity. Going over is the whole point of the widget, so
// it is the one state that changes colour.
class CapacityMeter : public QWidget {
public:
    explicit CapacityMeter(QWidget* parent = nullptr) : QWidget(parent) {
        setFixedHeight(6);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    void set(int committed, int capacity) {
        committed_ = committed;
        capacity_ = capacity;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        painter.setBrush(theme::hairline());
        painter.drawRoundedRect(QRectF(0, 0, width(), height()), 3, 3);
        if (capacity_ <= 0 || committed_ <= 0) return;

        const qreal fraction = committed_ / static_cast<qreal>(capacity_);
        painter.setBrush(fraction > 1.0 ? theme::danger() : theme::accent());
        painter.drawRoundedRect(QRectF(0, 0, width() * qMin(1.0, fraction), height()), 3, 3);
    }

private:
    int committed_ = 0;
    int capacity_ = 0;
};

// Capacity against what actually got finished, one pair per closed sprint.
// This is what turns the next capacity number into a measurement.
class VelocityStrip : public QWidget {
public:
    struct Past {
        QString name;
        int planned = 0;
        int completed = 0;
    };

    explicit VelocityStrip(QWidget* parent = nullptr) : QWidget(parent) {
        setFixedHeight(82);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    void set(const QList<Past>& past) {
        past_ = past;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        if (past_.isEmpty()) {
            painter.setPen(theme::textFaint());
            painter.setFont(theme::bodyFont(12));
            painter.drawText(rect(), Qt::AlignLeft | Qt::AlignVCenter,
                             "No closed sprints yet. Velocity appears once one is done, and until "
                             "then capacity is a guess.");
            return;
        }

        int ceiling = 1;
        for (const Past& p : past_) ceiling = qMax(ceiling, qMax(p.planned, p.completed));

        const int slot = qMin(110, width() / qMax(1, static_cast<int>(past_.size())));
        const int top = 6;
        const int floorY = height() - 20;

        for (int i = 0; i < past_.size(); ++i) {
            const Past& p = past_[i];
            const int x = i * slot;
            const int barWidth = qMax(9, slot / 4);

            const auto bar = [&](int value, int offset, const QColor& colour, bool solid) {
                const int h = static_cast<int>((floorY - top) * (value / static_cast<qreal>(ceiling)));
                QColor fill = colour;
                fill.setAlpha(solid ? 200 : 55);
                painter.setPen(QPen(colour, 1));
                painter.setBrush(fill);
                painter.drawRoundedRect(QRectF(x + offset, floorY - h, barWidth, h), 2, 2);
            };
            bar(p.planned, 2, theme::textLabel(), false);
            bar(p.completed, 4 + barWidth, theme::accent(), true);

            painter.setPen(theme::textFaint());
            painter.setFont(theme::labelFont(8));
            painter.drawText(QRect(x, floorY + 4, slot - 6, 14), Qt::AlignLeft | Qt::AlignVCenter,
                             painter.fontMetrics().elidedText(p.name, Qt::ElideRight, slot - 8));
        }
    }

private:
    QList<Past> past_;
};

// The card item the board's own delegate knows how to draw, so a card looks
// the same here as it does on the board.
QListWidgetItem* cardItem(const pm::Row& row, const std::string& titleId) {
    auto* item = new QListWidgetItem;
    item->setData(card::RowId, QString::fromStdString(row.id));
    item->setData(card::Title, QString::fromStdString(row.cell(titleId)));

    QStringList meta;
    const QString ref = QString::fromStdString(row.cell("ref"));
    if (!ref.isEmpty()) meta << ref;
    if (row.cell("ready") == "Needs refinement") meta << "NOT REFINED";
    if (row.cell("blocked") == "Yes") meta << "BLOCKED";
    item->setData(card::Meta, meta.join("  ·  "));

    item->setData(card::Blocked, row.cell("blocked") == "Yes");
    const QString points = QString::fromStdString(row.cell("points"));
    item->setData(card::Points, points.isEmpty() ? QString() : points + " pt");

    QColor tone = theme::textFaint();
    if (row.cell("ready") == "Refined") tone = theme::ok();
    else if (row.cell("ready") == "Needs refinement") tone = theme::warn();
    item->setData(card::Tone, tone);
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    return item;
}

QFrame* pile(QWidget* parent, const QString& title, BoardColumn** list, QLabel** count) {
    auto* panel = new QFrame(parent);
    panel->setStyleSheet(QString("QFrame { background: %1; border: 1px solid %2;"
                                 " border-radius: 10px; }")
                             .arg(theme::surface().name(), chrome::hairlineCss()));
    auto* inner = new QVBoxLayout(panel);
    inner->setContentsMargins(9, 12, 9, 10);
    inner->setSpacing(8);

    auto* head = new QHBoxLayout;
    head->addWidget(chrome::label(title, 10, theme::textLabel()));
    head->addStretch(1);
    *count = chrome::label("0", 10, theme::textFaint());
    head->addWidget(*count);
    inner->addLayout(head);

    *list = new BoardColumn(panel);
    (*list)->setMinimumHeight(280);
    inner->addWidget(*list, 1);
    return panel;
}

} // namespace

PlanningView::PlanningView(const pm::Field& field, QWidget* parent)
    : RichView(parent), field_(field) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    auto* top = new QHBoxLayout;
    top->setSpacing(10);
    top->addWidget(chrome::label("sprint", 10, theme::textLabel()));
    picker_ = new QComboBox(this);
    picker_->setMinimumWidth(180);
    connect(picker_, &QComboBox::currentIndexChanged, this, [this](int) {
        showing_ = picker_->currentData().toString();
        rebuild();
    });
    top->addWidget(picker_);
    goal_ = chrome::bodyText(QString(), theme::textSecondary(), 13);
    top->addWidget(goal_, 1);
    layout->addLayout(top);

    auto* capacityRow = new QHBoxLayout;
    capacityRow->setSpacing(12);
    committed_ = chrome::label(QString(), 10, theme::textFaint());
    committed_->setMinimumWidth(200);
    meter_ = new CapacityMeter(this);
    capacityRow->addWidget(committed_);
    capacityRow->addWidget(meter_, 1);
    layout->addLayout(capacityRow);

    advice_ = chrome::bodyText(QString(), theme::warn(), 12);
    advice_->setVisible(false);
    layout->addWidget(advice_);

    auto* piles = new QHBoxLayout;
    piles->setSpacing(12);
    piles->addWidget(pile(this, "the backlog", &backlog_, &backlogCount_), 1);
    piles->addWidget(pile(this, "in this sprint", &sprint_, &sprintCount_), 1);
    layout->addLayout(piles, 1);

    for (BoardColumn* list : {backlog_, sprint_}) {
        connect(list, &BoardColumn::dropped, this, [this] {
            harvest();
            rebuild();
            emit recordChanged();
        });
        connect(list, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
            openCard(item->data(card::RowId).toString());
        });
    }

    layout->addWidget(chrome::label("velocity: capacity against what got finished", 10,
                                    theme::textLabel()));
    velocity_ = new VelocityStrip(this);
    layout->addWidget(velocity_);
}

void PlanningView::setRecord(const std::shared_ptr<pm::Record>& record) {
    record_ = record;
    rebuild();
}

std::vector<pm::Row>* PlanningView::cards() {
    if (!record_ || field_.reads.empty()) return nullptr;
    return &record_->mutableEntry(field_.reads).rows;
}

const pm::Row* PlanningView::currentSprint() const {
    for (const pm::Row& sprint : sprints_)
        if (QString::fromStdString(sprint.cell("name")) == showing_) return &sprint;
    return nullptr;
}

void PlanningView::setRows(const std::vector<pm::Row>& rows) {
    sprints_ = rows;

    const QString keep = showing_;
    {
        QSignalBlocker block(picker_);
        picker_->clear();
        for (const pm::Row& sprint : sprints_) {
            const QString name = QString::fromStdString(sprint.cell("name"));
            if (!name.isEmpty()) picker_->addItem(name, name);
        }
        // Land on the sprint in flight rather than the first one ever run.
        int at = picker_->findData(keep);
        if (at < 0) {
            for (const pm::Row& sprint : sprints_) {
                if (sprint.cell("status") != "Active") continue;
                at = picker_->findData(QString::fromStdString(sprint.cell("name")));
                break;
            }
        }
        picker_->setCurrentIndex(qMax(0, at));
        showing_ = picker_->currentData().toString();
    }
    rebuild();
}

void PlanningView::rebuild() {
    if (!backlog_ || !sprint_) return;
    for (BoardColumn* list : {backlog_, sprint_}) {
        QSignalBlocker block(list);
        list->clear();
    }

    const std::vector<pm::Row>* rows = cards();
    if (rows) {
        for (const pm::Row& row : *rows) {
            const QString sprint = QString::fromStdString(row.cell("sprint"));
            // Finished work is not pullable and belongs in neither pile.
            const bool done = row.cell("state") == "Done";

            if (!showing_.isEmpty() && sprint == showing_) {
                QSignalBlocker block(sprint_);
                sprint_->addItem(cardItem(row, titleColumn()));
            } else if (sprint.isEmpty() && !done) {
                QSignalBlocker block(backlog_);
                backlog_->addItem(cardItem(row, titleColumn()));
            }
        }
    }
    refreshTotals();
}

std::string PlanningView::titleColumn() const {
    return field_.titleColumn.empty() ? std::string("title") : field_.titleColumn;
}

void PlanningView::harvest() {
    std::vector<pm::Row>* rows = cards();
    if (!rows || !record_) return;

    // Whatever sits on the right belongs to this sprint; whatever sits on the
    // left belongs to nobody yet. Cards in neither pile are left alone.
    const auto idsIn = [](BoardColumn* list) {
        QSet<QString> ids;
        for (int i = 0; i < list->count(); ++i)
            ids.insert(list->item(i)->data(card::RowId).toString());
        return ids;
    };
    const QSet<QString> inSprint = idsIn(sprint_);
    const QSet<QString> inBacklog = idsIn(backlog_);

    for (pm::Row& row : *rows) {
        const QString id = QString::fromStdString(row.id);
        if (inSprint.contains(id)) {
            row.setCell("sprint", showing_.toStdString());
            // Pulling a card in makes it ready to start rather than a wish.
            if (row.cell("state").empty() || row.cell("state") == "Backlog")
                row.setCell("state", "Ready");
        } else if (inBacklog.contains(id)) {
            row.cells.erase("sprint");
            row.setCell("state", "Backlog");
        }
    }
    record_->markDirty();
}

void PlanningView::openCard(const QString& rowId) {
    std::vector<pm::Row>* rows = cards();
    if (!rows) return;
    for (pm::Row& row : *rows) {
        if (QString::fromStdString(row.id) != rowId) continue;
        // The card's own definition lives on the board screen; this view only
        // knows the columns through the rows it was handed, so edit with the
        // field it reads from if we have it.
        if (!cardField_) return;
        CardDialog dialog(*cardField_, row, this);
        if (dialog.exec() != QDialog::Accepted) return;
        row = dialog.row();
        record_->markDirty();
        rebuild();
        emit recordChanged();
        return;
    }
}

void PlanningView::setCardField(const pm::Field* field) { cardField_ = field; }

int PlanningView::pointsIn(const QString& sprint, bool doneOnly) const {
    if (!record_ || field_.reads.empty()) return 0;
    const std::vector<pm::Row>* rows = record_->rows(field_.reads);
    if (!rows) return 0;
    int total = 0;
    for (const pm::Row& row : *rows) {
        if (QString::fromStdString(row.cell("sprint")) != sprint) continue;
        if (doneOnly && row.cell("state") != "Done") continue;
        total += QString::fromStdString(row.cell("points")).toInt();
    }
    return total;
}

void PlanningView::refreshTotals() {
    backlogCount_->setText(QString("%1 waiting").arg(backlog_->count()));
    sprintCount_->setText(QString("%1 pulled in").arg(sprint_->count()));

    const pm::Row* sprint = currentSprint();
    const int capacity = sprint ? QString::fromStdString(sprint->cell("capacity")).toInt() : 0;

    int committed = 0;
    int unrefined = 0;
    int unestimated = 0;
    for (int i = 0; i < sprint_->count(); ++i) {
        const QString points = sprint_->item(i)->data(card::Points).toString();
        if (points.isEmpty()) ++unestimated;
        committed += points.split(' ').first().toInt();
        if (sprint_->item(i)->data(card::Meta).toString().contains("NOT REFINED")) ++unrefined;
    }

    goal_->setText(sprint ? QString::fromStdString(sprint->cell("goal")) : QString());
    committed_->setText(capacity > 0
                            ? QString("%1 of %2 points committed").arg(committed).arg(capacity)
                            : QString("%1 points, no capacity set").arg(committed));
    static_cast<CapacityMeter*>(meter_)->set(committed, capacity);

    // The advice is the teaching: say what is wrong with this commitment, in
    // the order somebody would notice it.
    QStringList notes;
    if (capacity <= 0) {
        notes << "This sprint has no capacity, so there is nothing to be over. Capacity is how "
                 "many points you can actually finish, and the velocity below is where the "
                 "number comes from.";
    } else if (committed > capacity) {
        notes << QString("Committed %1 points against a capacity of %2. Something here will not "
                         "get finished. Deciding which, now, is the job; finding out on the last "
                         "day is not.")
                     .arg(committed)
                     .arg(capacity);
    }
    if (unestimated > 0) {
        notes << (unestimated == 1
                      ? QString("One card has no estimate. An unestimated card makes the whole "
                                "commitment unmeasurable.")
                      : QString("%1 cards have no estimate. An unestimated card makes the whole "
                                "commitment unmeasurable.").arg(unestimated));
    }
    if (unrefined > 0) {
        notes << (unrefined == 1
                      ? QString("One card pulled in is not refined yet. Work that is not ready to "
                                "start will spend the sprint being understood instead.")
                      : QString("%1 cards pulled in are not refined yet. Work that is not ready to "
                                "start will spend the sprint being understood instead.")
                            .arg(unrefined));
    }
    advice_->setText(notes.join("\n"));
    advice_->setVisible(!notes.isEmpty());

    QList<VelocityStrip::Past> past;
    for (const pm::Row& row : sprints_) {
        if (row.cell("status") != "Done") continue;
        const QString name = QString::fromStdString(row.cell("name"));
        past << VelocityStrip::Past{name, QString::fromStdString(row.cell("capacity")).toInt(),
                                    pointsIn(name, true)};
    }
    static_cast<VelocityStrip*>(velocity_)->set(past);
}
