#include "chrome.h"

#include <QAction>
#include <QEnterEvent>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QLinearGradient>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QVBoxLayout>

namespace chrome {

// ── Card ─────────────────────────────────────────────────────────────────

Card::Card(QWidget* parent) : QFrame(parent) {
    body_ = new QVBoxLayout(this);
    body_->setContentsMargins(22, 20, 22, 20);
    body_->setSpacing(10);
    setAttribute(Qt::WA_StyledBackground, false);
}

void Card::setHighlighted(bool on) {
    if (highlighted_ == on) return;
    highlighted_ = on;
    update();
}

void Card::setHoverable(bool on) {
    hoverable_ = on;
    setAttribute(Qt::WA_Hover, on);
}

void Card::setOnClick(std::function<void()> handler) {
    onClick_ = std::move(handler);
    const bool active = static_cast<bool>(onClick_);
    setHoverable(active);
    setCursor(active ? Qt::PointingHandCursor : Qt::ArrowCursor);
    // The dashboard is the first thing a keyboard lands on, so a card that does
    // something has to be reachable without a mouse.
    setFocusPolicy(active ? Qt::StrongFocus : Qt::NoFocus);
}

void Card::mousePressEvent(QMouseEvent* event) {
    if (onClick_ && event->button() == Qt::LeftButton) {
        pressed_ = true;
        update();
    }
    QFrame::mousePressEvent(event);
}

void Card::mouseReleaseEvent(QMouseEvent* event) {
    // Released outside the card is a cancelled click, the same as any button.
    const bool fire = pressed_ && onClick_ && event->button() == Qt::LeftButton &&
                      rect().contains(event->position().toPoint());
    pressed_ = false;
    update();
    QFrame::mouseReleaseEvent(event);
    // Last, because the handler usually replaces the page this card is on.
    if (fire) onClick_();
}

void Card::keyPressEvent(QKeyEvent* event) {
    if (onClick_ && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
                     event->key() == Qt::Key_Space)) {
        event->accept();
        onClick_();
        return;
    }
    QFrame::keyPressEvent(event);
}

void Card::enterEvent(QEnterEvent* event) {
    hovered_ = true;
    if (hoverable_) update();
    QFrame::enterEvent(event);
}

void Card::leaveEvent(QEvent* event) {
    hovered_ = false;
    if (hoverable_) update();
    QFrame::leaveEvent(event);
}

void Card::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF outer = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    const qreal radius = 12;

    QLinearGradient fill(outer.topLeft(), outer.bottomLeft());
    fill.setColorAt(0, theme::raised());
    fill.setColorAt(1, theme::voidBg());
    painter.setPen(Qt::NoPen);
    painter.setBrush(fill);
    painter.drawRoundedRect(outer, radius, radius);

    QColor edge = theme::hairline();
    if (highlighted_) edge = theme::accentAt(150);
    // Pressed and focused both read as the full accent, which is the one focus
    // rule the rest of the app follows. A keyboard user has to be able to see
    // where they are.
    else if (onClick_ && (pressed_ || hasFocus())) edge = theme::accentAt(150);
    else if (hoverable_ && hovered_) edge = theme::accentAt(85);
    painter.setPen(QPen(edge, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(outer, radius, radius);

    // The echo three pixels inside the stroke: the suite's traced edge.
    painter.setPen(QPen(theme::accentAt(highlighted_ ? 80 : 56), 1));
    painter.drawRoundedRect(outer.adjusted(3, 3, -3, -3), radius - 3, radius - 3);
}

// ── Type ─────────────────────────────────────────────────────────────────

Eyebrow::Eyebrow(const QString& text, QWidget* parent) : QLabel(parent) {
    setFont(theme::labelFont(11));
    QPalette palette = this->palette();
    palette.setColor(QPalette::WindowText, theme::accent());
    setPalette(palette);
    setEyebrowText(text);
}

void Eyebrow::setEyebrowText(const QString& text) {
    // The house eyebrow opens with a dimmed comment marker.
    setText(text.isEmpty() ? QString() : QString("// ") + text);
}

Heading::Heading(const QString& text, int pixelSize, QWidget* parent) : QLabel(text, parent) {
    setFont(theme::serifFont(pixelSize));
    QPalette palette = this->palette();
    palette.setColor(QPalette::WindowText, theme::textPrimary());
    setPalette(palette);
    setWordWrap(true);
}

// ── Badge ────────────────────────────────────────────────────────────────

Badge::Badge(const QString& text, QWidget* parent) : QLabel(parent) {
    face_ = theme::labelFont(9);
    face_.setCapitalization(QFont::MixedCase);   // the text is upper-cased instead
    setFont(face_);
    setAlignment(Qt::AlignCenter);
    setBadgeText(text);
}

void Badge::setBadgeText(const QString& text) {
    setText(text.toUpper());
    const QFontMetrics metrics(face_);
    setFixedHeight(20);
    // Fixed, not minimum: a layout under pressure would otherwise shave a
    // badge down to three letters and an ellipsis. Anything past the cap
    // elides inside the badge instead.
    setFixedWidth(qMin(metrics.horizontalAdvance(text.toUpper()) + 22, 236));
    update();
}

void Badge::setTone(pm::Tone tone) { setColour(theme::toneColour(tone)); }

void Badge::setColour(const QColor& colour) {
    colour_ = colour;
    update();
}

void Badge::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QColor border = colour_;
    border.setAlpha(96);
    QColor fill = colour_;
    fill.setAlpha(20);

    const QRectF box = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    painter.setPen(QPen(border, 1));
    painter.setBrush(fill);
    painter.drawRoundedRect(box, 2, 2);

    painter.setPen(colour_);
    painter.setFont(face_);
    const QFontMetrics metrics(face_);
    painter.drawText(box, Qt::AlignCenter,
                     metrics.elidedText(text(), Qt::ElideRight,
                                        static_cast<int>(box.width()) - 10));
}

// ── Provenance ───────────────────────────────────────────────────────────

QString provenanceName(pm::Provenance provenance) {
    return QString::fromStdString(pm::provenanceToString(provenance));
}

QColor provenanceColour(pm::Provenance provenance) {
    switch (provenance) {
        case pm::Provenance::Specified:  return theme::houseBlue();
        case pm::Provenance::Agreed:     return theme::houseViolet();
        case pm::Provenance::Unobjected: return theme::ember();
        case pm::Provenance::Untagged:   break;
    }
    return theme::textFaint();
}

ProvenanceControl::ProvenanceControl(QWidget* parent) : QWidget(parent) {
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(8);

    tag_ = new Badge("untagged", this);
    tag_->setCursor(Qt::PointingHandCursor);
    tag_->setToolTip("Where this entry came from. Only specified and agreed count as decisions.");
    tag_->installEventFilter(this);

    evidenceButton_ = button("+ evidence", "quiet", this);
    evidenceButton_->setToolTip("The quote and date this entry rests on.");

    row->addWidget(tag_);
    row->addWidget(evidenceButton_);
    row->addStretch(1);

    connect(evidenceButton_, &QPushButton::clicked, this, &ProvenanceControl::editEvidence);
    refresh();
}

void ProvenanceControl::set(pm::Provenance provenance, const QString& evidence) {
    provenance_ = provenance;
    evidence_ = evidence;
    refresh();
}

void ProvenanceControl::refresh() {
    tag_->setBadgeText(provenanceName(provenance_));
    tag_->setColour(provenanceColour(provenance_));

    if (evidence_.trimmed().isEmpty()) {
        evidenceButton_->setText("+ EVIDENCE");
        evidenceButton_->setToolTip("The quote and date this entry rests on.");
    } else {
        QString shown = evidence_.simplified();
        if (shown.size() > 58) shown = shown.left(55) + "...";
        evidenceButton_->setText(shown);
        evidenceButton_->setToolTip(evidence_);
    }
    evidenceButton_->setFont(theme::bodyFont(11));
}

bool ProvenanceControl::eventFilter(QObject* watched, QEvent* event) {
    if (watched == tag_ && event->type() == QEvent::MouseButtonRelease) {
        openMenu();
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void ProvenanceControl::openMenu() {
    QMenu menu(this);
    const pm::Provenance states[] = {pm::Provenance::Untagged, pm::Provenance::Specified,
                                     pm::Provenance::Agreed, pm::Provenance::Unobjected};
    const char* help[] = {
        "untagged — nothing claimed",
        "specified — they said it (quote it)",
        "agreed — they approved a summary",
        "unobjected — proposed, no reply. Not a decision.",
    };
    for (int i = 0; i < 4; ++i) {
        QAction* action = menu.addAction(QString::fromUtf8(help[i]));
        action->setCheckable(true);
        action->setChecked(provenance_ == states[i]);
        pm::Provenance chosen = states[i];
        connect(action, &QAction::triggered, this, [this, chosen] {
            provenance_ = chosen;
            refresh();
            emit changed();
        });
    }
    menu.exec(tag_->mapToGlobal(QPoint(0, tag_->height() + 4)));
}

void ProvenanceControl::editEvidence() {
    bool accepted = false;
    const QString text = QInputDialog::getMultiLineText(
        this, "Evidence",
        "The quote and date this entry rests on.\n"
        "An entry with no evidence is not a decision, whatever the tag says.",
        evidence_, &accepted);
    if (!accepted) return;
    evidence_ = text;
    refresh();
    emit changed();
}

// ── Small helpers ────────────────────────────────────────────────────────

QPushButton* button(const QString& text, const QString& kind, QWidget* parent) {
    auto* b = new QPushButton(text.toUpper(), parent);
    b->setCursor(Qt::PointingHandCursor);
    QFont face = theme::labelFont(kind == "quiet" ? 10 : 11);
    face.setCapitalization(QFont::MixedCase);
    b->setFont(face);
    if (!kind.isEmpty()) b->setProperty("house", kind);
    return b;
}

QLabel* label(const QString& text, int pixelSize, const QColor& colour) {
    auto* l = new QLabel(text);
    l->setFont(theme::labelFont(pixelSize));
    QPalette palette = l->palette();
    palette.setColor(QPalette::WindowText, colour);
    l->setPalette(palette);
    return l;
}

QLabel* bodyText(const QString& text, const QColor& colour, int pixelSize) {
    auto* l = new QLabel(text);
    l->setFont(theme::bodyFont(pixelSize));
    l->setWordWrap(true);
    l->setTextInteractionFlags(Qt::TextSelectableByMouse);
    QPalette palette = l->palette();
    palette.setColor(QPalette::WindowText, colour);
    l->setPalette(palette);
    return l;
}

QString hairlineCss(int alpha) {
    const QColor c = theme::hairline();
    return QString("rgba(%1,%2,%3,%4)").arg(c.red()).arg(c.green()).arg(c.blue()).arg(alpha);
}

QFrame* rule(QWidget* parent) {
    auto* line = new QFrame(parent);
    line->setFixedHeight(1);
    line->setStyleSheet("background: " + hairlineCss() + ";");
    return line;
}

void paintBloom(QPainter& painter, const QRect& rect, qreal strength) {
    QRadialGradient bloom(rect.center().x(), rect.top() - rect.height() * 0.5,
                          rect.width() * 0.44);
    QColor centre = theme::accent();
    centre.setAlphaF(qBound(0.0, 0.085 * strength, 1.0));
    bloom.setColorAt(0, centre);
    centre.setAlpha(0);
    bloom.setColorAt(1, centre);
    painter.fillRect(rect, bloom);
}

} // namespace chrome
