# AutoPM project management review

A PM-domain review of `Locke-Werks/AutoPM` at commit `cc975e4`, covering PR #1
in full. Not the security review: R9 and card C16 are still open and nothing
here touches them.

13 findings: 3 material, 6 worth doing, 4 small. Plus a coverage table against
the ten knowledge areas and a list of what to leave alone.

Method: read all 13 files in `definitions/` and the core that consumes them,
mapped every field against the artifact contents in the PMBOK Guide 6th edition,
and built the project with MSVC 2022 and Qt 6.8.3 to run its own test suite
(231 checks, 0 failed) and confirm the view code behind each `view:` key. The
judgement is against `01-charter.md` §3, `02-handoff-m2.md` §8 and
`03-handback-m2.md`, as the charter asks. Nothing on any host was changed.

## Before you start

Two things about the standard changed while this was being built, and they change
how the rest of this review reads.

**The PMBOK Guide is on its 8th edition.** PMI released it on 2025-11-13 and put
it on sale 2026-01-13. It reintroduces process groups, as "focus areas," and
carries about 40 processes across seven performance domains under six
principles. The 6th edition this product cites was retired in November 2022.

**The PMP exam changed on 2026-07-09.** The previous version retired 2026-07-08,
so only the new one is offered now. Its Business Environment domain went from 8%
to 26% of the exam.

Finding 1 is what that costs the product. Finding 2 is the gap you can already
see without it, and the exam reweighting is why it is now the most valuable
content change in the tool rather than a completeness item. Everything else is
edition-independent.

Sources for both are listed at the end.

## The short answer

Against `02-handoff-m2.md` §8 it passes, every line. Against the four success
criteria in charter §3 it passes the two that can be judged now, and criterion 4
(explaining each artifact without opening the tool) is the one the content
quality actually decides. On that, the teaching content is better than the tools
this competes with. Several things in it are stated more precisely than the
standard states them, and a few are stated more precisely than most PMP courses
manage: velocity as a planning input and never a target, the fifth risk response
strategy, blocked as a flag rather than a column, "a change log where everything
is approved means the control is not real."

The largest structural gap is the one before the charter: everything that decides
whether a project should exist, and the measurement at the end that only that
half makes possible. Findings 2 and 3 are that. The rest is what a reviewer hands
back after a thorough read, and most of it is one or two lines per file.

## Coverage against PMBOK 6

Read as: what a student would learn, and what they would not know exists. Against
the 6th edition, because that is what the product cites. Finding 1 is about that
choice.

| Knowledge area | Covered | Where | Not covered |
|---|---|---|---|
| 4 Integration | 4.1, 4.3, 4.5, 4.6, 4.7 | charter, board, issues, status, changes, closeout | **4.2** no project management plan or baseline set anywhere (finding 13). 4.4 only at closeout |
| 5 Scope | 5.2, 5.3, 5.4 | requirements, scope | 5.1 named in a file header with no field behind it. 5.5 as prose only |
| 6 Schedule | 6.2, 6.3, 6.4, 6.5 | schedule (`depends`, `basis`, `baseline`) | 6.1. 6.6 folded into status. Critical path math correctly excluded |
| 7 Cost | one charter field | `charter.budget` | Everything else, by a documented charter exclusion |
| 8 Quality | acceptance criteria and definition of done | scope, board | **8.1, 8.2, 8.3.** No quality policy, metric or assurance. Not excluded either (finding 5) |
| 9 Resource | per-row owners | scope, schedule, board, risks, issues | **all six processes.** No roster, no responsibility assignment matrix (finding 4) |
| 10 Communications | status reports, engagement notes | status, stakeholders | **10.1.** Reports with no plan behind them saying who gets them (finding 6) |
| 11 Risk | 11.1, 11.2, 11.3, 11.5 | risks | 11.4 reasonably out. No triggers, no fallback, reserves only in an example (finding 7) |
| 12 Procurement | nothing | | **all three.** Absent and not excluded (finding 12) |
| 13 Stakeholder | 13.1, 13.2 | stakeholders | 13.4. Engagement gap is recordable but nothing monitors it |
| Business documents §1.2.6 | one charter field | `charter.business_case` | **the business case as an artifact, the benefits management plan, needs assessment, options, selection** (finding 2) |
| Agile Practice Guide | iteration planning, review, retrospective, DoR, DoD, WIP, velocity | sprints, board, review | Burndown. Product vs sprint backlog named only implicitly |

The charter screen itself is complete. All twelve items of 4.1.3.1 are present
and each is in its own field:

| 4.1.3.1 charter content | Field |
|---|---|
| project purpose | `purpose` (10-charter.pmdef:41) |
| measurable objectives and success criteria | `success_criteria` (:79) |
| high-level requirements | `high_level_requirements` (:97) |
| high-level description, boundaries, key deliverables | `description_deliverables` (:116) + `out_of_scope` (:131) |
| overall project risk | `overall_risk` (:150) |
| summary milestone schedule | `milestones` (:167) |
| preapproved financial resources | `budget` (:207) |
| key stakeholder list | `key_stakeholders` (:220) |
| project approval requirements | `approval_requirements` (:238) |
| project exit criteria | `exit_criteria` (:256) |
| assigned PM, responsibility and authority | `pm_authority` (:274) |
| name and authority of the sponsor | `sponsor` (:289) |

Splitting boundaries out into its own field is a departure from the standard's
list and it is the right one. It is also the only charter field with a check
that refuses an empty answer on grounds of what it costs later
(10-charter.pmdef:133).

---

## 1. The product cites a retired edition, and its stated reason no longer holds (material)

### The problem

The citation strategy is written down in two places, and it is a real argument
rather than an accident:

```
Field lists and process numbers follow the PMBOK Guide, 6th edition, which
still carries the prescriptive artifact contents later editions moved away
from.
```
`README.md`, "Source basis"

```
source: |
  PMBOK Guide, 6th ed., process 4.1 Develop Project Charter, in the Initiating
  process group. The field list below follows that process's charter contents.
  Later editions (7th onward) teach principles rather than fixed lists; this
  list is still the recognized checklist.
```
`definitions/10-charter.pmdef:35-39`

That was correct about the 7th edition. It is not correct about the 8th. PMI
released the 8th edition on 2025-11-13, and its own announcement lists the
"reintroduction of Process Groups as Focus Areas" and "seven performance
domains, including 40 newly evolved processes." Processes came back. "Later
editions teach principles rather than fixed lists" now describes one edition,
not the direction of travel.

Underneath that, the 6th edition was retired in November 2022. Its process
content did not disappear: PMI moved it into **Process Groups: A Practice
Guide**, which reproduces all 49 processes across the five process groups with
their inputs, tools and techniques, and outputs. That book is current, in print,
and a member download.

### Why it matters here rather than academically

**The process number is shown three times per screen visit.** It is a badge on
the screen header (`src/gui/screenview.cpp:50-51`), a badge in the help panel
(`src/gui/helppanel.cpp:98-99` and `:126-129`), and a badge on the walkthrough
page (`src/gui/guided.cpp:224-225`). A study aid that shows `4.1` that often is
teaching numbering, whether or not it means to. The numbering it teaches is from
a process model PMI has since reorganized.

**Charter §3's fourth success criterion is the one at risk.** "You can explain
each PM artifact and when it's used without opening the tool" is the criterion
that the content quality decides, and it is currently met against a superseded
edition. That is a different outcome from failing it, and it is not the one the
criterion was written for.

**The exam moved.** The PMP exam changed on 2026-07-09 and the previous version
retired the day before, so there is no longer a version of the exam that the 6th
edition is the primary reference for. Business Environment went from 8% to 26%
of it, which is the single largest weighting change and is the domain that covers
the business case, benefits realization, strategic alignment and value delivery.
That is finding 2's subject matter, which is why finding 2 is now the highest
value content change in the product rather than a fidelity item.

### The fix

Three options. The recommendation is the middle one, and the first is the one to
avoid.

**Do not leave it implicit.** The current text asserts something about later
editions that has stopped being true. Whatever else changes, those two passages
need rewriting, because a study guide whose stated basis is wrong about the
standard is worse than one that names an older basis on purpose.

**Recommended: re-cite against Process Groups: A Practice Guide.** The process
numbers do not move, because that guide reproduces the 6th edition's process set.
Nothing in `definitions/` needs renumbering and no field list changes. What
changes is the prose: every `source:` block currently saying "PMBOK Guide, 6th
ed., process X" becomes a citation to a publication that is still in print and
still downloadable, which is what somebody checking the tool's claims needs.

For `10-charter.pmdef:35-39`:

```
source: |
  Process Groups: A Practice Guide (PMI, 2022), process 4.1 Develop Project
  Charter, in the Initiating process group. The field list below follows that
  process's charter contents. That guide carries forward the 49 processes and
  their artifact contents from the PMBOK Guide 6th edition, which was retired
  in 2022; the 8th edition reorganizes the same ground into focus areas and
  performance domains without the prescriptive content lists. The process
  numbering here is the practice guide's.
```

And in the README's "Source basis", the same substitution, plus one sentence
saying the 8th edition exists and what it did. The honest version of the original
argument still works: the practice guide is where the prescriptive artifact
contents live, and that is why the field lists follow it.

**Later: add the 8th-edition mapping as a second line per screen.** Once the 8th
edition is in hand, each screen can name both, the practice guide process it
follows and the focus area or performance domain the 8th edition files it under.
That is the version a student sitting the current exam wants, because the
vocabulary on the exam is the 8th edition's. It is a bigger job and it needs the
book, so it is not a today task.

### One thing not to do

Do not renumber the `process:` keys to 8th-edition numbering without the book
open. The 8th edition is deliberately nonprescriptive, so the artifact contents
that this product's field lists come from may not be in it in list form. Every
field in `definitions/` traces to a content list. Changing the citations before
confirming the lists survive would break the one claim the product rests on,
which is that every field names where it comes from.

Finding 9 is the same layer of the product, smaller: one screen's `process:` key
names a process from the wrong process group. Worth fixing in the same pass.

---

## 2. There is no demand process, so a project appears to begin with its own charter (material)

### The problem

The first screen is the charter, and the charter is where a project is
*authorized*. Everything that decides whether a project should exist happens
before that, and none of it is in the tool.

The one trace is a single text field:

```
field business_case
  prompt: Why is this worth the time compared with everything else you could be doing?
  label: Business case / justification
  type: text
  origin: standard
  source: |
    Business documents (the business case and benefits management plan), which
    are inputs to 4.1 and feed the charter.
```
`definitions/10-charter.pmdef:59-67`

That `source:` block is correct and it contradicts the field's own placement.
The business case is a business document. It is created before the project, it
is owned by the sponsor or the portfolio rather than by the project manager, and
it is an *input* to 4.1 rather than a section of the charter. Filed as the second
field of the charter screen with `origin: standard`, it teaches that the project
manager writes the justification for their own project, which is the thing a PMP
question on business documents is usually testing you have not assumed.

The words "benefits management plan" appear exactly once in the repository, in
that `source:` line, with no field behind them. `needs assessment`, `portfolio`,
`intake`, `feasibility` and `options` appear nowhere.

### What it costs downstream

Three things, and the third is the one that matters.

**The tool cannot hold a project that was considered and not chartered.** That
is most of a demand funnel. A PM who only ever sees authorized work does not
learn the part of the job that is saying no.

**There is nowhere to record why this project beat the alternatives.**
`00-project-record.md` §6 lists four tools that already exist (ProjectMan,
MemoryBook, MindTether, the experience ledger) and the charter's business case
says "No existing tool holds project-level truth." That is an options analysis.
It was done, in prose, in a markdown file, and the tool it produced cannot
represent it.

**Closeout measures outputs and never outcomes.** `closeout.outcome` answers the
charter's success criteria one at a time (95-closeout.pmdef:53-85), and that is
the right thing to do with success criteria. But success criteria say whether the
project delivered what it said it would. Benefits say whether it was worth
doing, and they are a different answer. Look at charter §3's four criteria: every
field has a source tag, the project runs inside the tool, a second project is
managed in it, and you can explain each artifact without the tool. Three are
outputs. Only the fourth is a benefit, and it is the only one that cannot be
checked by looking at the software. A benefits table would have separated them,
given each a measure and a date, and named who is still watching after closeout.

Nothing in the current record can ask "was it worth building." That is the
question the whole provenance apparatus exists to make answerable, applied one
level up.

And per finding 1: this is the ground the current PMP exam moved most heavily
onto. Business Environment tripled its weighting, and the business case,
benefits realization and strategic alignment are what that domain holds.

### The fix

A screen before the charter. New file `definitions/05-intake.pmdef`:

```
# Intake and business case — before the project exists
#
# PMBOK Guide 6th ed. §1.2.6, the project business documents: the business case
# (§1.2.6.1) and the benefits management plan (§1.2.6.2). Both are written
# before there is a project, both are owned by the sponsor or the portfolio
# rather than by the project manager, and both are inputs to 4.1.
#
# This screen exists because a project that starts at its own charter cannot say
# why it was chosen over anything else, and cannot be measured at the end
# against anything but its own promises.

screen: intake
title: Intake and Business Case
phase: Before the project
process: Business documents, PMBOK 6th ed. §1.2.6
order: 5
teaches: |
  Where a project comes from before anybody charters it: who asked, what need
  it answers, what else was considered, and who decided this one was worth
  doing. By the end you should be able to say why the business case is not part
  of the charter, who owns it, and why benefits and success criteria are two
  different measurements.
before: |
  Nothing, and on most projects this is already written by the time a project
  manager arrives. If it is not, saying so is itself useful: a project with no
  business case has no defence when priorities move.
after: |
  The charter. The need becomes its purpose, the recommended option becomes its
  high-level description, and the benefits are what closeout measures against.
  A charter written before this is an authorization for something nobody
  compared with anything.
intro: |
  A project starts as a demand. Somebody asks for something, or a need turns up
  that nothing existing answers. Before it becomes a project, three questions
  get answered: what is the need, what are the options, and which one is worth
  doing.

  The answers are business documents, not project documents. The sponsor owns
  them. The project manager advises on them and then inherits them.
source: |
  PMBOK Guide, 6th ed., §1.2.6 Project Business Documents. The business case
  (§1.2.6.1) holds the needs assessment and the analysis of alternatives; the
  benefits management plan (§1.2.6.2) holds the target benefits, their metrics,
  the timeframe for realizing them and the benefits owner. Both are inputs to
  4.1 Develop Project Charter.

field request
  prompt: Who asked for this, when, and in what words?
  check filled: A project with no traceable request started because somebody had an idea on a Tuesday. Say who asked and quote them.
  label: Where the demand came from
  type: text
  origin: standard
  source: |
    The request, need or opportunity a needs assessment is written about. It
    precedes the business case.
  why: |
    Every project answers something. Writing down who asked and what they
    actually said is what lets you check later whether the thing you built
    answers the thing they asked for.

    It is also the first provenance entry on the project. A demand recorded as
    specified, with a quote, is a project whose existence you can defend.
  if_blank: |
    Nobody can say later why the project was started, so nobody can say whether
    it still needs to exist.
  example: |
    Nyx, 2026-09-15: "I want to build my own PM software." Does PM work by
    intuition and wants to see how it fits a PM job.

field need
  prompt: What need is this answering? State it without naming a solution.
  check words>=12: Too short, or it names the solution. A need that only makes sense as a description of the thing you already decided to build is not a need.
  label: Needs assessment
  type: text
  origin: standard
  source: |
    Needs assessment: the business need, the current state and the gap between
    them, written before any solution is chosen. It is what the options below
    are argued against.
  why: |
    A need stated as a solution rules out every other solution before anyone has
    compared them. "We need a PM tool" is a solution. "Project intent lives in
    chat, gets retyped, and usually is not" is a need, and a tool is one of
    several answers to it.
  if_blank: |
    The options below are decoration, because the answer was chosen before the
    question was written down.
  example: |
    Project intent, decisions and status live in chat and in memory. Nothing
    holds them anywhere that survives a session, so the same decision gets made
    twice and neither time is recorded.

field options
  prompt: What else could answer that need, including doing nothing? What did each one cost?
  check rows>=3: Two options is a decision already made. Include doing nothing: it is always available, and it is the one every business case has to beat.
  check column:option: Every row needs a name.
  check column:verdict: An option with no verdict is not a comparison. Say why each one lost.
  label: Options considered
  type: table
  origin: standard
  view: log
  title_column: option
  tone_column: verdict
  source: |
    Business case (§1.2.6.1): the analysis of alternatives, including the
    do-nothing option, against which the recommended option is justified.
  why: |
    The value of an options table is not the option you picked. It is the ones
    you did not, with the reason, so that when somebody asks in three months
    "why not just use Jira" the answer is a row rather than a memory.

    Doing nothing is always one of the options and it is the honest baseline. If
    the recommended option cannot beat it, the project should not be chartered.
  if_blank: |
    The project looks inevitable in hindsight, and the first person to suggest a
    cheaper alternative does it after the build has started.
  example: |
    O3 | Build it | A month of evenings | Teaches the role while producing the record | Recommended
  column ref
    label: ID
    type: line
    width: 1
  column option
    label: Option
    type: line
    width: 4
  column cost
    label: Cost or effort
    type: text
    width: 3
  column benefit
    label: What it would give you
    type: text
    width: 4
  column risk
    label: Main risk
    type: text
    width: 3
  column verdict
    label: Verdict
    type: choice
    options: Recommended=ok, Rejected=danger, Deferred=warn, Do-nothing baseline=faint
    width: 2

field benefits
  prompt: What measurable benefit does this produce, how is it measured, and when will you know?
  check rows>=2: One benefit is a hope. Name at least two, each with a measure.
  check column:measure: A benefit with no measure can be neither claimed nor denied at closeout. Say what you would count.
  check column:realized_by: A benefit with no date is never late. Say when you expect to see it.
  check column:owner: Name who is still watching for this after the project closes. It is usually not the project manager.
  label: Benefits and how they get measured
  type: table
  origin: standard
  view: log
  title_column: benefit
  tone_column: status
  source: |
    Benefits management plan (§1.2.6.2): target benefits, strategic alignment,
    the timeframe for realizing them, the benefits owner, the metrics, and the
    assumptions they rest on.
  why: |
    Success criteria say whether the project delivered. Benefits say whether it
    was worth doing. They are different answers, and a project can meet every
    criterion in its charter while producing nothing anyone wanted.

    Most benefits arrive after the project has closed, which is why they have an
    owner who is not the project manager. That is the whole reason this is a
    business document rather than a project document.
  if_blank: |
    Closeout can only ask whether you built what you said you would. Whether it
    was worth building never gets asked, which is how the same unwanted thing
    gets built twice.
  example: |
    B1 | Can explain each PM artifact with the tool closed | Answer a question on each of the 13 artifacts, unaided | 2026-11-01 | Nyx | Not yet measured
  column ref
    label: ID
    type: line
    width: 1
  column benefit
    label: Target benefit
    type: text
    width: 5
  column measure
    label: How it is measured
    type: text
    width: 4
  column baseline
    label: Where it starts
    type: line
    width: 2
  column target
    label: Target
    type: line
    width: 2
  column realized_by
    label: Expected by
    type: date
    width: 2
  column owner
    label: Benefits owner
    type: line
    width: 2
  column status
    label: Status
    type: choice
    options: Not yet measured=faint, On track=accent, Realized=ok, Missed=danger
    width: 2

field authorization
  prompt: Who decided this was worth doing, which option did they pick, and on what date?
  check date: A selection decision with no date is not a decision. Say who chose it and when.
  label: Selection decision
  type: line
  origin: standard
  source: |
    The decision to authorize the project, taken on the business case before the
    charter is written. In an organization it is a portfolio or governance
    decision. On a personal project it is the sponsor deciding to spend their
    own evenings.
  why: |
    This is the line between a demand and a project. Everything above it is a
    candidate; everything below it is authorized work. Without it the charter is
    the first record of a choice that was actually made earlier, on grounds that
    are now lost.
  if_blank: |
    The project appears to begin with its own charter, and the reason it was
    picked over everything else is not in the record.
  example: Option O3 chosen by the sponsor, 2026-09-15.
```

Then three edits:

- `definitions/10-charter.pmdef:11`, add `requires: intake` alongside the
  existing keys, so the walkthrough says what should come first.
- `definitions/10-charter.pmdef:65-67`, extend the `source:` block so the field
  says what it is: `The charter summarizes the business case. The case itself is
  a business document, written on the Intake screen before the project exists,
  and it is not one of the charter contents in 4.1.3.1.`
- `definitions/95-closeout.pmdef`, a benefits field after `outcome`, so the
  closing question is asked twice on purpose:

```
field benefits_realized
  prompt: For each benefit the business case promised: has it shown up yet?
  check column:state: An unanswered benefit is the one the project was actually for. Say yes, no, or too early.
  label: Benefits against the business case
  type: table
  origin: standard
  view: log
  title_column: benefit
  tone_column: state
  source: |
    Benefits management plan (§1.2.6.2), measured at closure. 4.7 confirms the
    deliverables were accepted; the benefits owner confirms, later, whether they
    were worth accepting.
  why: |
    The field above answers the charter. This one answers the business case, and
    the two come apart more often than anybody expects: a project that met every
    success criterion and produced no benefit is the most common expensive
    outcome in project management.

    "Too early to say" is a real answer here and is the honest one for most
    benefits on the day a project closes. What is not acceptable is leaving the
    row out.
  if_blank: |
    The project is judged only on whether it did what it said, and the reason it
    was funded goes unexamined.
  example: |
    B1 | Can explain each artifact unaided | Too early | Retest 2026-11-01 | Nyx
  column benefit
    label: Benefit
    type: text
    width: 5
  column state
    label: Realized
    type: choice
    options: Yes=ok, Partly=warn, No=danger, Too early=faint
    width: 2
  column evidence
    label: Evidence or when to recheck
    type: text
    width: 4
  column owner
    label: Owner from here
    type: line
    width: 2
```

### Two consequences, both fine

`phase: Before the project` adds a sixth group to the rail and to
`Definitions::phases()` (`src/core/definition.cpp:249-256`), which derives the
phase list from screen order. `nextScreen` still proposes the intake screen for a
record whose phase is `Initiating`, because it only skips screens from phases
*later* than the one the project has reached (`src/core/coach.cpp:206-214`). No
code change is needed for either.

Adding five fields moves record #1 off its current completion figure. That is
the right direction and it is R7's own argument: the record looked 93% complete
on first run because it was pre-loaded, and five honestly blank fields at the
front of a project that was chartered without them is a more accurate picture
than 93%.

---

## 3. The charter's `after:` block promises a chain the record cannot carry (material)

### The problem

```
after: |
  Everything downstream. The charter's stakeholder list becomes the
  stakeholder register; its high-level requirements become the requirements
  screen; its milestones become the schedule; its boundaries become the scope
  statement's exclusions.
```
`definitions/10-charter.pmdef:23-27`

Four flows claimed. Three of the four source fields are free text:

| Claimed flow | Source field | Type | Destination | Type |
|---|---|---|---|---|
| stakeholder list becomes the register | `key_stakeholders` :224 | `text` | `stakeholders.register` | `table` |
| high-level requirements become the requirements screen | `high_level_requirements` :101 | `text` | `requirements.items` | `table` |
| milestones become the schedule | `milestones` :172 | `table` | `schedule.activities` | `table`, no `reads:` |
| boundaries become the exclusions | `out_of_scope` :135 | `text` | `scope.exclusions` | `text` |

So every one of them is the project manager retyping. That is fine as PM work and
it is what a real handoff between artifacts looks like. It becomes a defect in
one specific place, because the tool has a mechanism for exactly this and uses it
once: `reads: board.cards` at `definitions/68-sprints.pmdef:92`, which is why a
card pulled into a sprint is on the board a moment later and there is one backlog
rather than two.

The place it costs something is the measurement loop. `charter.success_criteria`
is `type: text` (`:83`). `closeout.outcome` is a table whose `why:` says "This is
the charter's §3 answered, one line at a time"
(`definitions/95-closeout.pmdef:66-68`). A paragraph cannot be answered one line
at a time, and nothing checks that every criterion got a row. The check that
exists, `check column:met`, only fires on rows that are already there
(`:55`), so a closeout that answers two of four criteria and omits the two that
went badly passes the coach silently.

The requirements traceability matrix shows the same crack from the other side.
Its `traces` column exists (`definitions/30-requirements.pmdef:86-89`) and its
example traces to `Charter §3`, a section number in a markdown file
(`:63`). The RTM points at prose because charter objectives are not addressable.
A matrix whose left-hand side is not a set of rows is not a matrix.

### The fix, in two parts

**Part one needs no code.** Make `charter.success_criteria` a table, and have
the RTM and closeout point at its refs.

```
field success_criteria
  prompt: How will you know it worked? One condition per row, each checkable as true or false.
  check rows>=3: Fewer than three criteria is not a definition of success, it is a mood.
  check column:criterion: Every row needs the criterion itself.
  check column:measure: A criterion with no measure is an opinion with the number left out. Say what you would check.
  label: Measurable objectives and success criteria
  type: table
  origin: standard
  view: log
  title_column: criterion
  source: Charter content: measurable project objectives and related success criteria.
  why: |
    It is how you will know the project worked, which is a different question
    from whether it shipped. "Measurable" is the whole point: each criterion
    should be something you can check as true or false without arguing.

    One per row rather than one paragraph, because closeout answers them one at
    a time, and because a requirement that cannot name the objective it serves
    is not traceable to anything.
  if_blank: |
    The project cannot succeed or fail. It just stops, and nobody can say
    whether it was worth doing.
  example: |
    S1 | Every field in the tool has a source tag and a why | Open all 13 screens and read every field's panel
  column ref
    label: ID
    type: line
    width: 1
  column criterion
    label: Success criterion
    type: text
    width: 5
  column measure
    label: How it is checked
    type: text
    width: 4
```

No `met` column here on purpose. Grading a criterion on the charter screen is
grading yourself on the day you set the exam.

Then in `95-closeout.pmdef`, add to `field outcome` a `traces` column and a check
that refuses an untraced row:

```
  check column:traces: Say which charter criterion this row answers. A closeout that answers criteria it invented is not a closeout.
  column traces
    label: Answers
    type: line
    width: 2
```

**Part two is the completeness check, and it is a real code change**, because
`reviewField` sees one entry and cannot see the field it is supposed to cover:

```cpp
// src/core/coach.h
struct Check {
    enum class Kind { Filled, MinWords, MinRows, EveryRowHas, MentionsDate, MentionsAny, Covers };
```

```cpp
// src/core/coach.h — a covers: check needs the rest of the record to see what
// it is meant to be covering. Passed where there is one; where there is not the
// check stays quiet rather than failing an answer it cannot see.
std::vector<std::string> reviewField(const Field& field, const Entry* entry,
                                     const Record* record = nullptr);
```

```cpp
// src/core/coach.cpp, in parseCheck, beside the column: and mentions: cases
        if (what == "covers")   { out.kind = Check::Kind::Covers;      out.argument = argument; return true; }
```

```cpp
// src/core/coach.cpp, in reviewField
            case Check::Kind::Covers: {
                // Every row of the field named in the rule has to be answered by
                // a row here, matched on the ref it traces to. No record means
                // nothing to compare against, so the check does not fire.
                if (!record) break;
                const std::vector<Row>* expected = record->rows(check.argument);
                if (!expected) break;
                for (const Row& want : *expected) {
                    bool answered = false;
                    for (const Row& row : rows) {
                        const std::string traces = row.cell("traces");
                        if (traces.empty()) continue;
                        if (traces == want.id || traces == want.cell("ref")) { answered = true; break; }
                    }
                    if (!answered) { passed = false; break; }
                }
                break;
            }
```

The default argument keeps all eight existing call sites compiling. The four
that hold a record pass it: `reviewScreen` and `progressOf` in
`src/core/coach.cpp`, `src/gui/guided.cpp` and `src/gui/screenview.cpp` pass
`record_`, and `src/mcp/main.cpp` passes `record.get()`. Then closeout gets:

```
  check covers:charter.success_criteria: A success criterion with no row here is a criterion you are avoiding. An honest no is worth more than a missing line.
```

That is the loop closed: the charter sets criteria as rows, the RTM traces
requirements to them, and closeout cannot pass the coach while quietly skipping
one.

### One migration note, because it is silent

Changing a field from `text` to `table` orphans any answer already in a record.
`Record::load` reads both `value` and `row` children, and `serialize` writes both
back (`src/core/record.cpp:138-165`, `:183-215`), so the old paragraph stays in
the file while the table editor shows only rows. `Entry::isEmpty()` is false
because `value` is set, so the field still counts as answered and the new
`rows>=3` check reports it as thin with no explanation of where the old text
went.

Record #1 has this field filled. Retype the three criteria as rows and delete
the `value:` line from `charter.success_criteria` in the record file by hand.
It is one line in plain text, which is the reason the format was chosen.

---

## 4. Resource management has no artifact (worth doing)

Knowledge area 9 has no screen and no field. Ownership is recorded, once per
row, in five places: `scope.wbs` `owner` (`40-scope.pmdef:128`),
`schedule.activities` `owner` (`50-schedule.pmdef:90`), `board.cards` `owner`
(`70-board.pmdef:107`), `risks.register` `owner` (`60-risks.pmdef:100`) and
`issues.issues` `owner` (`85-issues-decisions.pmdef:78`). All five are
`type: line`, so nothing consolidates them and nothing distinguishes the person
doing the work from the person answerable for it.

The charter's exclusions do not cover this. `out_of_scope` rules out "resource
leveling / Gantt math" (`01-charter.md` §5), which is resource *optimization*.
Resource *management* is the roster, the responsibility assignment, and who has
authority over whom, and none of it is either present or excluded.

The teaching cost is specific: a responsibility assignment matrix is the
artifact most PM interviews ask about after the WBS, and RACI is the vocabulary
P3 exists to teach. A student who finishes AutoPM cannot say what the four
letters stand for.

The fix is one screen in Planning, `definitions/35-resources.pmdef`, with two
tables. A roster:

```
field roster
  prompt: Who is working on this, in what role, and how much of them do you have?
  check rows>=1: A project with nobody on it is not resourced. Name at least yourself, with the share of your time it actually gets.
  check column:role: A person with no role is a name. Say what they are on the project to do.
  check column:availability: "Available" is not a number. Say how much of them you have, because a plan built on all of somebody is built on a person who does not exist.
  label: Team roster
  type: table
  origin: standard
  view: log
  title_column: name
  source: |
    9.1 Plan Resource Management, output: the team roster and the roles,
    responsibilities, required competencies and authority that go with each.
  why: |
    Durations assume a person and a share of their week. Writing the share down
    is what makes a slip diagnosable: an activity that took twice its estimate
    on a quarter of somebody's time did not take twice as long.

    Authority is the column people skip and the one that causes the arguments.
    It says who can decide what without coming back to the project manager.
  if_blank: |
    Estimates assume everybody is fully available, which nobody is, and the
    schedule is optimistic by however much that is wrong.
  example: |
    Delivery team | Builds from the handoffs | C++/Qt, Win32 | One session at a time | Implements, does not decide
  column name
    label: Name
    type: line
    width: 2
  column role
    label: Role on the project
    type: line
    width: 3
  column skills
    label: Competencies needed
    type: text
    width: 3
  column availability
    label: Availability
    type: line
    width: 2
  column authority
    label: Can decide
    type: text
    width: 3
```

And a responsibility assignment, long form, one row per pairing:

```
field assignments
  prompt: For each piece of work, who is accountable and who does it?
  check rows>=3: Fewer than three assignments and the matrix is not doing anything a list of owners was not already doing.
  check column:raci: Every row needs a letter. Two people accountable for one thing means nobody is.
  label: Responsibility assignment
  type: table
  origin: standard
  view: log
  title_column: item
  tone_column: raci
  source: |
    9.1 Plan Resource Management: the responsibility assignment matrix, most
    often a RACI chart. One row per pairing here rather than one column per
    person, because a table with a column per person needs a new column every
    time somebody joins.
  why: |
    RACI: responsible does the work, accountable answers for it, consulted is
    asked first, informed is told afterwards. The two that matter are the first
    two, and the rule is that exactly one person is accountable for any one
    thing. Two is nobody.

    It is worth writing even on a project of one person, because it is where you
    find out that the same person is accountable for every line, which is a real
    finding about the project rather than a formality.
  if_blank: |
    Work has owners but no accountability, so the question "who decided this was
    finished" has no answer.
  example: |
    1.2 Charter screen | Delivery team | Nyx | | Archon | Accountable
  column item
    label: Work or deliverable
    type: line
    width: 3
  column responsible
    label: Responsible (does it)
    type: line
    width: 2
  column accountable
    label: Accountable (answers for it)
    type: line
    width: 2
  column consulted
    label: Consulted
    type: line
    width: 2
  column informed
    label: Informed
    type: line
    width: 2
  column raci
    label: Primary letter
    type: choice
    options: Accountable=danger, Responsible=accent, Consulted=neutral, Informed=faint
    width: 2
```

With `requires: scope` and `order: 35`, so it sits between requirements and the
scope screen in the rail and after the WBS exists in the walkthrough.

## 5. Quality has no artifact, and is not excluded either (worth doing)

Knowledge area 8 is represented by two fields, both about acceptance rather than
about quality: `scope.acceptance_criteria` (`40-scope.pmdef:52`) and
`board.definition_of_done` (`70-board.pmdef:143`). The definition of done's `why:`
calls itself "the cheapest quality control a team has" (`:154`), which is true
and is not a quality management plan.

Nothing records what standard the work is held to, what gets measured, or how it
is checked other than by someone accepting it. `quality metric`, `cost of
quality` and `Plan Quality` appear nowhere in the repository.

This is a decision rather than an omission to fix blind, and there are two
honest answers. Either a small screen in Planning:

```
field standard
  prompt: What standard does the work have to meet, beyond somebody accepting it?
  check words>=10: Name the standard and how it is checked. "High quality" is a wish; "builds clean at /W4 with no new warnings" is a standard.
  label: Quality standard
  type: text
  origin: standard
  source: |
    8.1 Plan Quality Management, output: the quality management plan, which
    states the standards the project will be held to and how compliance is
    demonstrated.
  why: |
    Acceptance criteria say what the deliverable must do. A quality standard
    says how well, and the difference is what stops "it works" and "it is
    finished" from being treated as the same claim.
  if_blank: |
    Quality is whatever the person accepting it happened to notice, which means
    it varies with how tired they were.
  example: |
    Builds clean at /W4. Core changes ship with tests. Every field's help panel
    written before the field is called done.
```

with a metrics table beside it, or a line added to the charter's `out_of_scope`
saying quality management is deliberately carried by the definition of done and
nothing else. Either is defensible. Leaving it unstated teaches that the
knowledge area does not exist, which is the one outcome that costs something
against success criterion 4.

## 6. Status reports have no communications plan behind them (worth doing)

`90-status.pmdef` produces work performance reports and cites 10.2 Manage
Communications in its `source:` (`:29-32`). 10.1 Plan Communications Management
is absent, so nothing says who receives a report, how often, in what format, or
through what channel. The nearest thing is `stakeholders.strategy`
(`20-stakeholders.pmdef:97`), which covers engagement strategy under 13.2.

This one is cheap, because the register already holds the people. Two columns on
`stakeholders.register`:

```
  column receives
    label: Gets what, how often
    type: line
    width: 3
  column channel
    label: How
    type: choice
    options: In person, Call, Written report, Chat, Record only
    width: 2
```

and a check that makes the omission visible:

```
  check column:receives: Somebody in this table is not being told anything. Say what each stakeholder gets and how often, or the reports go to whoever asks.
```

The teaching line is that a report nobody agreed to receive is a report nobody
reads, and that the communications plan is the artifact that turns a stakeholder
register into something operational.

## 7. The risk register has no trigger, and reserves live only in an example (worth doing)

`60-risks.pmdef` is the strongest single file in the product. It has the five
threat strategies named correctly including escalate (`:55-59`), the cause-event-
effect sentence structure (`:61-62`), owners, planned responses, and `Occurred`
as a status. Two things are missing and both are the same kind of thing.

**No trigger.** A risk has a planned response and nothing says when to fire it.
That is how a mitigation that was written down still happens late. The register
contents in 11.2 include the warning signs, and they are the operational half of
a response.

```
  column trigger
    label: What you would see first
    type: text
    width: 4
```
```
  check column:trigger: A response with no trigger fires when somebody happens to notice. Say what you would see first, so the response has a moment to start at.
```

**Reserves appear once, in an example.** `50-schedule.pmdef:121` has "No
contingency reserve added" inside the `example:` block of `basis`. Contingency
reserve against identified risks, and management reserve against the unknown, is
one of the distinctions a PMP exam tests hardest, and the tool mentions it only
as sample text. A line in `risks.thresholds`'s `why:` naming both, or a
`reserves` field beside it, fixes it.

A `fallback` column is worth considering beside `response` at the same time: the
planned response is what you do to stop it, the fallback is what you do when the
response did not work, and a register that conflates them has no plan B written
anywhere.

## 8. The issue log's own `why:` names a column that does not exist (small)

```
  why: |
    Every issue needs one named owner and one date. Shared ownership of an
    issue means nobody owns it.

    The escalation column matters more than it looks: an issue that has been
    open past its due date with no escalation is really a decision to do
    nothing, taken quietly.
```
`definitions/85-issues-decisions.pmdef:49-55`

There is no escalation column. The table has `ref`, `issue`, `raised_on`,
`priority`, `owner`, `due`, `resolution`, `status` (`:61-94`), and `status` has
an `Escalated` value. So the state is recordable and the thing the prose says
matters, who it went to and when, is not.

The help panel prints that `why:` block verbatim beside the table
(`src/gui/helppanel.cpp`), so the text points at a column the reader is looking
at and cannot find.

```
  column escalated_to
    label: Escalated to
    type: line
    width: 2
  column escalated_on
    label: Escalated
    type: date
    width: 2
```

## 9. The Issues screen shows a process number from the wrong process group (small)

```
phase: Monitoring and Controlling
process: 4.3 Direct and Manage Project Work
```
`definitions/85-issues-decisions.pmdef:9-10`

4.3 is an Executing process. The screen is filed under Monitoring and
Controlling, correctly, because that is where an issue log is worked. The
`source:` block gets this right and says so: "Issue log: 4.3 Direct and Manage
Project Work (output), maintained through 4.5 Monitor and Control Project Work"
(`:33-34`).

The `process:` key is the one the user sees. It is drawn as a badge on the screen
header (`src/gui/screenview.cpp:50-51`), in the help panel
(`src/gui/helppanel.cpp:98-99` and `:126-129`) and on the walkthrough page
(`src/gui/guided.cpp:224-225`), so a study guide that pairs a Monitoring and
Controlling phase with an Executing process number shows that pairing three
times per visit. Process group per process is exactly what gets memorized from a
tool like this.

`process: 4.5 Monitor and Control Project Work` matches the phase and the
`source:` block already explains the 4.3 origin. Every other screen's pairing
checks out: 4.1 and 13.1 are the two Initiating processes, 5.2, 5.3, 5.4, 6.5,
11.2 and 11.5 are Planning, 4.3 on the board is Executing, 4.6 and 4.5 are
Monitoring and Controlling, 4.7 is the only Closing process, and the two agile
screens are correctly marked as coming from the Agile Practice Guide rather than
the PMBOK Guide.

This is the same layer of the product as finding 1. Worth doing in one pass.

## 10. The change log records when a change arrived, not when it was decided (small)

`80-changes.pmdef` has `raised_by` (`:71`), `raised_on` (`:75`) and `decided_by`
(`:83`). There is no decision date.

Two things go missing with it. Approval cycle time, which is the number that
tells you whether change control is a process or a bottleneck. And the date the
baseline actually moved, which is what a rebaseline note on the schedule screen
should be able to point at: `schedule.baseline`'s example says "One rebaseline so
far, logged on the Change Log screen" (`50-schedule.pmdef:139-140`) and the
change log cannot say when.

```
  column decided_on
    label: Decided
    type: date
    width: 2
```
```
  check column:decided_on: A decided change with no decision date cannot show how long approval took, and cannot say when the baseline moved.
```

## 11. The assumption log is a charter output filed under Planning (small)

```
field assumptions
  source: |
    Assumption log (4.1 output, maintained throughout). Assumptions and
    constraints are recorded together because both limit what the plan can be.
```
`definitions/30-requirements.pmdef:96-104`

The `source:` is right. The assumption log is one of the two outputs of 4.1,
alongside the charter itself. It sits on a Planning screen gated behind
`requires: charter` (`:11`), so the tool asks for assumptions after the charter
is signed while its own source line says they come out of writing it.

The `before:` block on that screen reinforces it: "The charter has to be signed.
Detailed requirements are Planning work" (`:17-20`). True of requirements, not
true of the assumption log.

Cheapest fix is an `assumptions` field on the charter screen, with the
requirements one kept and reworded as elaboration. That also matches how it
works in practice: the first assumptions are the ones you make while writing the
charter, and they are the ones most worth catching.

## 12. Procurement is absent and not excluded (small)

Knowledge area 12 has nothing, and the charter's out-of-scope list does not
mention it. For a personal desktop tool with a $0 budget, excluding it is
obviously right. Write the exclusion. An unstated omission and a stated decision
look identical in the product and teach opposite things, and this is the second
of the two knowledge areas in that position (quality is the other).

One line in `charter.out_of_scope`'s example, and one line in the charter record,
is the whole fix.

## 13. Nothing names what is baselined (worth doing)

4.2 Develop Project Management Plan has no screen, which is defensible: a
screen called "Project Management Plan" collecting nine subsidiary plans would
be a form nobody fills in. The subsidiary content is distributed to the screens
that use it, which is better teaching. `changes.process` is the change
management plan and says so (`80-changes.pmdef:99-102`), `risks.thresholds` is
the risk management plan (`60-risks.pmdef:116-118`), `schedule.baseline` is the
schedule baseline, `sprints.cadence` and `board.definition_of_done` are the
agile equivalents.

What is missing is the sentence that ties them together. Nowhere does the tool
say which artifacts are baselined, as of when, and therefore which ones can only
change through the change log. That distinction, plan versus baseline versus
project document and what each one costs to change, is a core concept and the
tool currently teaches it only for the schedule.

A `baselines` field on the Change Log screen is the natural home, because it is
the screen that governs them:

```
field baselines
  prompt: What is baselined on this project, and as of when?
  check filled: Until this says something, "changing the plan" and "editing a document" are the same action, and only one of them should need approval.
  check date: A baseline with no date is not a baseline. Say what was approved and when.
  label: What is under change control
  type: text
  origin: standard
  source: |
    4.2 Develop Project Management Plan: the scope, schedule and cost baselines
    are the approved versions, changed only through 4.6 Perform Integrated
    Change Control. Project documents are not baselined and do not need it.
  why: |
    Three things get confused and only one of them needs a change request. A
    baseline is approved and moving it is a decision. A subsidiary plan is how
    you intend to work and can be revised. A project document, the risk register
    or the issue log, is updated as a matter of course.

    Listing what is baselined here is what makes the rest of this screen mean
    something: without it, every edit looks like a change and so nothing does.
  if_blank: |
    Change control applies to whatever somebody remembers it applying to, which
    in practice is nothing.
  example: |
    Scope baseline (scope statement and WBS) approved 2026-09-15. Schedule
    baseline from the charter milestones, same date. No cost baseline: the
    budget is $0. Risk register and issue log are documents, not baselines.
```

---

## On Closing, which is correctly blank

`03-handback-m2.md` and PR #1 both say the Closing screens are blank because the
project has not closed, and that this is the correct state rather than a gap.
That is right and it is the part of the tool most people would have filled in
with a guess.

Two things are missing from the screen rather than from the record, and both are
worth a field before the project reaches it.

**4.7 is Close Project *or Phase*.** The tool closes a project. In a hybrid
with five phases and five milestones, each phase should gate: deliverables for
that phase accepted, lessons captured while they are still true, and a decision
to proceed. The screen is `title: Closeout`, singular, and the record has one set
of answers. A `phase_closures` table, one row per phase with the date it closed
and who accepted it, would make the "or Phase" half of the process real. It would
also mean M1 through M4 get closed rather than passed.

**The final report is a 4.7 output and has no field.** `archive`
(`95-closeout.pmdef:132`) covers where the record lives and who owns the product.
`status.summary` is the running statement. Neither is the final report, which is
the summary of performance against the plan that the sponsor keeps after the
project is gone.

Resource release belongs here too, and follows finding 4: if there is a roster,
closing releases it.

---

## Do not change these

These are right, and several are right in ways that competing tools are not. A
remediation pass should leave them alone.

**The charter is complete against 4.1.3.1.** All twelve contents, one field
each, verified item by item in the table above. Splitting boundaries into its own
field is a departure and an improvement.

**Risk and issue are kept apart and taught, not just separated.** "A risk is an
uncertain event that would affect the project if it happened. It is not a
problem: a problem has already happened, and belongs on the Issue Log"
(`60-risks.pmdef:23-26`), and the mirror statement at
`85-issues-decisions.pmdef:25-28`. `status: Occurred` on the register is the
transition between them.

**All five threat response strategies, including escalate**
(`60-risks.pmdef:55-59`), with the opportunity mirror set named in the same
breath. Escalate is the one most tools and most study aids omit, and omitting it
is why people mitigate things that were never theirs to mitigate.

**"A good risk statement has a cause, an event and an effect"**
(`60-risks.pmdef:61-62`). This is the single most useful sentence in the product.

**MoSCoW with Won't as a real option** (`30-requirements.pmdef:77-81`), and the
reason given: "it forces a real ranking instead of everything being high"
(`:52-54`). A priority scheme with no bottom is a priority scheme with no top.

**The 100% rule and the decomposition heuristic**
(`40-scope.pmdef:101-108`): "If you cannot estimate an item, it is not
decomposed far enough; if you are tracking hours on it, it is decomposed too
far." That is the answer to the only real question anyone has about a WBS.

**"The WBS decomposes deliverables, not activities. Each item is a noun"**
(`40-scope.pmdef:28-29`). Most people get this wrong for years.

**The baseline as the thing that makes lateness possible**
(`50-schedule.pmdef:132-135`): "without a baseline, a schedule that is always
being adjusted is never late."

**"A log where everything is approved means the control is not real"**
(`80-changes.pmdef:51-52`), and rejection framed as a healthy outcome. That is a
governance insight, not a form instruction.

**Velocity as a planning input and never a target**
(`68-sprints.pmdef:103-109`): "Capacity is a measurement, not an ambition" and
"The moment it becomes a target, cards get estimated higher and the number stops
measuring anything." Almost every agile tool on the market gets this wrong by
putting velocity on a dashboard as a goal.

**Over-commitment shown rather than blocked** (`68-sprints.pmdef:98-101`):
"deciding what to drop is the planning." Correct, and the same principle as the
coach.

**Blocked as a flag rather than a column** (`70-board.pmdef:81-83`), because a
blocked card still occupies the state it is stuck in and moving it would make the
WIP count lie. That is a genuine insight about board mechanics.

**Review and retrospective as two events with different audiences**
(`74-review.pmdef:29-32`): "a retrospective with stakeholders in the room stops
being honest." One action per retrospective rather than ten (`:102-105`).

**One backlog, not two** (`68-sprints.pmdef:92`, `reads: board.cards`). The
mechanism that makes sprint planning move the board's own cards is the right
answer to a problem most hybrid tools solve with a second list that drifts.

**The coach never blocks** (`src/core/coach.h:7-9`, and the framing in the README
and in every `check` message). Being told and carrying on is a PM decision. This
is the correct relationship between a tool and a practitioner, and it is what
makes the difference between a study guide and a workflow engine.

**`nextScreen` will not propose a screen from a phase the project has not
reached** (`src/core/coach.cpp:195-217`). "Telling somebody to go and write its
lessons learned is worse advice than saying nothing." Right call, and the reason
Closing is correctly blank rather than nagging.

**Blank is not thin** (`src/core/coach.cpp:169-181`). Counting unanswered fields
as bad answers would report a project nobody has started as a project full of bad
work. The test suite covers this (`tests/test_core.cpp`, "coach: an unanswered
field is not a thin answer").

**Provenance.** Not in the standard, better than the standard for this purpose,
and enforced at the MCP boundary rather than suggested. The four states and the
rule that only two are decisions is a real contribution, and `unobjected` is the
state every other decision log silently treats as agreement.

**The definition files as the study guide.** Every `why:` and `if_blank:` in
these 13 files says what breaks rather than what the field is. That is the
difference between help text and teaching, and it is consistent across all of
them.

---

## One note for anyone adding a screen

`view: matrix` draws nothing at all unless both axis columns are `type: choice`
with declared options. `MatrixView::paintEvent` returns early on
`rowAxis->options.empty() || colAxis->options.empty()`
(`src/gui/planviews.cpp:277-281`), with no message and no fallback to the grid.

Since the product's central claim is that adding a screen means adding a file,
this is the one place where a plausible file produces a blank rectangle and no
explanation. It is why the responsibility assignment in finding 4 is specified as
`view: log` rather than as the matrix it looks like it should be. Either document
the constraint in a comment at the top of `20-stakeholders.pmdef`, which is the
file anyone will copy from, or have the view fall back to the table when its axes
will not work.

---

## Suggested order

**First, the edition decision.** Finding 1, and finding 9 with it. Re-citing
against Process Groups: A Practice Guide costs one `source:` block per file plus
the README's Source basis, and it moves every existing citation from a retired
edition to one that is in print. Nothing renumbers. Do this before the other
content work, because the other findings add `source:` blocks and they should be
written against whatever the answer is.

**Second, and before the second project starts.** Findings 2 and 3 together. The
intake screen, the benefits table, the success criteria as rows, and the
`covers:` check. They are one piece of work: the front end that says why a
project exists and the back end that measures whether it was worth doing, joined
by the only chain in the tool that closes. Doing this before SmallHours goes into
AutoPM means the second project gets chartered the way the tool will teach it
from then on, and success criterion 3 tests the whole lifecycle rather than the
part after authorization. The exam reweighting in finding 1 is the second reason:
this is the content the current Business Environment domain is made of.

**Third, the silent contradiction.** Finding 8. The issue log's `why:` pointing
at a column that does not exist. Three lines, and it is visible to the reader in
a product whose entire claim is that every field names its source correctly.

**Fourth, the two absent knowledge areas.** Findings 4 and 5. Resource
management as a screen, quality as either a screen or a written exclusion.
Finding 12 rides along with the quality decision, since it is the same choice
made the other way.

**Fifth, the operational gaps.** Findings 6, 7, 10 and 13. Communications
columns on the register, triggers and reserves on risks, a decision date on the
change log, and the baselines field. All small, all one file each.

**Last.** Finding 11, the assumption log's placement, and the Closing notes:
phase closure, the final report, resource release. None of them blocks anything
and the Closing ones are not needed until the project is closing.

Nothing here changes the verdict on §8, and nothing here is a defect in what was
built. It is a content review of a study guide. Two findings are worth acting on
before anything else: the standard moved underneath the citations, and the tool
teaches the project management lifecycle from the moment a project is authorized,
when the part before that is where a project manager decides whether there should
be a project at all.

---

## Sources for finding 1

The edition and exam claims, so they can be checked rather than taken:

- PMI's own launch announcement, 2025-11-13, for the 8th edition's contents
  including "reintroduction of Process Groups as Focus Areas" and "seven
  performance domains, including 40 newly evolved processes":
  <https://www.projectmanagement.com/articles/1134510/pmi-launches-the-pmbok--guide---eighth-edition>
- PMI on the new exam, launched 2026-07-09:
  <https://www.pmi.org/certifications/project-management-pmp/new-exam>
- The current PMP Examination Content Outline, for the domain weightings
  (Business Environment 8% to 26%, People 42% to 33%, Process 50% to 41%):
  <https://www.pmi.org/-/media/pmi/documents/public/pdf/certifications/new-pmp-examination-content-outline-2026.pdf>
- Process Groups: A Practice Guide (PMI, 2022), which carries the 49 processes
  and their ITTOs forward from the 6th edition:
  <https://pmi.bookstore.ipgbook.com/process-groups--a-practice-guide-products-9781628257830.php>

All four are free to read. The practice guide and both PMBOK editions are member
downloads from pmi.org under Resources, then Standards, then Library of Global
Standards.
