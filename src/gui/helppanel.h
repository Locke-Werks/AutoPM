// The "why" panel. Focus a field and this explains where it comes from in the
// standard, why a PM fills it in, what breaks when nobody does, and what a good
// answer looks like.
//
// This panel is the study guide. Everything it shows comes from the definition
// files, not from code.
#pragma once

#include "core/definition.h"

#include <QWidget>

class QLabel;
class QVBoxLayout;

class HelpPanel : public QWidget {
    Q_OBJECT
public:
    explicit HelpPanel(QWidget* parent = nullptr);

    void showScreen(const pm::Screen& screen);
    void showField(const pm::Screen& screen, const pm::Field& field);
    void showColumn(const pm::Screen& screen, const pm::Field& field, const pm::Column& column);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void clearBody();
    void addSection(const QString& label, const QString& text, const QColor& colour);

    QVBoxLayout* body_ = nullptr;
    QLabel* eyebrow_ = nullptr;
    QLabel* title_ = nullptr;
    QWidget* badgeRow_ = nullptr;
};
