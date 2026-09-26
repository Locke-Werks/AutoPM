// The house components, as widgets: the traced-edge card, the accent eyebrow,
// the serif heading, the small uppercase badge, and the provenance control that
// every entry in a record carries.
#pragma once

#include "core/definition.h"
#include "core/record.h"
#include "theme.h"

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QString>

#include <functional>

class QVBoxLayout;

namespace chrome {

// A card with the suite's traced edge: the outer stroke plus an accent echo
// three pixels inside it.
class Card : public QFrame {
    Q_OBJECT
public:
    explicit Card(QWidget* parent = nullptr);

    void setHighlighted(bool on);
    void setHoverable(bool on);

    // A card that acts on a click. Handing it a handler is what makes it a
    // control: hoverable, cursor-marked, reachable by Tab and answerable to
    // Enter or Space. A card without one stays inert, which is most of them.
    void setOnClick(std::function<void()> handler);

    QVBoxLayout* body() const { return body_; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QVBoxLayout* body_ = nullptr;
    bool highlighted_ = false;
    bool hoverable_ = false;
    bool hovered_ = false;
    bool pressed_ = false;
    std::function<void()> onClick_;
};

// "// INITIATING · 4.1 DEVELOP PROJECT CHARTER"
class Eyebrow : public QLabel {
    Q_OBJECT
public:
    explicit Eyebrow(const QString& text, QWidget* parent = nullptr);
    void setEyebrowText(const QString& text);
};

class Heading : public QLabel {
    Q_OBJECT
public:
    explicit Heading(const QString& text, int pixelSize = 26, QWidget* parent = nullptr);
};

// Small uppercase pill. Tone drives the colour; there is no filled variant,
// because the accent is spent on emission rather than on fill.
class Badge : public QLabel {
    Q_OBJECT
public:
    explicit Badge(const QString& text, QWidget* parent = nullptr);
    void setTone(pm::Tone tone);
    void setColour(const QColor& colour);
    void setBadgeText(const QString& text);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QColor colour_ = theme::textLabel();
    QFont face_;
};

// The four-state source tag plus its evidence line, on every field and row.
class ProvenanceControl : public QWidget {
    Q_OBJECT
public:
    explicit ProvenanceControl(QWidget* parent = nullptr);

    void set(pm::Provenance provenance, const QString& evidence);
    pm::Provenance provenance() const { return provenance_; }
    QString evidence() const { return evidence_; }

signals:
    void changed();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void openMenu();
    void editEvidence();
    void refresh();

    pm::Provenance provenance_ = pm::Provenance::Untagged;
    QString evidence_;
    Badge* tag_ = nullptr;
    QPushButton* evidenceButton_ = nullptr;
};

QPushButton* button(const QString& text, const QString& kind = QString(),
                    QWidget* parent = nullptr);
QLabel* label(const QString& text, int pixelSize = 11, const QColor& colour = theme::textLabel());
QLabel* bodyText(const QString& text, const QColor& colour = theme::textSecondary(),
                 int pixelSize = 14);
QFrame* rule(QWidget* parent = nullptr);

// Makes a widget open something on a single left click, with the cursor to
// say so. Selectable text inside it would take the click for itself, so its
// labels stop being selectable: the thing the click opens is where the text
// gets edited.
void onClick(QWidget* widget, std::function<void()> handler);
// The label half of that on its own, for a Card that already handles clicks.
void passClicksThrough(QWidget* widget);

// The hairline as a style sheet colour, so widgets that need it in a sheet do
// not restate the numbers.
QString hairlineCss(int alpha = 41);

QString provenanceName(pm::Provenance provenance);
QColor provenanceColour(pm::Provenance provenance);

// A soft accent bloom, painted behind the top of a screen. It belongs to the
// room, not to any object in it.
void paintBloom(QPainter& painter, const QRect& rect, qreal strength = 1.0);

} // namespace chrome
