// The generic grid behind every table field. Columns, their types and their
// allowed values all come from the definition file.
//
// Two columns are added that the definition does not declare: the row's source
// tag and its evidence. Every row in a record carries its own provenance, not
// just the field as a whole.
#pragma once

#include "core/definition.h"
#include "core/record.h"

#include <QStyledItemDelegate>
#include <QWidget>
#include <vector>

class QTableWidget;
class QLabel;

// Draws choice values as small tone-coloured chips and supplies the right
// editor for each column type.
class CellDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit CellDelegate(const pm::Field* field, QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    const pm::Column* columnAt(int visualIndex) const;
    const pm::Field* field_ = nullptr;
};

class TableEditor : public QWidget {
    Q_OBJECT
public:
    TableEditor(const pm::Field& field, QWidget* parent = nullptr);

    void setRows(const std::vector<pm::Row>& rows);
    std::vector<pm::Row> rows() const;

signals:
    void edited();
    void columnFocused(const QString& columnId);
    void rowActivated(int row);

private:
    void addRow(const pm::Row& row);
    void appendBlankRow();
    void removeSelectedRows();
    void refreshCount();

    const pm::Field field_;
    QTableWidget* table_ = nullptr;
    QLabel* count_ = nullptr;
    bool loading_ = false;
};

// The two columns every table grows: source and evidence.
int provenanceColumnIndex(const pm::Field& field);
int evidenceColumnIndex(const pm::Field& field);
