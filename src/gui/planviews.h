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

#include <QRectF>
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
    void mouseReleaseEvent(QMouseEvent* event) override;

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
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    struct Hit {
        QRectF rect;
        std::vector<int> rows;
    };

    const pm::Field field_;
    std::vector<pm::Row> rows_;
    std::vector<Hit> hits_;   // where the last paint put each name
};

class LogView : public RichView {
    Q_OBJECT
public:
    explicit LogView(const pm::Field& field, QWidget* parent = nullptr);
    void setRows(const std::vector<pm::Row>& rows) override;

private:
    const pm::Field field_;
    QVBoxLayout* stack_ = nullptr;
};
