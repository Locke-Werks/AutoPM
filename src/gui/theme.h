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
#include <QString>

class QWidget;

namespace theme {

// Ground and structure. These never change with the project accent.
inline QColor voidBg()    { return QColor("#07030e"); }
inline QColor surface()   { return QColor("#0d0518"); }
inline QColor raised()    { return QColor("#150a24"); }
inline QColor sunken()    { return QColor("#0a0414"); }
inline QColor hairline()  { return QColor(190, 120, 255, 33); }
inline QColor hairlineStrong() { return QColor(190, 120, 255, 66); }

inline QColor textPrimary()   { return QColor("#f7effc"); }
inline QColor textBody()      { return QColor("#d9c8e8"); }
inline QColor textSecondary() { return QColor("#b9a3cf"); }
inline QColor textLabel()     { return QColor("#a98cc4"); }
inline QColor textFaint()     { return QColor("#7a6690"); }

inline QColor ok()     { return QColor("#8fe3b0"); }
inline QColor warn()   { return QColor("#ffc93c"); }
inline QColor danger() { return QColor("#ff7d94"); }

// One accent per project. Set once when a record is opened.
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
