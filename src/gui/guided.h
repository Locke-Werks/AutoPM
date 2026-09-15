// The walkthrough: the tool teaching the job rather than filing the answers.
//
// One question at a time, in lifecycle order. Each step says what the artifact
// is, why a PM produces it, what goes wrong when nobody does, and what a good
// answer looks like — then asks for yours, and tells you what is thin about it
// before moving on.
//
// It never blocks. Being told the charter is not signed yet and choosing to
// carry on anyway is a PM decision; the tool's job is to make sure it was one.
#pragma once

#include "core/coach.h"
#include "core/definition.h"
#include "core/record.h"

#include <QWidget>
#include <memory>
#include <vector>

class QLabel;
class QStackedWidget;
class QVBoxLayout;
class QPushButton;
class QScrollArea;

class GuidedView : public QWidget {
    Q_OBJECT
public:
    GuidedView(const pm::Definitions& definitions, QWidget* parent = nullptr);

    void setRecord(const std::shared_ptr<pm::Record>& record);
    // screenId empty means "start wherever the record actually is".
    void start(const QString& screenId = QString());

signals:
    void finished();          // leave the walkthrough
    void recordEdited();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    // A step is either a screen's opening lesson, one field, or the recap.
    struct Step {
        int screen = 0;
        int field = -1;        // -1 lesson, -2 recap
    };

    void buildSteps(int fromScreen);
    void showStep(int index);
    void goBack();
    void goNext();

    QWidget* buildLesson(const pm::Screen& screen);
    QWidget* buildField(const pm::Screen& screen, const pm::Field& field);
    QWidget* buildRecap(const pm::Screen& screen);
    QWidget* teachingBlock(const pm::Field& field, QWidget* parent);

    const pm::Definitions& definitions_;
    std::shared_ptr<pm::Record> record_;
    std::vector<Step> steps_;
    int at_ = 0;

    QScrollArea* scroll_ = nullptr;
    QWidget* stage_ = nullptr;
    QLabel* counter_ = nullptr;
    QLabel* phase_ = nullptr;
    QWidget* rail_ = nullptr;
    QPushButton* back_ = nullptr;
    QPushButton* next_ = nullptr;
    QPushButton* leave_ = nullptr;
};
