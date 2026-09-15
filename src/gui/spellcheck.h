// Spell checking, borrowed from Windows.
//
// Qt Widgets has none. Rather than ship a dictionary, this asks the spell
// checker the operating system already has (ISpellChecker, Windows 8 and
// later), which means it uses the user's own language and their own added
// words rather than a second opinion that disagrees with every other
// application on the machine.
//
// Where that is unavailable the checker reports nothing misspelled, so the
// text boxes behave exactly as they did before.
#pragma once

#include <QString>
#include <QStringList>
#include <QSyntaxHighlighter>
#include <QVector>

namespace spell {

struct Span {
    int start = 0;
    int length = 0;
};

// Started once at launch. Safe to call when it failed: everything below then
// answers as though nothing is ever misspelled.
void initialise();
void shutdown();
bool available();

QVector<Span> mistakesIn(const QString& text);
QStringList suggestionsFor(const QString& word);
void ignoreWord(const QString& word);      // this session only
void addToDictionary(const QString& word); // the user's own dictionary

} // namespace spell

// Underlines what the operating system does not recognise. Attached to the
// document of a text box; does nothing at all when the checker is unavailable.
class SpellHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit SpellHighlighter(QTextDocument* document);

protected:
    void highlightBlock(const QString& text) override;
};
