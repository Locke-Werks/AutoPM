#include "theme.h"

#include <QDir>
#include <QFontDatabase>
#include <QWidget>

#ifdef _WIN32
#include <windows.h>
#include <dwmapi.h>
#endif

namespace theme {
namespace {

QColor g_accent("#ff2d95");
QString g_label = "Segoe UI";
QString g_serif = "Georgia";
QString g_body  = "Segoe UI";
QString g_mono  = "Cascadia Mono";
bool g_houseFonts = false;

QString rgba(const QColor& c) {
    return QString("rgba(%1,%2,%3,%4)").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}

// Picks the first family that is actually installed.
QString firstAvailable(const QStringList& candidates, const QString& fallback) {
    const QStringList families = QFontDatabase::families();
    for (const QString& candidate : candidates) {
        for (const QString& family : families) {
            if (family.compare(candidate, Qt::CaseInsensitive) == 0) return family;
        }
    }
    return fallback;
}

} // namespace

QColor accent() { return g_accent; }

void setAccent(const QColor& colour) {
    if (colour.isValid()) g_accent = colour;
}

QColor accentAt(int alpha) {
    QColor c = g_accent;
    c.setAlpha(alpha);
    return c;
}

QColor mix(const QColor& a, const QColor& b, qreal amountOfA) {
    amountOfA = qBound(0.0, amountOfA, 1.0);
    return QColor::fromRgbF(a.redF()   * amountOfA + b.redF()   * (1 - amountOfA),
                            a.greenF() * amountOfA + b.greenF() * (1 - amountOfA),
                            a.blueF()  * amountOfA + b.blueF()  * (1 - amountOfA));
}

QColor toneColour(pm::Tone tone) {
    switch (tone) {
        case pm::Tone::Accent: return accent();
        case pm::Tone::Ok:     return ok();
        case pm::Tone::Warn:   return warn();
        case pm::Tone::Danger: return danger();
        case pm::Tone::Faint:  return textFaint();
        case pm::Tone::Neutral: break;
    }
    return textSecondary();
}

void loadFonts(const QString& assetsDir) {
    QDir fonts(assetsDir + "/fonts");
    if (fonts.exists()) {
        const QStringList files = fonts.entryList({"*.ttf", "*.otf"}, QDir::Files);
        for (const QString& file : files)
            QFontDatabase::addApplicationFont(fonts.filePath(file));
    }

    g_label = firstAvailable({"Chakra Petch", "Segoe UI Semibold"}, "Segoe UI");
    g_serif = firstAvailable({"Instrument Serif", "Constantia", "Georgia"}, "Georgia");
    g_body  = firstAvailable({"Outfit", "Segoe UI Variable Text", "Segoe UI"}, "Segoe UI");
    g_mono  = firstAvailable({"Cascadia Mono", "Consolas"}, "Consolas");
    g_houseFonts = (g_label == "Chakra Petch" && g_body == "Outfit");
}

bool houseFontsAvailable() { return g_houseFonts; }

QString resolvedFaces() {
    return QString("label: %1\nserif: %2\nbody:  %3\nmono:  %4\nhouse faces: %5\n")
        .arg(g_label, g_serif, g_body, g_mono, g_houseFonts ? "yes" : "no (falling back)");
}

QFont labelFont(int pixelSize, bool heavy) {
    QFont font(g_label);
    font.setPixelSize(pixelSize);
    font.setWeight(heavy ? QFont::DemiBold : QFont::Medium);
    font.setCapitalization(QFont::AllUppercase);
    // The house label is spaced out far enough to read as a label rather than
    // as small text.
    font.setLetterSpacing(QFont::AbsoluteSpacing, pixelSize * 0.16);
    return font;
}

QFont serifFont(int pixelSize) {
    QFont font(g_serif);
    font.setPixelSize(pixelSize);
    font.setWeight(QFont::Normal);
    font.setLetterSpacing(QFont::AbsoluteSpacing, -0.2);
    return font;
}

QFont bodyFont(int pixelSize) {
    QFont font(g_body);
    font.setPixelSize(pixelSize);
    return font;
}

QFont monoFont(int pixelSize) {
    QFont font(g_mono);
    font.setPixelSize(pixelSize);
    return font;
}

QString styleSheet() {
    const QString accentHex = g_accent.name();
    const QString hair = rgba(hairline());
    const QString hairStrong = rgba(hairlineStrong());
    const QString accent12 = rgba(accentAt(30));
    const QString accent20 = rgba(accentAt(52));
    const QString accent45 = rgba(accentAt(115));

    return QString(R"(
QWidget {
    background: transparent;
    color: %{body};
}
QMainWindow, QDialog {
    background: %{void};
}
QToolTip {
    background: %{raised};
    color: %{primary};
    border: 1px solid %{hairStrong};
    padding: 6px 9px;
}

/* ── Inputs ──────────────────────────────────────────────────────── */

QLineEdit, QTextEdit, QPlainTextEdit, QComboBox, QSpinBox, QDateEdit {
    background: %{sunken};
    color: %{body};
    border: 1px solid %{hair};
    border-radius: 4px;
    padding: 7px 10px;
    selection-background-color: %{accent20};
    selection-color: %{primary};
}
QLineEdit:hover, QTextEdit:hover, QPlainTextEdit:hover, QComboBox:hover, QDateEdit:hover {
    border-color: %{hairStrong};
}
QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QComboBox:focus, QDateEdit:focus {
    border-color: %{accent45};
    background: %{raised};
}
QLineEdit:disabled, QTextEdit:disabled, QComboBox:disabled {
    color: %{faint};
}
QComboBox::drop-down { border: none; width: 22px; }
QComboBox::down-arrow {
    image: none;
    border-left: 4px solid transparent;
    border-right: 4px solid transparent;
    border-top: 5px solid %{label};
    margin-right: 8px;
}
QComboBox QAbstractItemView {
    background: %{raised};
    color: %{body};
    border: 1px solid %{hairStrong};
    selection-background-color: %{accent20};
    selection-color: %{primary};
    padding: 4px;
    outline: none;
}

/* ── Buttons ─────────────────────────────────────────────────────── */

QPushButton {
    background: transparent;
    color: %{secondary};
    border: 1px solid %{hair};
    border-radius: 3px;
    padding: 8px 16px;
}
QPushButton:hover {
    color: %{primary};
    border-color: %{hairStrong};
}
QPushButton:pressed { background: %{accent12}; }
QPushButton:disabled { color: %{faint}; border-color: %{hair}; }
QPushButton[house="primary"] {
    color: %{primary};
    border-color: %{accent45};
    background: %{accent12};
}
QPushButton[house="primary"]:hover {
    background: %{accent20};
    border-color: %{accentHex};
}
QPushButton[house="quiet"] {
    border-color: transparent;
    color: %{label};
    padding: 5px 9px;
}
QPushButton[house="quiet"]:hover { color: %{accentHex}; }
QPushButton[house="danger"]:hover { color: %{danger}; border-color: %{danger}; }

/* ── Scrollbars: a hairline, not a widget ────────────────────────── */

QScrollBar:vertical {
    background: transparent; width: 10px; margin: 0;
}
QScrollBar::handle:vertical {
    background: %{hairStrong}; border-radius: 5px; min-height: 40px;
}
QScrollBar::handle:vertical:hover { background: %{accent45}; }
QScrollBar:horizontal {
    background: transparent; height: 10px; margin: 0;
}
QScrollBar::handle:horizontal {
    background: %{hairStrong}; border-radius: 5px; min-width: 40px;
}
QScrollBar::handle:horizontal:hover { background: %{accent45}; }
QScrollBar::add-line, QScrollBar::sub-line { height: 0; width: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }
QScrollArea { border: none; }

/* ── Tables ──────────────────────────────────────────────────────── */

QTableWidget, QTableView {
    background: %{sunken};
    alternate-background-color: %{surface};
    gridline-color: %{hair};
    border: 1px solid %{hair};
    border-radius: 6px;
    selection-background-color: %{accent20};
    selection-color: %{primary};
    outline: none;
}
QTableWidget::item, QTableView::item { padding: 5px 7px; border: none; }
QHeaderView::section {
    background: %{surface};
    color: %{label};
    border: none;
    border-bottom: 1px solid %{hairStrong};
    border-right: 1px solid %{hair};
    padding: 8px 7px;
}
QTableCornerButton::section { background: %{surface}; border: none; }

/* ── Lists and trees ─────────────────────────────────────────────── */

QListWidget, QTreeWidget, QTreeView {
    background: transparent;
    border: none;
    outline: none;
}
QTreeWidget::item, QTreeView::item { padding: 5px 4px; }
QTreeWidget::item:selected, QTreeView::item:selected {
    background: %{accent12};
    color: %{primary};
}

QSplitter::handle { background: %{hair}; }
QSplitter::handle:horizontal { width: 1px; }
QSplitter::handle:vertical { height: 1px; }

QMenu {
    background: %{raised};
    border: 1px solid %{hairStrong};
    padding: 5px;
}
QMenu::item { padding: 7px 22px 7px 14px; border-radius: 3px; }
QMenu::item:selected { background: %{accent12}; color: %{primary}; }
QMenu::separator { height: 1px; background: %{hair}; margin: 5px 8px; }

QCheckBox { spacing: 8px; }
QCheckBox::indicator {
    width: 15px; height: 15px;
    border: 1px solid %{hairStrong};
    border-radius: 3px;
    background: %{sunken};
}
QCheckBox::indicator:checked { background: %{accentHex}; border-color: %{accentHex}; }

QProgressBar {
    background: %{sunken};
    border: 1px solid %{hair};
    border-radius: 3px;
    height: 6px;
    text-align: center;
}
QProgressBar::chunk { background: %{accentHex}; border-radius: 2px; }
)")
        .replace("%{void}", voidBg().name())
        .replace("%{surface}", surface().name())
        .replace("%{raised}", raised().name())
        .replace("%{sunken}", sunken().name())
        .replace("%{hairStrong}", hairStrong)
        .replace("%{hair}", hair)
        .replace("%{primary}", textPrimary().name())
        .replace("%{body}", textBody().name())
        .replace("%{secondary}", textSecondary().name())
        .replace("%{label}", textLabel().name())
        .replace("%{faint}", textFaint().name())
        .replace("%{danger}", danger().name())
        .replace("%{accentHex}", accentHex)
        .replace("%{accent12}", accent12)
        .replace("%{accent20}", accent20)
        .replace("%{accent45}", accent45)
        .replace("%{labelFace}", g_label)
        .replace("%{bodyFace}", g_body);
}

void applyDarkFrame(QWidget* window) {
#ifdef _WIN32
    if (!window) return;
    HWND handle = reinterpret_cast<HWND>(window->winId());
    BOOL dark = TRUE;
    // 20 is DWMWA_USE_IMMERSIVE_DARK_MODE on current Windows; 19 was the
    // pre-20H1 spelling. Asking for both covers either.
    DwmSetWindowAttribute(handle, 20, &dark, sizeof(dark));
    DwmSetWindowAttribute(handle, 19, &dark, sizeof(dark));
#else
    Q_UNUSED(window);
#endif
}

} // namespace theme
