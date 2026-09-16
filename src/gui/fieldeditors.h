// The editors a field type maps to. Nothing here knows which field it is
// editing: the definition file decides the type, and this hands back a widget.
#pragma once

#include "core/definition.h"

#include <functional>

#include <QComboBox>
#include <QDateEdit>
#include <QLineEdit>
#include <QTextEdit>
#include <QWidget>

namespace editors {

// A text box that grows with its content instead of scrolling inside a fixed
// three-line window, which is what makes a form of them readable.
class GrowingTextEdit : public QTextEdit {
    Q_OBJECT
public:
    explicit GrowingTextEdit(QWidget* parent = nullptr);
    QSize sizeHint() const override;
    void setMinimumLines(int lines);

    // Right-click offers corrections for the word under the cursor.
    void showContextMenu(const QPoint& where);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void fitToContent();
    int minimumLines_ = 3;
};

// A date field that can be genuinely empty, because most dates on a fresh
// charter are not known yet and a default of today is a lie.
class ClearableDateEdit : public QWidget {
    Q_OBJECT
public:
    explicit ClearableDateEdit(QWidget* parent = nullptr);

    QString isoDate() const;
    void setIsoDate(const QString& iso);

signals:
    void edited();

private:
    void setEmpty(bool empty);

    QDateEdit* date_ = nullptr;
    bool empty_ = true;
};

// Builds the editor for a field or a table cell. `owner` receives a
// FocusIn-driven signal through the returned widget's "pm-focus" event filter.
QWidget* makeEditor(const QString& type, const std::vector<pm::Option>& options,
                    const QString& placeholder, QWidget* parent = nullptr);

QString editorValue(QWidget* editor);
void setEditorValue(QWidget* editor, const QString& value);

// Connects whatever change signal the editor actually has.
void connectChanged(QWidget* editor, QObject* context, const std::function<void()>& slot);

// Fills a combo box with the options, keeping an empty first entry so a choice
// can be left unanswered.
void fillChoices(QComboBox* combo, const std::vector<pm::Option>& options);

} // namespace editors
