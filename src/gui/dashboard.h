// The project's own front page: how far through the lifecycle it is, what the
// record actually claims, and which fields are still blank.
//
// The gaps list is the point. Every empty field already carries the sentence
// explaining what breaks when nobody fills it in, so the tool can tell you
// what you are risking rather than just showing an empty box.
#pragma once

#include "core/definition.h"
#include "core/record.h"

#include <QWidget>
#include <memory>

class QVBoxLayout;

class Dashboard : public QWidget {
    Q_OBJECT
public:
    Dashboard(const pm::Definitions& definitions, const std::shared_ptr<pm::Record>& record,
              QWidget* parent = nullptr);

    void refresh();

signals:
    void openScreen(const QString& screenId);
    // A screen, scrolled to the entry a stat tile counted. A number is only
    // useful if clicking it lands on one of the things it counted.
    void openEntry(const QString& screenId, const QString& fieldId);
    void walkThrough(const QString& screenId);
    // The phase or summary was changed here.
    void recordEdited();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QWidget* buildHeader();
    QWidget* buildNextStep();
    QWidget* buildStats();
    QWidget* buildPhases();
    QWidget* buildGaps();
    void choosePhase(QWidget* anchor);
    void editSummary();

    const pm::Definitions& definitions_;
    std::shared_ptr<pm::Record> record_;
    QVBoxLayout* column_ = nullptr;
};
