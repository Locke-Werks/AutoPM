// The screens, loaded from the files in definitions/.
//
// Nothing about a field is written in code: its label, its type, its source in
// the standard, why a PM fills it in and what breaks when it is left blank all
// come from the definition file. Adding a screen means adding a file.
#pragma once

#include <string>
#include <vector>

namespace pm {

// Tone drives colour only. A choice with no tone is drawn as plain text.
enum class Tone { Neutral, Accent, Ok, Warn, Danger, Faint };

Tone toneFromString(const std::string& name);

struct Option {
    std::string value;
    Tone tone = Tone::Neutral;
};

struct Column {
    std::string id;
    std::string label;
    std::string type = "line";   // line | text | choice | date | number
    std::string help;
    std::vector<Option> options;
    std::string wip;             // board lanes: "Doing=3, Review=2"
    int width = 1;               // relative column width in the table view
    bool hidden = false;         // carried in the record, not shown in the grid
};

// A rule the guided walkthrough checks an answer against, with the sentence it
// says when the answer does not meet it. These are coaching, never blocking:
// a PM can always move on, they just get told what is thin about the answer.
struct Check {
    enum class Kind { Filled, MinWords, MinRows, EveryRowHas, MentionsDate, MentionsAny };
    Kind kind = Kind::Filled;
    int number = 0;            // MinWords, MinRows
    std::string argument;      // EveryRowHas: column id. MentionsAny: comma list
    std::string message;
};

// One input on a screen. `type` decides the editor; `view` decides whether the
// GUI draws something richer than a grid for a table.
struct Field {
    std::string id;
    std::string label;
    std::string type = "text";   // line | text | choice | date | table
    std::string origin = "standard";  // standard | yours
    std::string view;            // board | tree | timeline | matrix (tables only)
    std::string source;
    std::string why;
    std::string ifBlank;
    std::string example;
    std::string placeholder;
    // The question the walkthrough actually asks. Without one it falls back to
    // the label, which reads like a form rather than like being taught.
    std::string prompt;
    std::vector<Option> options;
    std::vector<Column> columns;
    std::vector<Check> checks;

    // Board view: which column holds the lane, and which holds the card title.
    std::string groupBy;
    std::string titleColumn;
    std::string subtitleColumn;
    std::string toneColumn;
    // Tree view: the column holding the outline number (1, 1.1, 1.1.2).
    std::string outlineColumn;
    // Timeline view: start and finish columns.
    std::string startColumn;
    std::string finishColumn;
    std::string progressColumn;
    // Matrix view: the two axis columns.
    std::string rowsColumn;
    std::string colsColumn;
    // A view that operates on another field's rows names it here, as
    // "screen.field". Sprint planning moves the board's cards about.
    std::string reads;

    bool isTable() const { return type == "table"; }
    const Column* column(const std::string& id) const;
};

struct Screen {
    std::string id;
    std::string title;
    std::string phase;      // Initiating | Planning | Executing | ...
    std::string process;    // "4.1 Develop Project Charter"
    std::string intro;
    std::string source;
    // The teaching half. `teaches` is what you should be able to explain when
    // you finish; `before` is what has to be true before you start, and why;
    // `after` is what the screen sets up next.
    std::string teaches;
    std::string before;
    std::string after;
    std::vector<std::string> prerequisites;   // screen ids that should come first
    int order = 0;
    std::vector<Field> fields;

    const Field* field(const std::string& id) const;
};

// Every screen found in a definitions directory, sorted by `order`.
class Definitions {
public:
    bool loadDirectory(const std::string& path, std::string& error);

    const std::vector<Screen>& screens() const { return screens_; }
    const Screen* screen(const std::string& id) const;
    // "board.cards" -> that field, for a view that works on another screen's rows.
    const Field* fieldByKey(const std::string& key) const;
    std::vector<std::string> phases() const;   // in lifecycle order, as found

private:
    std::vector<Screen> screens_;
};

} // namespace pm
