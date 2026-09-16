// The teaching half of the tool.
//
// A record can be complete and still be bad PM work: milestones with no dates,
// risks with no owner, a decision log where everything is "unobjected". The
// coach reads an answer against the rules its definition file declares and
// says what is thin about it.
//
// Nothing here blocks. A PM can always move on; they just get told what they
// are moving on from.
#pragma once

#include "definition.h"
#include "record.h"

#include <string>
#include <vector>

namespace pm {

// "words>=8", "rows>=3", "column:target", "date", "filled",
// "mentions:because,so that", "covers:charter.success_criteria"
bool parseCheck(const std::string& rule, const std::string& message, Check& out);

struct Note {
    std::string fieldId;
    std::string message;
};

// The checks this answer fails. Empty means the answer holds up.
//
// A covers: check has to see the field it is meant to be covering, which lives
// elsewhere in the record, so callers holding one pass it. Callers that do not
// get the answer they always did: the check stays quiet rather than failing an
// answer it cannot see.
std::vector<std::string> reviewField(const Field& field, const Entry* entry,
                                     const Record* record = nullptr);

// Every note across a screen, in field order.
std::vector<Note> reviewScreen(const Screen& screen, const Record& record);

struct Progress {
    int fields = 0;
    int answered = 0;
    int notes = 0;
    bool complete() const { return fields > 0 && answered == fields; }
    bool clean() const { return complete() && notes == 0; }
};

Progress progressOf(const Screen& screen, const Record& record);

// Screens named in `prerequisites` that are not complete yet. The walkthrough
// warns about these rather than locking them, because the order is the lesson,
// not a rule the software gets to enforce.
std::vector<std::string> unmetPrerequisites(const Screen& screen, const Definitions& definitions,
                                            const Record& record);

// The next screen worth working on: the first incomplete one in lifecycle
// order. Empty when the record is finished.
std::string nextScreen(const Definitions& definitions, const Record& record);

} // namespace pm
