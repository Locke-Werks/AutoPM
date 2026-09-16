#include "fieldeditors.h"

#include "spellcheck.h"
#include "theme.h"

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMenu>
#include <QPushButton>
#include <QTextBlock>
#include <QSpinBox>

namespace editors {

// ── GrowingTextEdit ──────────────────────────────────────────────────────

GrowingTextEdit::GrowingTextEdit(QWidget* parent) : QTextEdit(parent) {
    setAcceptRichText(false);
    // A form is tabbed through. Nobody wants a tab character inside a charter
    // field, and every other control in the window already moves on Tab.
    setTabChangesFocus(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setFont(theme::bodyFont(14));
    new SpellHighlighter(document());
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &GrowingTextEdit::showContextMenu);

    connect(this, &QTextEdit::textChanged, this, &GrowingTextEdit::fitToContent);
    connect(document(), &QTextDocument::contentsChanged, this, &GrowingTextEdit::fitToContent);
}

void GrowingTextEdit::setMinimumLines(int lines) {
    minimumLines_ = qMax(1, lines);
    fitToContent();
}

void GrowingTextEdit::fitToContent() {
    document()->setTextWidth(viewport()->width());
    const int lineHeight = QFontMetrics(font()).lineSpacing();
    const int content = static_cast<int>(document()->size().height()) + 18;
    const int floorHeight = minimumLines_ * lineHeight + 18;
    const int wanted = qBound(floorHeight, content, 900);
    if (height() != wanted) {
        setFixedHeight(wanted);
        updateGeometry();
    }
}

void GrowingTextEdit::showContextMenu(const QPoint& where) {
    QMenu* menu = createStandardContextMenu();

    QTextCursor cursor = cursorForPosition(where);
    cursor.select(QTextCursor::WordUnderCursor);
    const QString word = cursor.selectedText();

    const QStringList suggestions =
        spell::available() && !word.isEmpty() ? spell::suggestionsFor(word) : QStringList();
    const bool wrong = !suggestions.isEmpty();

    if (wrong) {
        auto* first = menu->actions().isEmpty() ? nullptr : menu->actions().first();
        for (const QString& option : suggestions) {
            auto* action = new QAction(option, menu);
            connect(action, &QAction::triggered, this, [this, cursor, option]() mutable {
                cursor.insertText(option);
            });
            menu->insertAction(first, action);
        }
        auto* ignore = new QAction(QString("Ignore \"%1\"").arg(word), menu);
        connect(ignore, &QAction::triggered, this, [this, word] {
            spell::ignoreWord(word);
            document()->markContentsDirty(0, document()->characterCount());
        });
        auto* learn = new QAction(QString("Add \"%1\" to the dictionary").arg(word), menu);
        connect(learn, &QAction::triggered, this, [this, word] {
            spell::addToDictionary(word);
            document()->markContentsDirty(0, document()->characterCount());
        });
        menu->insertAction(first, ignore);
        menu->insertAction(first, learn);
        menu->insertSeparator(first);
    }

    menu->exec(mapToGlobal(where));
    delete menu;
}

void GrowingTextEdit::resizeEvent(QResizeEvent* event) {
    QTextEdit::resizeEvent(event);
    fitToContent();
}

QSize GrowingTextEdit::sizeHint() const {
    QSize hint = QTextEdit::sizeHint();
    hint.setHeight(height());
    return hint;
}

// ── ClearableDateEdit ────────────────────────────────────────────────────

ClearableDateEdit::ClearableDateEdit(QWidget* parent) : QWidget(parent) {
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(6);

    date_ = new QDateEdit(this);
    date_->setDisplayFormat("yyyy-MM-dd");
    date_->setCalendarPopup(true);
    date_->setDate(QDate::currentDate());
    date_->setSpecialValueText(" — ");
    date_->setMinimumDate(QDate(1900, 1, 1));

    auto* clear = new QPushButton("×", this);
    clear->setProperty("house", "quiet");
    clear->setFixedWidth(26);
    clear->setToolTip("Clear the date");
    clear->setCursor(Qt::PointingHandCursor);

    row->addWidget(date_, 1);
    row->addWidget(clear);

    connect(date_, &QDateEdit::dateChanged, this, [this] {
        if (empty_) setEmpty(false);
        emit edited();
    });
    connect(clear, &QPushButton::clicked, this, [this] {
        setEmpty(true);
        emit edited();
    });
    setEmpty(true);
}

void ClearableDateEdit::setEmpty(bool empty) {
    empty_ = empty;
    QSignalBlocker block(date_);
    if (empty) {
        date_->setDate(date_->minimumDate());
    } else if (date_->date() == date_->minimumDate()) {
        date_->setDate(QDate::currentDate());
    }
}

QString ClearableDateEdit::isoDate() const {
    if (empty_ || date_->date() == date_->minimumDate()) return QString();
    return date_->date().toString("yyyy-MM-dd");
}

void ClearableDateEdit::setIsoDate(const QString& iso) {
    QSignalBlocker block(date_);
    const QDate parsed = QDate::fromString(iso.trimmed(), "yyyy-MM-dd");
    if (parsed.isValid()) {
        date_->setDate(parsed);
        empty_ = false;
    } else {
        date_->setDate(date_->minimumDate());
        empty_ = true;
    }
}

// ── Factory ──────────────────────────────────────────────────────────────

void fillChoices(QComboBox* combo, const std::vector<pm::Option>& options) {
    combo->clear();
    combo->addItem(QString(" — "), QString());
    for (const pm::Option& option : options) {
        const QString value = QString::fromStdString(option.value);
        combo->addItem(value, value);
        const int index = combo->count() - 1;
        combo->setItemData(index, theme::toneColour(option.tone), Qt::ForegroundRole);
    }
}

QWidget* makeEditor(const QString& type, const std::vector<pm::Option>& options,
                    const QString& placeholder, QWidget* parent) {
    if (type == "choice") {
        auto* combo = new QComboBox(parent);
        fillChoices(combo, options);
        combo->view()->setTextElideMode(Qt::ElideRight);
        return combo;
    }
    if (type == "date") {
        return new ClearableDateEdit(parent);
    }
    if (type == "number") {
        auto* spin = new QSpinBox(parent);
        spin->setRange(0, 9999);
        spin->setSpecialValueText(" — ");
        return spin;
    }
    if (type == "text") {
        auto* edit = new GrowingTextEdit(parent);
        edit->setPlaceholderText(placeholder);
        return edit;
    }
    auto* line = new QLineEdit(parent);
    line->setPlaceholderText(placeholder);
    line->setFont(theme::bodyFont(14));
    return line;
}

QString editorValue(QWidget* editor) {
    if (auto* combo = qobject_cast<QComboBox*>(editor)) return combo->currentData().toString();
    if (auto* date = qobject_cast<ClearableDateEdit*>(editor)) return date->isoDate();
    if (auto* spin = qobject_cast<QSpinBox*>(editor))
        return spin->value() == 0 ? QString() : QString::number(spin->value());
    if (auto* text = qobject_cast<QTextEdit*>(editor)) return text->toPlainText();
    if (auto* line = qobject_cast<QLineEdit*>(editor)) return line->text();
    return QString();
}

void setEditorValue(QWidget* editor, const QString& value) {
    if (auto* combo = qobject_cast<QComboBox*>(editor)) {
        int index = combo->findData(value);
        // A value the definition no longer lists is kept rather than dropped,
        // so editing a definition never silently loses record content.
        if (index < 0 && !value.isEmpty()) {
            combo->addItem(value + "  (not in the definition)", value);
            index = combo->count() - 1;
            combo->setItemData(index, theme::warn(), Qt::ForegroundRole);
        }
        combo->setCurrentIndex(qMax(0, index));
        return;
    }
    if (auto* date = qobject_cast<ClearableDateEdit*>(editor)) { date->setIsoDate(value); return; }
    if (auto* spin = qobject_cast<QSpinBox*>(editor)) { spin->setValue(value.toInt()); return; }
    if (auto* text = qobject_cast<QTextEdit*>(editor)) { text->setPlainText(value); return; }
    if (auto* line = qobject_cast<QLineEdit*>(editor)) { line->setText(value); return; }
}

void connectChanged(QWidget* editor, QObject* context, const std::function<void()>& slot) {
    if (auto* combo = qobject_cast<QComboBox*>(editor)) {
        QObject::connect(combo, &QComboBox::currentIndexChanged, context, [slot](int) { slot(); });
        return;
    }
    if (auto* date = qobject_cast<ClearableDateEdit*>(editor)) {
        QObject::connect(date, &ClearableDateEdit::edited, context, slot);
        return;
    }
    if (auto* spin = qobject_cast<QSpinBox*>(editor)) {
        QObject::connect(spin, &QSpinBox::valueChanged, context, [slot](int) { slot(); });
        return;
    }
    if (auto* text = qobject_cast<QTextEdit*>(editor)) {
        QObject::connect(text, &QTextEdit::textChanged, context, slot);
        return;
    }
    if (auto* line = qobject_cast<QLineEdit*>(editor)) {
        QObject::connect(line, &QLineEdit::textEdited, context, [slot](const QString&) { slot(); });
        return;
    }
}

} // namespace editors
