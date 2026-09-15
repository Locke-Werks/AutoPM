// The sprint board. Columns come from the allowed values of the field's
// group_by column, and the WIP limit on each one comes from that column's
// wip: line in the definition file. Nothing about the board is hard-coded.
#pragma once

#include "chrome.h"
#include "core/definition.h"
#include "core/record.h"

#include <QDialog>
#include <QListWidget>
#include <QWidget>
#include <vector>

class QComboBox;
class QLabel;
class QHBoxLayout;

// A widget that shows a table field as something other than a grid.
class RichView : public QWidget {
    Q_OBJECT
public:
    explicit RichView(QWidget* parent = nullptr) : QWidget(parent) {}
    virtual void setRows(const std::vector<pm::Row>& rows) = 0;

signals:
    void rowsChanged(const std::vector<pm::Row>& rows);
    void rowActivated(int index);
};

// One column of the board. It reports a drop so the board can rewrite the
// rows from what is actually on screen.
class BoardColumn : public QListWidget {
    Q_OBJECT
public:
    explicit BoardColumn(QWidget* parent = nullptr);

signals:
    void dropped();

protected:
    void dropEvent(QDropEvent* event) override;
};

class BoardView : public RichView {
    Q_OBJECT
public:
    BoardView(const pm::Field& field, QWidget* parent = nullptr);

    void setRows(const std::vector<pm::Row>& rows) override;

private:
    void rebuild();
    void harvest();          // read the columns back into rows_
    void addCard();
    void openCard(const QString& rowId);
    void refreshHeaders();
    QStringList sprints() const;

    const pm::Field field_;
    const pm::Column* laneColumn_ = nullptr;
    std::vector<pm::Row> rows_;
    std::vector<BoardColumn*> lists_;
    std::vector<QLabel*> counts_;
    QHBoxLayout* columnRow_ = nullptr;
    QComboBox* sprintFilter_ = nullptr;
    QLabel* summary_ = nullptr;
    QString sprintShown_;
};

// Edits every column of one row, including the multi-line ones a grid cell
// cannot show.
class CardDialog : public QDialog {
    Q_OBJECT
public:
    CardDialog(const pm::Field& field, const pm::Row& row, QWidget* parent = nullptr);
    pm::Row row() const;

private:
    const pm::Field field_;
    pm::Row row_;
    std::vector<std::pair<std::string, QWidget*>> editors_;
    chrome::ProvenanceControl* provenance_ = nullptr;
};
