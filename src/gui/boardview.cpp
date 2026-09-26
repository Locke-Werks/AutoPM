#include "boardview.h"

#include "fieldeditors.h"
#include "theme.h"

#include <algorithm>

#include <QAbstractItemView>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDropEvent>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QScrollArea>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QVBoxLayout>

namespace {

constexpr int kRowIdRole = card::RowId;
constexpr int kTitleRole = card::Title;
constexpr int kMetaRole = card::Meta;
constexpr int kToneRole = card::Tone;
constexpr int kBlockedRole = card::Blocked;
constexpr int kPointsRole = card::Points;

// "Doing=3, Review=2" from the definition file.
QHash<QString, int> parseWip(const std::string& spec) {
    QHash<QString, int> limits;
    const QStringList parts = QString::fromStdString(spec).split(',', Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        const QStringList pair = part.split('=');
        if (pair.size() != 2) continue;
        limits.insert(pair[0].trimmed(), pair[1].trimmed().toInt());
    }
    return limits;
}

pm::Tone toneFor(const pm::Column* column, const QString& value) {
    if (!column) return pm::Tone::Neutral;
    for (const pm::Option& option : column->options)
        if (QString::fromStdString(option.value) == value) return option.tone;
    return pm::Tone::Neutral;
}

// The card itself: an accent stripe carrying the priority, the title, and a
// quiet line of metadata underneath.
class CardDelegate : public QStyledItemDelegate {
public:
    explicit CardDelegate(QAbstractItemView* view) : QStyledItemDelegate(view), view_(view) {}

    // Width comes from the viewport, not from option.rect: during the first
    // layout pass option.rect is not the column width yet, and a card sized
    // from it clips its own title.
    int textWidth() const {
        return qMax(110, view_->viewport()->width() - 48);
    }

    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex& index) const override {
        const QString title = index.data(kTitleRole).toString();
        QFontMetrics metrics(theme::bodyFont(13));
        const QRect bounds = metrics.boundingRect(QRect(0, 0, textWidth(), 1000),
                                                  Qt::TextWordWrap, title);
        const bool hasMeta = !index.data(kMetaRole).toString().isEmpty();
        return QSize(view_->viewport()->width(),
                     qMax(58, bounds.height() + (hasMeta ? 42 : 26)));
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        QRectF box = QRectF(option.rect).adjusted(5.5, 3.5, -5.5, -4.5);
        const bool selected = option.state & QStyle::State_Selected;
        const bool hovered = option.state & QStyle::State_MouseOver;
        const bool blocked = index.data(kBlockedRole).toBool();

        QLinearGradient fill(box.topLeft(), box.bottomLeft());
        fill.setColorAt(0, theme::raised());
        fill.setColorAt(1, theme::voidBg());
        painter->setPen(Qt::NoPen);
        painter->setBrush(fill);
        painter->drawRoundedRect(box, 7, 7);

        QColor edge = theme::hairline();
        if (blocked) edge = QColor(theme::danger().red(), theme::danger().green(),
                                   theme::danger().blue(), 150);
        else if (selected) edge = theme::accentAt(150);
        else if (hovered) edge = theme::accentAt(85);
        painter->setPen(QPen(edge, 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(box, 7, 7);

        // Priority stripe down the left edge.
        const QColor stripe = index.data(kToneRole).value<QColor>();
        if (stripe.isValid()) {
            QPainterPath path;
            path.addRoundedRect(QRectF(box.left(), box.top() + 1, 3, box.height() - 2), 1.5, 1.5);
            painter->fillPath(path, stripe);
        }

        const int left = static_cast<int>(box.left()) + 13;
        const int right = static_cast<int>(box.right()) - 10;

        // The points pill is placed first so the title can be given the space
        // that is actually left over.
        const QString points = index.data(kPointsRole).toString();
        int titleRight = right;
        if (!points.isEmpty()) {
            const QFont pointsFont = theme::monoFont(10);
            const QFontMetrics metrics(pointsFont);
            const int pillWidth = metrics.horizontalAdvance(points) + 12;
            const QRect pill(right - pillWidth, static_cast<int>(box.top()) + 9, pillWidth, 17);
            painter->setFont(pointsFont);
            painter->setPen(QPen(theme::accentAt(90), 1));
            painter->setBrush(theme::accentAt(22));
            painter->drawRoundedRect(QRectF(pill), 2, 2);
            painter->setPen(theme::accent());
            painter->drawText(pill, Qt::AlignCenter, points);
            titleRight = pill.left() - 9;
        }

        painter->setPen(theme::textPrimary());
        painter->setFont(theme::bodyFont(13));
        const QRect titleRect(left, static_cast<int>(box.top()) + 8, titleRight - left, 1000);
        QRect used;
        painter->drawText(titleRect, Qt::TextWordWrap | Qt::AlignTop,
                          index.data(kTitleRole).toString(), &used);

        const QString meta = index.data(kMetaRole).toString();
        if (!meta.isEmpty()) {
            painter->setPen(theme::textFaint());
            painter->setFont(theme::labelFont(9, false));
            painter->drawText(QRect(left, used.bottom() + 6, right - left, 16),
                              Qt::AlignLeft | Qt::AlignVCenter,
                              painter->fontMetrics().elidedText(meta, Qt::ElideRight, right - left));
        }

        painter->restore();
    }

private:
    QAbstractItemView* view_ = nullptr;
};

} // namespace

// ── BoardColumn ──────────────────────────────────────────────────────────

BoardColumn::BoardColumn(QWidget* parent) : QListWidget(parent) {
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setMouseTracking(true);
    setSpacing(0);
    setUniformItemSizes(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setItemDelegate(new CardDelegate(this));
    // Cards are sized from the column width, so a resize has to re-measure them.
    connect(this, &QListWidget::itemChanged, this, [this] { doItemsLayout(); });
}

void BoardColumn::dropEvent(QDropEvent* event) {
    QListWidget::dropEvent(event);
    // The board rewrites every row from what is on screen, so it does not
    // matter which widget the item landed in or where it came from.
    emit dropped();
}

// ── BoardView ────────────────────────────────────────────────────────────

BoardView::BoardView(const pm::Field& field, QWidget* parent)
    : RichView(parent), field_(field) {
    laneColumn_ = field_.column(field_.groupBy);
    if (!laneColumn_ && !field_.columns.empty()) laneColumn_ = &field_.columns[0];

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    auto* toolbar = new QHBoxLayout;
    toolbar->setSpacing(10);

    auto* add = chrome::button("+ card", "primary", this);
    connect(add, &QPushButton::clicked, this, &BoardView::addCard);

    sprintFilter_ = new QComboBox(this);
    sprintFilter_->setMinimumWidth(150);
    connect(sprintFilter_, &QComboBox::currentIndexChanged, this, [this](int) {
        sprintShown_ = sprintFilter_->currentData().toString();
        rebuild();
    });

    summary_ = chrome::label("", 10, theme::textFaint());

    toolbar->addWidget(add);
    if (field_.column("sprint")) {
        toolbar->addWidget(chrome::label("sprint", 10, theme::textLabel()));
        toolbar->addWidget(sprintFilter_);
    } else {
        sprintFilter_->hide();
    }
    toolbar->addStretch(1);
    toolbar->addWidget(summary_);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* strip = new QWidget;
    columnRow_ = new QHBoxLayout(strip);
    columnRow_->setContentsMargins(0, 0, 0, 0);
    columnRow_->setSpacing(12);
    scroll->setWidget(strip);
    scroll->setMinimumHeight(430);

    layout->addLayout(toolbar);
    layout->addWidget(scroll, 1);

    // One column per allowed value of the grouping column.
    const QHash<QString, int> limits = parseWip(laneColumn_ ? laneColumn_->wip : std::string());
    if (laneColumn_) {
        for (const pm::Option& option : laneColumn_->options) {
            const QString name = QString::fromStdString(option.value);

            auto* panel = new QFrame(strip);
            panel->setMinimumWidth(206);
            panel->setMaximumWidth(300);
            panel->setStyleSheet(QString("QFrame { background: %1; border: 1px solid %2;"
                                         " border-radius: 10px; }")
                                     .arg(theme::surface().name())
                                     .arg(chrome::hairlineCss()));
            auto* panelLayout = new QVBoxLayout(panel);
            panelLayout->setContentsMargins(9, 12, 9, 10);
            panelLayout->setSpacing(8);

            auto* headerRow = new QHBoxLayout;
            auto* name_ = chrome::label(name, 10, theme::toneColour(option.tone));
            auto* count = chrome::label("0", 10, theme::textFaint());
            if (limits.contains(name)) {
                count->setToolTip(QString("Work in progress limit: %1. Finishing beats starting.")
                                      .arg(limits.value(name)));
            }
            headerRow->addWidget(name_);
            headerRow->addStretch(1);
            headerRow->addWidget(count);

            auto* list = new BoardColumn(panel);
            list->setProperty("lane", name);
            connect(list, &BoardColumn::dropped, this, [this] {
                harvest();
                refreshHeaders();
                emit rowsChanged(rows_);
            });
            connect(list, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
                openCard(item->data(kRowIdRole).toString());
            });

            panelLayout->addLayout(headerRow);
            panelLayout->addWidget(list, 1);

            columnRow_->addWidget(panel);
            lists_.push_back(list);
            counts_.push_back(count);
        }
    }
    columnRow_->addStretch(1);
}

QStringList BoardView::sprints() const {
    QStringList found;
    for (const pm::Row& row : rows_) {
        const QString sprint = QString::fromStdString(row.cell("sprint"));
        if (!sprint.isEmpty() && !found.contains(sprint)) found << sprint;
    }
    found.sort();
    return found;
}

void BoardView::setRows(const std::vector<pm::Row>& rows) {
    rows_ = rows;

    if (sprintFilter_->isVisible() || field_.column("sprint")) {
        const QString current = sprintShown_;
        QSignalBlocker block(sprintFilter_);
        sprintFilter_->clear();
        sprintFilter_->addItem("All sprints", QString());
        for (const QString& sprint : sprints()) sprintFilter_->addItem(sprint, sprint);
        const int at = sprintFilter_->findData(current);
        sprintFilter_->setCurrentIndex(at < 0 ? 0 : at);
        sprintShown_ = sprintFilter_->currentData().toString();
    }
    rebuild();
}

void BoardView::rebuild() {
    for (BoardColumn* list : lists_) {
        QSignalBlocker block(list);
        list->clear();
    }
    if (!laneColumn_) return;

    const pm::Column* priority = field_.column(field_.toneColumn);
    const pm::Column* title = field_.column(field_.titleColumn);
    const std::string titleId = title ? title->id : (field_.columns.empty() ? "" : field_.columns[0].id);

    for (const pm::Row& row : rows_) {
        if (!sprintShown_.isEmpty() &&
            QString::fromStdString(row.cell("sprint")) != sprintShown_)
            continue;

        const QString lane = QString::fromStdString(row.cell(laneColumn_->id));
        int target = 0;
        for (size_t i = 0; i < laneColumn_->options.size(); ++i) {
            if (QString::fromStdString(laneColumn_->options[i].value) == lane) {
                target = static_cast<int>(i);
                break;
            }
        }
        if (target >= static_cast<int>(lists_.size())) continue;

        auto* item = new QListWidgetItem;
        item->setData(kRowIdRole, QString::fromStdString(row.id));
        item->setData(kTitleRole, QString::fromStdString(row.cell(titleId)));

        QStringList meta;
        const QString ref = QString::fromStdString(row.cell("ref"));
        if (!ref.isEmpty()) meta << ref;
        const QString owner = QString::fromStdString(row.cell(field_.subtitleColumn));
        if (!owner.isEmpty()) meta << owner;
        const QString sprint = QString::fromStdString(row.cell("sprint"));
        if (!sprint.isEmpty() && sprintShown_.isEmpty()) meta << sprint;
        if (row.cell("blocked") == "Yes") meta << "BLOCKED";
        item->setData(kMetaRole, meta.join("  ·  "));

        item->setData(kBlockedRole, row.cell("blocked") == "Yes");
        const QString points = QString::fromStdString(row.cell("points"));
        item->setData(kPointsRole, points.isEmpty() ? QString() : points + " pt");
        item->setData(kToneRole, theme::toneColour(
            toneFor(priority, QString::fromStdString(row.cell(field_.toneColumn)))));
        item->setFlags(item->flags() | Qt::ItemIsDragEnabled);

        QSignalBlocker block(lists_[target]);
        lists_[target]->addItem(item);
    }
    refreshHeaders();
}

void BoardView::harvest() {
    if (!laneColumn_) return;
    // Walk the columns in order and write each card's lane back. Order on
    // screen becomes order in the record, so a reorder is a real edit.
    std::vector<pm::Row> reordered;
    std::vector<bool> taken(rows_.size(), false);

    for (size_t columnIndex = 0; columnIndex < lists_.size(); ++columnIndex) {
        const QString lane = QString::fromStdString(laneColumn_->options[columnIndex].value);
        BoardColumn* list = lists_[columnIndex];
        for (int i = 0; i < list->count(); ++i) {
            const QString rowId = list->item(i)->data(kRowIdRole).toString();
            for (size_t r = 0; r < rows_.size(); ++r) {
                if (taken[r] || QString::fromStdString(rows_[r].id) != rowId) continue;
                pm::Row copy = rows_[r];
                copy.setCell(laneColumn_->id, lane.toStdString());
                reordered.push_back(copy);
                taken[r] = true;
                break;
            }
        }
    }
    // Anything filtered out of view keeps its place rather than disappearing.
    for (size_t r = 0; r < rows_.size(); ++r)
        if (!taken[r]) reordered.push_back(rows_[r]);

    rows_ = reordered;
}

void BoardView::refreshHeaders() {
    const QHash<QString, int> limits = parseWip(laneColumn_ ? laneColumn_->wip : std::string());
    int totalPoints = 0;
    int doneOnBoard = 0;

    for (size_t i = 0; i < lists_.size(); ++i) {
        const int count = lists_[i]->count();
        const QString lane = QString::fromStdString(laneColumn_->options[i].value);
        QString text = QString::number(count);
        QColor colour = theme::textFaint();
        if (limits.contains(lane)) {
            const int limit = limits.value(lane);
            text = QString("%1 / %2").arg(count).arg(limit);
            // Over the limit is the one thing on a board worth shouting about.
            if (count > limit) colour = theme::danger();
        }
        counts_[i]->setText(text);
        QPalette palette = counts_[i]->palette();
        palette.setColor(QPalette::WindowText, colour);
        counts_[i]->setPalette(palette);
    }

    for (const pm::Row& row : rows_) {
        const int points = QString::fromStdString(row.cell("points")).toInt();
        totalPoints += points;
        if (!laneColumn_->options.empty() &&
            row.cell(laneColumn_->id) == laneColumn_->options.back().value)
            doneOnBoard += points;
    }
    if (totalPoints > 0)
        summary_->setText(QString("%1 of %2 points done").arg(doneOnBoard).arg(totalPoints));
    else
        summary_->setText(QString("%1 cards").arg(rows_.size()));
}

void BoardView::addCard() {
    pm::Row row;
    int highest = 0;
    for (const pm::Row& existing : rows_) {
        const QString ref = QString::fromStdString(existing.cell("ref"));
        if (ref.startsWith('C')) highest = qMax(highest, ref.mid(1).toInt());
    }
    row.id = "c" + std::to_string(rows_.size() + 1);
    row.setCell("ref", ("C" + std::to_string(highest + 1)));
    if (laneColumn_ && !laneColumn_->options.empty())
        row.setCell(laneColumn_->id, laneColumn_->options.front().value);
    if (!sprintShown_.isEmpty()) row.setCell("sprint", sprintShown_.toStdString());

    CardDialog dialog(field_, row, this);
    if (dialog.exec() != QDialog::Accepted) return;
    rows_.push_back(dialog.row());
    setRows(rows_);
    emit rowsChanged(rows_);
}

void BoardView::openCard(const QString& rowId) {
    for (size_t i = 0; i < rows_.size(); ++i) {
        if (QString::fromStdString(rows_[i].id) != rowId) continue;
        CardDialog dialog(field_, rows_[i], this);
        if (dialog.exec() != QDialog::Accepted) return;
        rows_[i] = dialog.row();
        setRows(rows_);
        emit rowsChanged(rows_);
        return;
    }
}

// ── CardDialog ───────────────────────────────────────────────────────────

CardDialog::CardDialog(const pm::Field& field, const pm::Row& row, QWidget* parent)
    : QDialog(parent), field_(field), row_(row) {
    setWindowTitle(QString::fromStdString(field.label));
    setMinimumWidth(560);
    setStyleSheet(parent ? parent->window()->styleSheet() : QString());

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(26, 24, 26, 22);
    layout->setSpacing(14);

    layout->addWidget(new chrome::Eyebrow(QString::fromStdString(field.label)));
    layout->addWidget(new chrome::Heading(field.view == "board" ? "Card detail" : "Edit entry", 22));

    // The fields scroll and the heading and buttons stay put. A row with a
    // dozen columns is taller than a laptop screen, and a dialog that runs off
    // the bottom takes its Save button with it.
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* formHolder = new QWidget(scroll);
    formHolder->setAutoFillBackground(false);
    auto* form = new QFormLayout(formHolder);
    form->setContentsMargins(0, 0, 10, 0);
    form->setSpacing(11);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    for (const pm::Column& column : field_.columns) {
        QWidget* editor = editors::makeEditor(QString::fromStdString(column.type), column.options,
                                              QString(), this);
        if (auto* growing = qobject_cast<editors::GrowingTextEdit*>(editor))
            growing->setMinimumLines(2);
        editors::setEditorValue(editor, QString::fromStdString(row_.cell(column.id)));
        // Long labels wrap rather than push the fields past the right edge.
        auto* name = chrome::label(QString::fromStdString(column.label), 10);
        name->setWordWrap(true);
        name->setFixedWidth(130);
        form->addRow(name, editor);
        editors_.emplace_back(column.id, editor);
    }

    provenance_ = new chrome::ProvenanceControl(this);
    provenance_->set(row_.provenance, QString::fromStdString(row_.evidence));
    form->addRow(chrome::label("source", 10), provenance_);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    for (QAbstractButton* button : buttons->buttons()) {
        button->setCursor(Qt::PointingHandCursor);
        button->setFont(theme::labelFont(11));
    }
    buttons->button(QDialogButtonBox::Save)->setProperty("house", "primary");
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    scroll->setWidget(formHolder);
    layout->addWidget(scroll, 1);
    layout->addWidget(buttons);

    // As wide as the fields need and as tall, up to what the screen can show.
    // The text editors only know their heights once laid out at their real
    // width, so the height is measured again after the first show.
    const QRect available = (parent ? parent->screen() : screen())->availableGeometry();
    const int scrollbar = style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    const int wide = formHolder->sizeHint().width() + 26 * 2 + scrollbar;
    const auto fit = [this, scroll, formHolder, available] {
        const int outside = height() - scroll->viewport()->height();
        const int wanted = formHolder->sizeHint().height() + outside;
        resize(width(), std::min(wanted, available.height() * 9 / 10));
    };
    resize(std::max(minimumWidth(), wide), available.height() * 9 / 10);
    QTimer::singleShot(0, this, fit);
}

pm::Row CardDialog::row() const {
    pm::Row result = row_;
    for (const auto& pair : editors_) {
        const QString value = editors::editorValue(pair.second);
        if (value.isEmpty()) result.cells.erase(pair.first);
        else result.setCell(pair.first, value.toStdString());
    }
    result.provenance = provenance_->provenance();
    result.evidence = provenance_->evidence().toStdString();
    return result;
}
