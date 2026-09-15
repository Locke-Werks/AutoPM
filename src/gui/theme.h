// The house material language, as a desktop application rather than a page.
//
// Same rules as the Locke Werks product stylesheet: a wet near-black ground,
// violet-warmed text, hairlines that are never white, and one accent per
// project spent on emission rather than on fill. Corners are soft on cards and
// square on labels; the display face is a serif and every label is small,
// uppercase and letterspaced.
#pragma once

#include "core/definition.h"

#include <QColor>
#include <QFont>
#include <QList>
#include <QString>

class QWidget;

namespace theme {

// ── The house spectrum ───────────────────────────────────────────────────
//
// One continuous emissive ramp, blue through violet and magenta, plus two
// lights that sit off it. AutoPM spends hue on one axis, the same way every
// app in the house does: here it is how well-founded a claim is. Cool means
// somebody said it and you can quote them; hot means nobody has answered.
//
// Crimson (#FF1E3C) is reserved family-wide for the body's alarm. AutoPM has
// no business with it and does not use it anywhere, including as a project
// accent.

inline QColor houseBlue()    { return QColor("#3D7DFF"); }   // neither warm nor urgent
inline QColor houseViolet()  { return QColor("#B05CF6"); }   // structural
inline QColor houseMagenta() { return QColor("#FF2D95"); }   // loudest that is not reserved
inline QColor ember()        { return QColor("#FF5A2A"); }   // taken here as a signal
inline QColor cyan()         { return QColor("#2EE8FF"); }

// A project accent is chosen from these. Name, colour, and what the house
// says the colour is for.
struct Family {
    QString name;
    QColor colour;
    QString meaning;
};
const QList<Family>& families();

// ── Ground and structure ─────────────────────────────────────────────────
//
// Four surfaces, stepping by lightness only at a fixed hue and saturation. A
// surface that shifts hue as it lifts reads as a different material rather
// than a nearer one. Ground is the house violet-black.

inline QColor voidBg()  { return QColor("#07050E"); }   // the room
inline QColor raised()  { return QColor("#100B20"); }   // rail, help panel, cards
inline QColor surface() { return raised(); }
inline QColor input()   { return QColor("#17112F"); }   // inputs and hover
inline QColor pressed() { return QColor("#1F163E"); }   // pressed and selected
inline QColor sunken()  { return input(); }

// Hairlines are tinted, never grey: house violet at 16% and 34%. That is what
// makes a hairline part of the room instead of a border drawn on top of it.
inline QColor hairline()       { return QColor(176, 92, 246, 41); }
inline QColor hairlineStrong() { return QColor(176, 92, 246, 87); }

// Ink, measured rather than asserted, against the ground above.
inline QColor textPrimary()   { return QColor("#F7EFFC"); }   // 18.0:1
inline QColor textBody()      { return QColor("#D9C8E8"); }   // 12.9:1
inline QColor textSecondary() { return QColor("#B9A3CF"); }   //  8.9:1
inline QColor textLabel()     { return QColor("#A98CC4"); }   //  7.0:1
inline QColor textFaint()     { return QColor("#8D70A4"); }   //  4.8:1, the floor

// The signal set. Fixed, and never chosen: these are the one axis.
inline QColor ok()     { return houseBlue(); }      // solid, reached, specified
inline QColor warn()   { return houseMagenta(); }   // unresolved, proposed, watching
inline QColor danger() { return ember(); }          // broken, missed, blocked

// One accent per project, chosen from families(). It lights the room: nav,
// focus, the bloom, primary buttons. It never renders a verdict about an
// entry, which is what the signal set above is for.
QColor accent();
void setAccent(const QColor& colour);

// Accent at a given alpha, for the tinted fills and glows.
QColor accentAt(int alpha);
QColor mix(const QColor& a, const QColor& b, qreal amountOfA);

QColor toneColour(pm::Tone tone);

// The three house faces, with system fallbacks when the real files are not
// installed. Drop the .ttf files into assets/fonts to get the real type.
void loadFonts(const QString& assetsDir);
bool houseFontsAvailable();
QString resolvedFaces();   // what the three roles actually landed on

QFont labelFont(int pixelSize = 11, bool heavy = true);   // uppercase, letterspaced
QFont serifFont(int pixelSize = 22);                      // display headings
QFont bodyFont(int pixelSize = 14);
QFont monoFont(int pixelSize = 12);

QString styleSheet();

// Windows draws its own title bar; ask it for the dark one so the frame does
// not sit on the app like a white lid.
void applyDarkFrame(QWidget* window);

} // namespace theme
