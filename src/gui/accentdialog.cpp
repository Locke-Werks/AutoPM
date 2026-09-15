#include "accentdialog.h"

#include "chrome.h"
#include "theme.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>
#include <functional>

namespace {

// The colour itself, at the size it will actually be seen.
class Chip : public QWidget {
public:
    explicit Chip(const QColor& colour, QWidget* parent) : QWidget(parent), colour_(colour) {
        setFixedSize(46, 22);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        painter.setBrush(colour_);
        painter.drawRoundedRect(QRectF(rect()), 3, 3);
    }

private:
    QColor colour_;
};

// One family: its colour, its house name, and what the house says it is for.
// No signal of its own, because a class declared inside a .cpp never reaches
// moc; a callback does the same job with less ceremony.
class Swatch : public chrome::Card {
public:
    Swatch(const theme::Family& family, QWidget* parent)
        : chrome::Card(parent), colour_(family.colour) {
        setHoverable(true);
        setCursor(Qt::PointingHandCursor);
        body()->setContentsMargins(17, 12, 17, 12);
        body()->setSpacing(6);

        auto* head = new QHBoxLayout;
        head->setSpacing(12);
        head->addWidget(new Chip(family.colour, this));
        head->addWidget(chrome::label(family.name, 11, family.colour));
        head->addStretch(1);

        auto* hex = chrome::bodyText(family.colour.name().toUpper(), theme::textFaint(), 12);
        hex->setFont(theme::monoFont(11));
        head->addWidget(hex);

        body()->addLayout(head);
        body()->addWidget(chrome::bodyText(family.meaning, theme::textSecondary(), 13));
    }

    QColor colour() const { return colour_; }
    std::function<void()> onPick;

protected:
    void mouseReleaseEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton && onPick) onPick();
    }

private:
    QColor colour_;
};

} // namespace

AccentDialog::AccentDialog(const QString& projectName, const QColor& current, QWidget* parent)
    : QDialog(parent), chosen_(current) {
    setWindowTitle("Project colour");
    setFixedWidth(580);
    if (parent) setStyleSheet(parent->window()->styleSheet());

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(26, 22, 26, 18);
    layout->setSpacing(10);

    layout->addWidget(new chrome::Eyebrow(projectName, this));
    layout->addWidget(new chrome::Heading("What colour is this project?", 26, this));
    layout->addWidget(chrome::bodyText(
        "The accent lights the room: the rail, focus, the bloom, primary buttons. It never "
        "renders a verdict about an entry. Blue, magenta and ember do that, and those do not "
        "change with the project.",
        theme::textSecondary(), 13));

    auto* swatches = new QList<Swatch*>;   // owned by the dialog through the children
    for (const theme::Family& family : theme::families()) {
        auto* swatch = new Swatch(family, this);
        swatch->onPick = [this, swatch, swatches] {
            chosen_ = swatch->colour();
            for (Swatch* other : *swatches) other->setHighlighted(other == swatch);
        };
        swatch->setHighlighted(family.colour.rgb() == current.rgb());
        swatches->append(swatch);
        layout->addWidget(swatch);
    }
    connect(this, &QObject::destroyed, [swatches] { delete swatches; });

    layout->addWidget(chrome::bodyText(
        "Crimson is not offered. It is reserved across the house for the body's alarm, one "
        "honest meaning per app, and a project's colour is not that.",
        theme::textFaint(), 12));

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    for (QAbstractButton* button : buttons->buttons()) {
        button->setCursor(Qt::PointingHandCursor);
        QFont face = theme::labelFont(11);
        face.setCapitalization(QFont::MixedCase);
        button->setFont(face);
        button->setText(button->text().toUpper());
    }
    buttons->button(QDialogButtonBox::Ok)->setProperty("house", "primary");
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}
