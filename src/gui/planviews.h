// The other three ways a table gets drawn: a work breakdown as a tree, a
// schedule as bars on a date axis, a register as a probability-and-impact
// grid, and a log as a stack of entries.
//
// None of them is a second copy of the data. They read the same rows the grid
// edits, and the grid stays one toggle away.
#pragma once

#include "boardview.h"
#include "core/definition.h"
#include "core/record.h"

#include <QWidget>
#include <vector>

class QTreeWidget;
class QVBoxLayout;

class TreeView : public RichView {
    Q_OBJECT
public:
    explicit TreeView(const pm::Field& field, QWidget* parent = nullptr);
    void setRows(const std::vector<pm::Row>& rows) override;

private:
    const pm::Field field_;
    QTreeWidget* tree_ = nullptr;
};

class TimelineView : public RichView {
    Q_OBJECT
public:
    explicit TimelineView(const pm::Field& field, QWidget* parent = nullptr);
    void setRows(const std::vector<pm::Row>& rows) override;
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    const pm::Field field_;
    std::vector<pm::Row> rows_;
};

class MatrixView : public RichView {
    Q_OBJECT
public:
    explicit MatrixView(const pm::Field& field, QWidget* parent = nullptr);
    void setRows(const std::vector<pm::Row>& rows) override;
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    const pm::Field field_;
    std::vector<pm::Row> rows_;
};

class LogView : public RichView {
    Q_OBJECT
public:
    explicit LogView(const pm::Field& field, QWidget* parent = nullptr);
    void setRows(const std::vector<pm::Row>& rows) override;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    const pm::Field field_;
    QVBoxLayout* stack_ = nullptr;
};
