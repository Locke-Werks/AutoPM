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
#include <memory>
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

    // Only the views that work on another field's rows need this; the rest
    // ignore it.
    virtual void setRecord(const std::shared_ptr<pm::Record>&) {}

signals:
    void rowsChanged(const std::vector<pm::Row>& rows);
    void rowActivated(int index);
    // Something outside this field's own rows changed.
    void recordChanged();
};

// The item roles a board card carries. Shared so the planning tool can build
// the same cards without a second delegate.
namespace card {
constexpr int RowId = Qt::UserRole + 1;
constexpr int Title = Qt::UserRole + 2;
constexpr int Meta = Qt::UserRole + 3;
constexpr int Tone = Qt::UserRole + 4;
constexpr int Blocked = Qt::UserRole + 5;
constexpr int Points = Qt::UserRole + 6;
}

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

// Sprint planning: pulling work out of the backlog into an iteration, and
// finding out whether you have pulled too much.
//
// It works on the board's cards rather than on a second list of its own,
// because a sprint that does not contain the actual work is a spreadsheet.
// The field's `reads` key names the card field it moves about.
class PlanningView : public RichView {
    Q_OBJECT
public:
    PlanningView(const pm::Field& field, QWidget* parent = nullptr);

    void setRows(const std::vector<pm::Row>& rows) override;   // the sprints
    void setRecord(const std::shared_ptr<pm::Record>& record) override;

    // The card field's definition, so a card can be opened for editing from
    // here with the same dialog the board uses.
    void setCardField(const pm::Field* field);

private:
    void rebuild();
    void harvest();
    void refreshTotals();
    void openCard(const QString& rowId);
    std::string titleColumn() const;
    const pm::Row* currentSprint() const;
    std::vector<pm::Row>* cards();
    int pointsIn(const QString& sprint, bool doneOnly) const;

    const pm::Field field_;
    const pm::Field* cardField_ = nullptr;
    std::shared_ptr<pm::Record> record_;
    std::vector<pm::Row> sprints_;
    QString showing_;

    QComboBox* picker_ = nullptr;
    QLabel* goal_ = nullptr;
    QLabel* committed_ = nullptr;
    QLabel* advice_ = nullptr;
    QWidget* meter_ = nullptr;
    QWidget* velocity_ = nullptr;
    BoardColumn* backlog_ = nullptr;
    BoardColumn* sprint_ = nullptr;
    QLabel* backlogCount_ = nullptr;
    QLabel* sprintCount_ = nullptr;
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
