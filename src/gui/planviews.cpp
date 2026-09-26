#include "planviews.h"

#include "chrome.h"
#include "theme.h"

#include <QDate>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <algorithm>

namespace {

QString cell(const pm::Row& row, const std::string& column) {
    return QString::fromStdString(row.cell(column));
}

pm::Tone toneFor(const pm::Column* column, const QString& value) {
    if (!column) return pm::Tone::Neutral;
    for (const pm::Option& option : column->options)
        if (QString::fromStdString(option.value) == value) return option.tone;
    return pm::Tone::Neutral;
}

QDate parseDate(const QString& text) { return QDate::fromString(text.trimmed(), "yyyy-MM-dd"); }

int depthOfCode(const QString& code) {
    return code.trimmed().isEmpty() ? 0 : static_cast<int>(code.count('.'));
}

} // namespace

// ── TreeView ─────────────────────────────────────────────────────────────

TreeView::TreeView(const pm::Field& field, QWidget* parent) : RichView(parent), field_(field) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    tree_ = new QTreeWidget(this);
    tree_->setColumnCount(4);
    tree_->setHeaderLabels({"Item", "Level", "Owner", "Status"});
    tree_->setRootIsDecorated(true);
    tree_->setUniformRowHeights(false);
    tree_->setAlternatingRowColors(false);
    tree_->setMinimumHeight(260);
    tree_->setCursor(Qt::PointingHandCursor);
    tree_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    tree_->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tree_->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tree_->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    layout->addWidget(tree_);

    connect(tree_, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item, int) {
        emit rowActivated(item->data(0, Qt::UserRole).toInt());
    });
}

void TreeView::setRows(const std::vector<pm::Row>& rows) {
    tree_->clear();

    const pm::Column* statusColumn = field_.column(field_.toneColumn);
    const std::string outline = field_.outlineColumn.empty() ? "code" : field_.outlineColumn;
    const std::string title = field_.titleColumn.empty() ? "item" : field_.titleColumn;

    // The outline code is the structure: 1.2 is a child of 1. A row whose
    // parent code is missing is attached at the top rather than dropped.
    std::vector<std::pair<QString, QTreeWidgetItem*>> placed;

    for (size_t index = 0; index < rows.size(); ++index) {
        const pm::Row& row = rows[index];
        const QString code = cell(row, outline);

        QTreeWidgetItem* parentItem = nullptr;
        const int depth = depthOfCode(code);
        if (depth > 0) {
            const QString parentCode = code.section('.', 0, depth - 1);
            for (auto it = placed.rbegin(); it != placed.rend(); ++it) {
                if (it->first == parentCode) { parentItem = it->second; break; }
            }
        }

        auto* item = parentItem ? new QTreeWidgetItem(parentItem) : new QTreeWidgetItem(tree_);
        const QString label = code.isEmpty() ? cell(row, title)
                                             : QString("%1  %2").arg(code, cell(row, title));
        item->setText(0, label);
        item->setFont(0, theme::bodyFont(13));
        item->setForeground(0, depth == 0 ? theme::textPrimary() : theme::textBody());
        item->setText(1, cell(row, "kind"));
        item->setFont(1, theme::labelFont(9));
        item->setForeground(1, theme::textLabel());
        item->setText(2, cell(row, "owner"));
        item->setForeground(2, theme::textSecondary());

        const QString status = cell(row, field_.toneColumn);
        item->setText(3, status);
        item->setFont(3, theme::labelFont(9));
        item->setForeground(3, theme::toneColour(toneFor(statusColumn, status)));
        item->setData(0, Qt::UserRole, static_cast<int>(index));
        item->setToolTip(0, cell(row, "notes"));

        placed.emplace_back(code, item);
    }
    tree_->expandAll();
}

// ── TimelineView ─────────────────────────────────────────────────────────

TimelineView::TimelineView(const pm::Field& field, QWidget* parent)
    : RichView(parent), field_(field) {
    setMinimumHeight(200);
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
}

void TimelineView::setRows(const std::vector<pm::Row>& rows) {
    rows_ = rows;
    updateGeometry();
    update();
}

QSize TimelineView::sizeHint() const {
    return QSize(600, 56 + static_cast<int>(rows_.size()) * 30 + 14);
}

void TimelineView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    const int index = (event->pos().y() - 56) / 30;
    if (index >= 0 && index < static_cast<int>(rows_.size())) emit rowActivated(index);
}

void TimelineView::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const std::string startId = field_.startColumn.empty() ? "start" : field_.startColumn;
    const std::string finishId = field_.finishColumn.empty() ? "finish" : field_.finishColumn;
    const std::string titleId = field_.titleColumn.empty() ? "activity" : field_.titleColumn;
    const pm::Column* statusColumn = field_.column(field_.toneColumn);

    QDate earliest, latest;
    for (const pm::Row& row : rows_) {
        const QDate start = parseDate(cell(row, startId));
        const QDate finish = parseDate(cell(row, finishId));
        for (const QDate& date : {start, finish}) {
            if (!date.isValid()) continue;
            if (!earliest.isValid() || date < earliest) earliest = date;
            if (!latest.isValid() || date > latest) latest = date;
        }
    }

    if (!earliest.isValid()) {
        painter.setPen(theme::textFaint());
        painter.setFont(theme::bodyFont(13));
        painter.drawText(rect(), Qt::AlignCenter,
                         "No dates yet. Add rows with a start and a finish and they appear here.");
        return;
    }

    // A little air on both ends, so a bar never touches the edge.
    earliest = earliest.addDays(-2);
    latest = latest.addDays(2);
    if (earliest.daysTo(latest) < 14) latest = earliest.addDays(14);

    const int labelWidth = 214;
    const int left = labelWidth + 10;
    const int right = width() - 14;
    const qreal span = qMax(1, earliest.daysTo(latest));
    const auto xFor = [&](const QDate& date) {
        return left + (right - left) * (earliest.daysTo(date) / span);
    };

    // Month gridlines and labels.
    painter.setFont(theme::labelFont(9));
    QDate tick(earliest.year(), earliest.month(), 1);
    if (tick < earliest) tick = tick.addMonths(1);
    while (tick <= latest) {
        const int x = static_cast<int>(xFor(tick));
        painter.setPen(QPen(theme::hairline(), 1));
        painter.drawLine(x, 34, x, height() - 6);
        painter.setPen(theme::textFaint());
        painter.drawText(QRect(x + 5, 14, 90, 16), Qt::AlignLeft | Qt::AlignVCenter,
                         tick.toString("MMM yyyy"));
        tick = tick.addMonths(1);
    }

    // Today, if it is inside the window: the one line worth an accent.
    const QDate today = QDate::currentDate();
    if (today >= earliest && today <= latest) {
        const int x = static_cast<int>(xFor(today));
        painter.setPen(QPen(theme::accentAt(150), 1, Qt::DashLine));
        painter.drawLine(x, 30, x, height() - 6);
        painter.setPen(theme::accent());
        painter.setFont(theme::labelFont(8));
        painter.drawText(QRect(x - 30, 34, 60, 14), Qt::AlignCenter, "today");
    }

    painter.setPen(QPen(theme::hairline(), 1));
    painter.drawLine(0, 50, width(), 50);

    int y = 56;
    for (const pm::Row& row : rows_) {
        const QDate start = parseDate(cell(row, startId));
        QDate finish = parseDate(cell(row, finishId));
        if (!finish.isValid()) finish = start;

        painter.setPen(theme::textBody());
        painter.setFont(theme::bodyFont(12));
        const QString title = cell(row, titleId);
        painter.drawText(QRect(4, y, labelWidth, 26), Qt::AlignLeft | Qt::AlignVCenter,
                         painter.fontMetrics().elidedText(title, Qt::ElideRight, labelWidth - 8));

        if (start.isValid()) {
            const QString status = cell(row, field_.toneColumn);
            QColor colour = theme::toneColour(toneFor(statusColumn, status));
            if (status.isEmpty()) colour = theme::accent();

            const int x1 = static_cast<int>(xFor(start));
            const int x2 = static_cast<int>(xFor(finish));

            if (x2 - x1 < 4) {
                // A zero-length activity is a milestone: draw the diamond the
                // schedule convention uses, not a one-pixel bar.
                QPainterPath diamond;
                const int cx = x1;
                const int cy = y + 13;
                diamond.moveTo(cx, cy - 7);
                diamond.lineTo(cx + 7, cy);
                diamond.lineTo(cx, cy + 7);
                diamond.lineTo(cx - 7, cy);
                diamond.closeSubpath();
                painter.setPen(QPen(colour, 1));
                QColor fill = colour; fill.setAlpha(150);
                painter.setBrush(fill);
                painter.drawPath(diamond);
            } else {
                QRectF bar(x1, y + 6, x2 - x1, 15);
                QColor fill = colour; fill.setAlpha(46);
                painter.setPen(QPen(QColor(colour.red(), colour.green(), colour.blue(), 130), 1));
                painter.setBrush(fill);
                painter.drawRoundedRect(bar, 3, 3);

                const int percent = cell(row, field_.progressColumn).toInt();
                if (percent > 0) {
                    QRectF done(bar.left(), bar.top(), bar.width() * qMin(100, percent) / 100.0,
                                bar.height());
                    QColor solid = colour; solid.setAlpha(190);
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(solid);
                    painter.drawRoundedRect(done, 3, 3);
                }
            }
        }

        painter.setPen(QPen(theme::hairline(), 1));
        painter.drawLine(0, y + 28, width(), y + 28);
        y += 30;
    }
}

// ── MatrixView ───────────────────────────────────────────────────────────

MatrixView::MatrixView(const pm::Field& field, QWidget* parent)
    : RichView(parent), field_(field) {
    setMinimumHeight(260);
    setMouseTracking(true);
}

void MatrixView::setRows(const std::vector<pm::Row>& rows) {
    rows_ = rows;
    update();
}

QSize MatrixView::sizeHint() const { return QSize(600, 320); }

void MatrixView::mouseMoveEvent(QMouseEvent* event) {
    const bool over = std::any_of(hits_.begin(), hits_.end(), [&](const Hit& hit) {
        return hit.rect.contains(event->position());
    });
    setCursor(over ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

// A name opens its entry. "+3 more" lists the three, because a cell too full
// to draw them is exactly where one needs finding.
void MatrixView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    for (const Hit& hit : hits_) {
        if (!hit.rect.contains(event->position())) continue;
        if (hit.rows.size() == 1) {
            emit rowActivated(hit.rows.front());
            return;
        }
        QMenu menu(this);
        for (int index : hit.rows) {
            const pm::Row& row = rows_[static_cast<size_t>(index)];
            QString name = cell(row, field_.titleColumn);
            const QString ref = cell(row, "ref");
            if (!ref.isEmpty()) name = ref + " " + name;
            connect(menu.addAction(name), &QAction::triggered, this,
                    [this, index] { emit rowActivated(index); });
        }
        menu.exec(event->globalPosition().toPoint());
        return;
    }
}

void MatrixView::paintEvent(QPaintEvent*) {
    const pm::Column* rowAxis = field_.column(field_.rowsColumn);
    const pm::Column* colAxis = field_.column(field_.colsColumn);
    if (!rowAxis || !colAxis || rowAxis->options.empty() || colAxis->options.empty()) return;

    hits_.clear();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int axisLeft = 74;
    const int axisTop = 34;
    const int rows = static_cast<int>(rowAxis->options.size());
    const int cols = static_cast<int>(colAxis->options.size());
    const qreal cellWidth = (width() - axisLeft - 12) / static_cast<qreal>(cols);
    const qreal cellHeight = (height() - axisTop - 30) / static_cast<qreal>(rows);

    painter.setFont(theme::labelFont(9));
    painter.setPen(theme::textLabel());
    painter.drawText(QRect(4, 4, axisLeft + 60, 18), Qt::AlignLeft | Qt::AlignVCenter,
                     QString::fromStdString(rowAxis->label));
    painter.drawText(QRect(width() - 200, height() - 24, 190, 18),
                     Qt::AlignRight | Qt::AlignVCenter, QString::fromStdString(colAxis->label));

    for (int c = 0; c < cols; ++c) {
        painter.setPen(theme::textSecondary());
        painter.drawText(QRectF(axisLeft + c * cellWidth, axisTop - 22, cellWidth, 18),
                         Qt::AlignCenter, QString::fromStdString(colAxis->options[c].value));
    }

    for (int r = 0; r < rows; ++r) {
        painter.setPen(theme::textSecondary());
        painter.drawText(QRectF(4, axisTop + r * cellHeight, axisLeft - 10, cellHeight),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::fromStdString(rowAxis->options[r].value));

        for (int c = 0; c < cols; ++c) {
            const QRectF box(axisLeft + c * cellWidth + 2, axisTop + r * cellHeight + 2,
                             cellWidth - 4, cellHeight - 4);

            // Option index 0 is the strongest value on both axes, so the top
            // left corner is the one that matters most.
            const qreal worst = (rows - 1) + (cols - 1);
            const qreal severity = worst <= 0 ? 1.0
                                              : ((rows - 1 - r) + (cols - 1 - c)) / worst;
            QColor tint = severity > 0.66 ? theme::danger()
                        : severity > 0.33 ? theme::warn()
                                          : theme::textFaint();

            QColor fill = tint; fill.setAlpha(static_cast<int>(10 + severity * 26));
            QColor edge = tint; edge.setAlpha(static_cast<int>(40 + severity * 70));
            painter.setPen(QPen(edge, 1));
            painter.setBrush(fill);
            painter.drawRoundedRect(box, 6, 6);

            QStringList names;
            std::vector<int> indices;
            for (int i = 0; i < static_cast<int>(rows_.size()); ++i) {
                const pm::Row& row = rows_[static_cast<size_t>(i)];
                if (cell(row, rowAxis->id) != QString::fromStdString(rowAxis->options[r].value))
                    continue;
                if (cell(row, colAxis->id) != QString::fromStdString(colAxis->options[c].value))
                    continue;
                QString name = cell(row, field_.titleColumn);
                const QString ref = cell(row, "ref");
                if (!ref.isEmpty()) name = ref + " " + name;
                names << name;
                indices.push_back(i);
            }
            if (names.isEmpty()) continue;

            painter.setPen(theme::textBody());
            painter.setFont(theme::bodyFont(11));
            QRectF text = box.adjusted(9, 8, -9, -8);
            int line = 0;
            const int lineHeight = painter.fontMetrics().lineSpacing();
            for (const QString& name : names) {
                const QRectF slot(text.left(), text.top() + line * lineHeight, text.width(),
                                  lineHeight);
                if ((line + 1) * lineHeight > text.height()) {
                    hits_.push_back({slot, std::vector<int>(indices.begin() + line, indices.end())});
                    painter.setPen(theme::textFaint());
                    painter.drawText(slot,
                                     Qt::AlignLeft | Qt::AlignVCenter,
                                     QString("+%1 more").arg(names.size() - line));
                    break;
                }
                hits_.push_back({slot, {indices[static_cast<size_t>(line)]}});
                painter.drawText(slot, Qt::AlignLeft | Qt::AlignVCenter,
                                 painter.fontMetrics().elidedText(name, Qt::ElideRight,
                                                                  static_cast<int>(text.width())));
                ++line;
            }
        }
    }
}

// ── LogView ──────────────────────────────────────────────────────────────

LogView::LogView(const pm::Field& field, QWidget* parent) : RichView(parent), field_(field) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(12);
    stack_ = new QVBoxLayout;
    stack_->setContentsMargins(0, 0, 0, 0);
    stack_->setSpacing(9);
    outer->addLayout(stack_);

    // Adding belongs where the entries are. Sending people to the table view
    // to add one is how an empty log comes to look like it cannot be written.
    auto* toolbar = new QHBoxLayout;
    auto* add = chrome::button("+ entry", "primary", this);
    connect(add, &QPushButton::clicked, this, &LogView::addEntry);
    toolbar->addWidget(add);
    toolbar->addStretch(1);
    outer->addLayout(toolbar);

    setRows({});
}

void LogView::addEntry() {
    pm::Row row;
    row.id = "r" + std::to_string(rows_.size() + 1);
    // The next reference in the log's own series: I7 after I6, D24 after D23.
    if (field_.column("ref")) {
        QString prefix;
        int highest = 0;
        for (const pm::Row& existing : rows_) {
            const QString ref = cell(existing, "ref");
            int at = static_cast<int>(ref.size());
            while (at > 0 && ref[at - 1].isDigit()) --at;
            if (at == ref.size()) continue;
            prefix = ref.left(at);
            highest = qMax(highest, ref.mid(at).toInt());
        }
        if (!prefix.isEmpty())
            row.setCell("ref", (prefix + QString::number(highest + 1)).toStdString());
    }
    CardDialog dialog(field_, row, this);
    if (dialog.exec() != QDialog::Accepted) return;
    std::vector<pm::Row> rows = rows_;
    rows.push_back(dialog.row());
    setRows(rows);
    emit rowsChanged(rows);
}

void LogView::setRows(const std::vector<pm::Row>& rows) {
    rows_ = rows;
    while (QLayoutItem* item = stack_->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    if (rows.empty()) {
        auto* empty = chrome::bodyText("Nothing logged yet.", theme::textFaint(), 13);
        stack_->addWidget(empty);
        return;
    }

    const pm::Column* toneColumn = field_.column(field_.toneColumn);
    const std::string titleId = field_.titleColumn.empty()
                                    ? (field_.columns.empty() ? "" : field_.columns[0].id)
                                    : field_.titleColumn;

    for (size_t index = 0; index < rows.size(); ++index) {
        const pm::Row& row = rows[index];
        auto* card = new chrome::Card(this);
        card->setHoverable(true);
        card->body()->setContentsMargins(17, 14, 17, 14);
        card->body()->setSpacing(7);

        auto* head = new QHBoxLayout;
        head->setSpacing(8);
        const QString ref = cell(row, "ref");
        if (!ref.isEmpty()) {
            auto* badge = new chrome::Badge(ref, card);
            badge->setColour(theme::textLabel());
            head->addWidget(badge);
        }
        const QString tone = cell(row, field_.toneColumn);
        if (!tone.isEmpty()) {
            auto* badge = new chrome::Badge(tone, card);
            badge->setTone(toneFor(toneColumn, tone));
            head->addWidget(badge);
        }
        head->addStretch(1);
        if (row.provenance != pm::Provenance::Untagged) {
            auto* badge = new chrome::Badge(chrome::provenanceName(row.provenance), card);
            badge->setColour(chrome::provenanceColour(row.provenance));
            head->addWidget(badge);
        }
        card->body()->addLayout(head);

        auto* title = chrome::bodyText(cell(row, titleId), theme::textPrimary(), 14);
        card->body()->addWidget(title);

        QStringList details;
        for (const pm::Column& column : field_.columns) {
            if (column.id == titleId || column.id == "ref" || column.id == field_.toneColumn)
                continue;
            const QString value = cell(row, column.id);
            if (value.trimmed().isEmpty()) continue;
            details << QString("%1: %2").arg(QString::fromStdString(column.label), value);
        }
        if (!details.isEmpty())
            card->body()->addWidget(chrome::bodyText(details.join("\n"), theme::textSecondary(), 12));

        if (!row.evidence.empty()) {
            auto* evidence = chrome::bodyText(QString::fromStdString(row.evidence),
                                              theme::textFaint(), 12);
            evidence->setFont(theme::monoFont(11));
            card->body()->addWidget(evidence);
        }

        chrome::passClicksThrough(card);
        card->setOnClick([this, index] { emit rowActivated(static_cast<int>(index)); });
        stack_->addWidget(card);
    }
}
