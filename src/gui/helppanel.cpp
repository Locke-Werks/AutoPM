#include "helppanel.h"

#include "chrome.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {

QString orNothing(const std::string& text) { return QString::fromStdString(text).trimmed(); }

} // namespace

HelpPanel::HelpPanel(QWidget* parent) : QWidget(parent) {
    setMinimumWidth(300);
    setMaximumWidth(400);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* inner = new QWidget;
    body_ = new QVBoxLayout(inner);
    body_->setContentsMargins(24, 26, 22, 30);
    body_->setSpacing(0);

    eyebrow_ = new chrome::Eyebrow("why this field");
    title_ = new chrome::Heading(QString(), 21);
    title_->setWordWrap(true);

    badgeRow_ = new QWidget;
    auto* badges = new QHBoxLayout(badgeRow_);
    badges->setContentsMargins(0, 0, 0, 0);
    badges->setSpacing(6);
    badges->addStretch(1);

    body_->addWidget(eyebrow_);
    body_->addSpacing(9);
    body_->addWidget(title_);
    body_->addSpacing(10);
    body_->addWidget(badgeRow_);
    body_->addSpacing(18);
    body_->addStretch(1);

    scroll->setWidget(inner);
    outer->addWidget(scroll);
}

void HelpPanel::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), theme::surface());
    painter.setPen(QPen(theme::hairline(), 1));
    painter.drawLine(0, 0, 0, height());
}

void HelpPanel::clearBody() {
    // Everything after the fixed header (eyebrow, title, badges) is rebuilt on
    // every focus change.
    while (body_->count() > 6) {
        QLayoutItem* item = body_->takeAt(6);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    body_->addStretch(1);

    auto* badges = qobject_cast<QHBoxLayout*>(badgeRow_->layout());
    while (badges->count() > 1) {
        QLayoutItem* item = badges->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

void HelpPanel::addSection(const QString& label, const QString& text, const QColor& colour) {
    if (text.isEmpty()) return;
    int at = body_->count() - 1;   // before the trailing stretch
    body_->insertWidget(at++, chrome::label(label, 10, colour));
    body_->insertSpacing(at++, 7);
    auto* paragraph = chrome::bodyText(text, theme::textSecondary(), 13);
    body_->insertWidget(at++, paragraph);
    body_->insertSpacing(at++, 20);
}

void HelpPanel::showScreen(const pm::Screen& screen) {
    clearBody();
    eyebrow_->setText(QString("// %1").arg(QString::fromStdString(screen.phase).toUpper()));
    title_->setText(QString::fromStdString(screen.title));

    auto* badges = qobject_cast<QHBoxLayout*>(badgeRow_->layout());
    if (!screen.process.empty()) {
        auto* badge = new chrome::Badge(QString::fromStdString(screen.process));
        badge->setColour(theme::accent());
        badges->insertWidget(0, badge);
    }

    addSection("what this screen is", orNothing(screen.intro), theme::textLabel());
    addSection("where it comes from", orNothing(screen.source), theme::textLabel());
    addSection("how to use this panel",
               "Click into any field and this panel explains it: where it comes from in the "
               "standard, why a PM fills it in, what breaks when it is left blank, and what a "
               "good answer looks like.",
               theme::textLabel());
}

void HelpPanel::showField(const pm::Screen& screen, const pm::Field& field) {
    clearBody();
    eyebrow_->setText(QString("// %1").arg(QString::fromStdString(screen.title).toUpper()));
    title_->setText(QString::fromStdString(field.label));

    auto* badges = qobject_cast<QHBoxLayout*>(badgeRow_->layout());
    const bool isHers = field.origin == "yours";
    auto* origin = new chrome::Badge(isHers ? "your own" : "standard");
    origin->setColour(isHers ? theme::warn() : theme::accent());
    origin->setToolTip(isHers ? "Not in the standard. This field is your own."
                              : "This field comes from the PMBOK Guide.");
    badges->insertWidget(0, origin);

    if (!screen.process.empty()) {
        auto* process = new chrome::Badge(QString::fromStdString(screen.process));
        process->setColour(theme::textLabel());
        badges->insertWidget(1, process);
    }

    addSection("source", orNothing(field.source), theme::textLabel());
    addSection("why a pm fills it in", orNothing(field.why), theme::accent());
    addSection("if left blank", orNothing(field.ifBlank), theme::danger());
    addSection("example", orNothing(field.example), theme::ok());
}

void HelpPanel::showColumn(const pm::Screen& screen, const pm::Field& field,
                           const pm::Column& column) {
    showField(screen, field);
    const QString help = orNothing(column.help);
    QString text = help;
    if (text.isEmpty() && !column.options.empty()) {
        QStringList values;
        for (const pm::Option& option : column.options)
            values << QString::fromStdString(option.value);
        text = "Allowed values: " + values.join(" · ");
    }
    addSection(QString("column: %1").arg(QString::fromStdString(column.label)), text,
               theme::textLabel());
}
