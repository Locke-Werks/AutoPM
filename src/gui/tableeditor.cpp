#include "tableeditor.h"

#include "chrome.h"
#include "fieldeditors.h"
#include "theme.h"

#include <QCalendarWidget>
#include <QComboBox>
#include <QEvent>
#include <QTextCharFormat>
#include <QToolButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {

// A date cell is blank until somebody fills it in, which QDateEdit has no
// concept of. The convention is to park the value on the minimum and show
// special text there, and that is what this does. What it missed is that the
// minimum is also where the calendar opens and where stepping starts, so an
// untouched cell offered January 1900 and one press of Up recorded 1901.
//
// Blank still reads as blank and still saves as nothing. What changes is that
// the moment it stops being blank, it becomes today.
class DateCell : public QDateEdit {
public:
    explicit DateCell(QWidget* parent) : QDateEdit(parent) {
        setDisplayFormat("yyyy-MM-dd");
        setCalendarPopup(true);
        setMinimumDate(QDate(1900, 1, 1));
        setSpecialValueText(" — ");
        if (QCalendarWidget* calendar = calendarWidget()) {
            // Left alone the popup takes the width of the cell it was opened
            // from, which is narrow enough that Qt elides every day name to
            // "...", and a calendar whose columns are unlabelled is not one.
            calendar->setMinimumWidth(300);
            calendar->setFont(theme::bodyFont(12));
            calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
            calendar->setGridVisible(false);
            // Qt paints Saturday and Sunday red. Red means one thing in this
            // app and a weekend is not it.
            QTextCharFormat weekday;
            weekday.setForeground(theme::textBody());
            calendar->setWeekdayTextFormat(Qt::Saturday, weekday);
            calendar->setWeekdayTextFormat(Qt::Sunday, weekday);
            // The month arrows arrive as Qt's own icons, which are green.
            for (const auto& [name, glyph] : {std::pair<const char*, const char*>{"qt_calendar_prevmonth", "‹"},
                                             std::pair<const char*, const char*>{"qt_calendar_nextmonth", "›"}}) {
                if (auto* button = calendar->findChild<QToolButton*>(name)) {
                    button->setIcon(QIcon());
                    button->setToolButtonStyle(Qt::ToolButtonTextOnly);
                    button->setText(QString::fromUtf8(glyph));
                }
            }
            calendar->installEventFilter(this);
        }
    }

    bool blank() const { return date() == minimumDate(); }

protected:
    void stepBy(int steps) override {
        if (blank()) {
            setDate(QDate::currentDate());
            return;
        }
        QDateEdit::stepBy(steps);
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
        // Qt points the calendar at the edit's own date just before showing
        // it, which on a blank cell is 1900, so this has to happen after that
        // rather than in the constructor. Only the page moves: opening the
        // picker and thinking better of it still leaves the cell empty.
        if (event->type() == QEvent::Show && blank()) {
            if (auto* calendar = qobject_cast<QCalendarWidget*>(watched)) {
                const QDate today = QDate::currentDate();
                calendar->setCurrentPage(today.year(), today.month());
            }
        }
        return QDateEdit::eventFilter(watched, event);
    }
};

std::vector<pm::Option> provenanceOptions() {
    return {
        {"specified", pm::Tone::Ok},
        {"agreed", pm::Tone::Accent},
        {"unobjected", pm::Tone::Warn},
    };
}

pm::Tone toneForValue(const pm::Column& column, const QString& value) {
    for (const pm::Option& option : column.options) {
        if (QString::fromStdString(option.value) == value) return option.tone;
    }
    return pm::Tone::Neutral;
}

} // namespace

int provenanceColumnIndex(const pm::Field& field) {
    return static_cast<int>(field.columns.size());
}

int evidenceColumnIndex(const pm::Field& field) {
    return static_cast<int>(field.columns.size()) + 1;
}

// ── CellDelegate ─────────────────────────────────────────────────────────

CellDelegate::CellDelegate(const pm::Field* field, QObject* parent)
    : QStyledItemDelegate(parent), field_(field) {}

const pm::Column* CellDelegate::columnAt(int visualIndex) const {
    if (!field_) return nullptr;
    if (visualIndex < 0 || visualIndex >= static_cast<int>(field_->columns.size())) return nullptr;
    return &field_->columns[visualIndex];
}

QWidget* CellDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const {
    const int column = index.column();
    if (field_ && column == provenanceColumnIndex(*field_)) {
        auto* combo = new QComboBox(parent);
        editors::fillChoices(combo, provenanceOptions());
        return combo;
    }
    const pm::Column* definition = columnAt(column);
    if (!definition) return QStyledItemDelegate::createEditor(parent, option, index);

    if (definition->type == "choice") {
        auto* combo = new QComboBox(parent);
        editors::fillChoices(combo, definition->options);
        return combo;
    }
    if (definition->type == "date") return new DateCell(parent);
    if (definition->type == "number") {
        auto* spin = new QSpinBox(parent);
        spin->setRange(0, 9999);
        spin->setSpecialValueText(" — ");
        return spin;
    }
    auto* line = new QLineEdit(parent);
    line->setFont(theme::bodyFont(13));
    return line;
}

void CellDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    const QString value = index.data(Qt::EditRole).toString();
    if (auto* combo = qobject_cast<QComboBox*>(editor)) {
        int at = combo->findData(value);
        if (at < 0 && !value.isEmpty()) {
            combo->addItem(value + "  (not in the definition)", value);
            at = combo->count() - 1;
        }
        combo->setCurrentIndex(qMax(0, at));
        return;
    }
    if (auto* date = qobject_cast<QDateEdit*>(editor)) {
        const QDate parsed = QDate::fromString(value, "yyyy-MM-dd");
        date->setDate(parsed.isValid() ? parsed : date->minimumDate());
        return;
    }
    if (auto* spin = qobject_cast<QSpinBox*>(editor)) { spin->setValue(value.toInt()); return; }
    if (auto* line = qobject_cast<QLineEdit*>(editor)) { line->setText(value); return; }
    QStyledItemDelegate::setEditorData(editor, index);
}

void CellDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                const QModelIndex& index) const {
    if (auto* combo = qobject_cast<QComboBox*>(editor)) {
        model->setData(index, combo->currentData().toString(), Qt::EditRole);
        return;
    }
    if (auto* date = qobject_cast<QDateEdit*>(editor)) {
        const QString iso = date->date() == date->minimumDate()
                                ? QString()
                                : date->date().toString("yyyy-MM-dd");
        model->setData(index, iso, Qt::EditRole);
        return;
    }
    if (auto* spin = qobject_cast<QSpinBox*>(editor)) {
        model->setData(index, spin->value() == 0 ? QString() : QString::number(spin->value()),
                       Qt::EditRole);
        return;
    }
    if (auto* line = qobject_cast<QLineEdit*>(editor)) {
        model->setData(index, line->text(), Qt::EditRole);
        return;
    }
    QStyledItemDelegate::setModelData(editor, model, index);
}

void CellDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const {
    const QString value = index.data(Qt::DisplayRole).toString();
    const int column = index.column();

    bool isChip = false;
    QColor chipColour = theme::textSecondary();
    if (field_ && column == provenanceColumnIndex(*field_)) {
        isChip = !value.isEmpty();
        chipColour = chrome::provenanceColour(pm::provenanceFromString(value.toStdString()));
    } else if (const pm::Column* definition = columnAt(column)) {
        if (definition->type == "choice" && !value.isEmpty()) {
            isChip = true;
            chipColour = theme::toneColour(toneForValue(*definition, value));
        }
    }

    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, theme::accentAt(46));

    if (!isChip) {
        QStyleOptionViewItem plain(option);
        initStyleOption(&plain, index);
        plain.state &= ~QStyle::State_Selected;
        plain.palette.setColor(QPalette::Text, theme::textBody());
        QStyledItemDelegate::paint(painter, plain, index);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QFont chipFont = theme::labelFont(9);
    painter->setFont(chipFont);

    QFontMetrics metrics(chipFont);
    const int textWidth = metrics.horizontalAdvance(value.toUpper());
    QRect chip(option.rect.left() + 6,
               option.rect.center().y() - 9,
               qMin(textWidth + 16, option.rect.width() - 12), 18);

    QColor border = chipColour; border.setAlpha(100);
    QColor fill = chipColour;   fill.setAlpha(22);
    painter->setPen(QPen(border, 1));
    painter->setBrush(fill);
    painter->drawRoundedRect(QRectF(chip).adjusted(0.5, 0.5, -0.5, -0.5), 2, 2);

    painter->setPen(chipColour);
    painter->drawText(chip, Qt::AlignCenter, metrics.elidedText(value, Qt::ElideRight, chip.width() - 10));
    painter->restore();
}

QSize CellDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QSize hint = QStyledItemDelegate::sizeHint(option, index);
    hint.setHeight(qMax(hint.height(), 32));
    return hint;
}

// ── TableEditor ──────────────────────────────────────────────────────────

TableEditor::TableEditor(const pm::Field& field, QWidget* parent)
    : QWidget(parent), field_(field) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    table_ = new QTableWidget(this);
    const int columnCount = static_cast<int>(field_.columns.size()) + 2;
    table_->setColumnCount(columnCount);

    QStringList headers;
    for (const pm::Column& column : field_.columns)
        headers << QString::fromStdString(column.label);
    headers << "Source" << "Evidence";
    table_->setHorizontalHeaderLabels(headers);

    table_->verticalHeader()->setVisible(false);
    table_->setAlternatingRowColors(true);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table_->setItemDelegate(new CellDelegate(&field_, table_));
    table_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked |
                            QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);
    table_->setWordWrap(false);
    table_->setShowGrid(true);
    table_->horizontalHeader()->setHighlightSections(false);
    table_->horizontalHeader()->setFont(theme::labelFont(10));

    QHeaderView* header = table_->horizontalHeader();
    for (int i = 0; i < static_cast<int>(field_.columns.size()); ++i) {
        header->setSectionResizeMode(i, QHeaderView::Interactive);
        table_->setColumnWidth(i, 70 + field_.columns[i].width * 52);
    }
    header->setSectionResizeMode(provenanceColumnIndex(field_), QHeaderView::Fixed);
    table_->setColumnWidth(provenanceColumnIndex(field_), 110);
    header->setSectionResizeMode(evidenceColumnIndex(field_), QHeaderView::Stretch);

    auto* toolbar = new QHBoxLayout;
    toolbar->setSpacing(8);
    auto* add = chrome::button("+ row", "primary", this);
    auto* remove = chrome::button("remove", "danger", this);
    count_ = chrome::label("0 rows", 10, theme::textFaint());
    toolbar->addWidget(add);
    toolbar->addWidget(remove);
    toolbar->addStretch(1);
    toolbar->addWidget(count_);

    layout->addWidget(table_);
    layout->addLayout(toolbar);

    connect(add, &QPushButton::clicked, this, &TableEditor::appendBlankRow);
    connect(remove, &QPushButton::clicked, this, &TableEditor::removeSelectedRows);
    connect(table_, &QTableWidget::itemChanged, this, [this] {
        if (!loading_) emit edited();
    });
    connect(table_, &QTableWidget::currentCellChanged, this,
            [this](int, int column, int, int) {
                if (column >= 0 && column < static_cast<int>(field_.columns.size()))
                    emit columnFocused(QString::fromStdString(field_.columns[column].id));
            });
    connect(table_, &QTableWidget::cellDoubleClicked, this,
            [this](int row, int) { emit rowActivated(row); });
}

void TableEditor::addRow(const pm::Row& row) {
    const int at = table_->rowCount();
    table_->insertRow(at);
    table_->setRowHeight(at, 34);

    for (int i = 0; i < static_cast<int>(field_.columns.size()); ++i) {
        auto* item = new QTableWidgetItem(
            QString::fromStdString(row.cell(field_.columns[i].id)));
        item->setFont(theme::bodyFont(13));
        table_->setItem(at, i, item);
    }
    auto* source = new QTableWidgetItem(
        row.provenance == pm::Provenance::Untagged
            ? QString()
            : QString::fromStdString(pm::provenanceToString(row.provenance)));
    table_->setItem(at, provenanceColumnIndex(field_), source);

    auto* evidence = new QTableWidgetItem(QString::fromStdString(row.evidence));
    evidence->setFont(theme::bodyFont(12));
    evidence->setForeground(theme::textFaint());
    table_->setItem(at, evidenceColumnIndex(field_), evidence);

    // The row id rides along on the first cell so a reorder cannot lose it.
    table_->item(at, 0)->setData(Qt::UserRole, QString::fromStdString(row.id));
}

void TableEditor::setRows(const std::vector<pm::Row>& rows) {
    loading_ = true;
    table_->setRowCount(0);
    for (const pm::Row& row : rows) addRow(row);
    loading_ = false;
    refreshCount();
}

std::vector<pm::Row> TableEditor::rows() const {
    std::vector<pm::Row> collected;
    for (int at = 0; at < table_->rowCount(); ++at) {
        pm::Row row;
        if (QTableWidgetItem* first = table_->item(at, 0))
            row.id = first->data(Qt::UserRole).toString().toStdString();
        if (row.id.empty()) row.id = "r" + std::to_string(at + 1);

        for (int i = 0; i < static_cast<int>(field_.columns.size()); ++i) {
            QTableWidgetItem* item = table_->item(at, i);
            if (!item) continue;
            const QString value = item->text();
            if (!value.isEmpty()) row.setCell(field_.columns[i].id, value.toStdString());
        }
        if (QTableWidgetItem* source = table_->item(at, provenanceColumnIndex(field_)))
            row.provenance = pm::provenanceFromString(source->text().toStdString());
        if (QTableWidgetItem* evidence = table_->item(at, evidenceColumnIndex(field_)))
            row.evidence = evidence->text().toStdString();
        collected.push_back(std::move(row));
    }
    return collected;
}

void TableEditor::appendBlankRow() {
    pm::Row row;
    row.id = "r" + std::to_string(table_->rowCount() + 1);
    addRow(row);
    refreshCount();
    table_->setCurrentCell(table_->rowCount() - 1, 0);
    table_->editItem(table_->item(table_->rowCount() - 1, 0));
    emit edited();
}

void TableEditor::removeSelectedRows() {
    QList<int> targets;
    for (const QModelIndex& index : table_->selectionModel()->selectedRows())
        targets << index.row();
    if (targets.isEmpty() && table_->currentRow() >= 0) targets << table_->currentRow();
    std::sort(targets.begin(), targets.end(), std::greater<int>());
    for (int row : targets) table_->removeRow(row);
    refreshCount();
    if (!targets.isEmpty()) emit edited();
}

void TableEditor::refreshCount() {
    const int rows = table_->rowCount();
    count_->setText(rows == 1 ? "1 row" : QString("%1 rows").arg(rows));
}
