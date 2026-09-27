// One screen, built from its definition. Every field becomes a card: its
// label, whether it is standard or the tool's own, its editor, and its source tag.
//
// A table field whose definition names a view (board, tree, timeline, matrix)
// gets that view alongside the grid, with a toggle between them.
#pragma once

#include "core/definition.h"
#include "core/record.h"

#include <QHash>
#include <QWidget>
#include <memory>

class QVBoxLayout;
class TableEditor;

class ScreenView : public QWidget {
    Q_OBJECT
public:
    ScreenView(const pm::Screen& screen, const std::shared_ptr<pm::Record>& record,
               const pm::Definitions* definitions, QWidget* parent = nullptr);

    const pm::Screen& screen() const { return screen_; }
    void reload();

    // The card holding one field, or null when this screen has no such field.
    // Lets a caller scroll to an entry instead of dropping somebody at the top
    // of a page and leaving them to find which line was meant.
    QWidget* fieldCard(const QString& fieldId) const;

signals:
    void walkThrough();
    void fieldFocused(const QString& fieldId);
    void columnFocused(const QString& fieldId, const QString& columnId);
    void screenFocused();
    void recordEdited();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QWidget* buildHeader();
    QWidget* buildFieldCard(const pm::Field& field);
    QWidget* buildRichView(const pm::Field& field, TableEditor* grid);
    std::string keyFor(const pm::Field& field) const;

    const pm::Screen screen_;
    std::shared_ptr<pm::Record> record_;
    const pm::Definitions* definitions_ = nullptr;
    QVBoxLayout* column_ = nullptr;
    QHash<QObject*, QString> focusOwners_;
    QHash<QString, QWidget*> fieldCards_;
};
