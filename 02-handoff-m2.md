# Handoff to Delivery Team — PM Tool, Work Package M2

**From:** Nyx (project manager and sponsor)
**To:** Delivery team
**Date:** 2026-09-15
**Milestone:** M2, Initiating screens (target: 2026-09-15)
**Project record:** `00-project-record.md` and `01-charter.md`, in this folder and in the "Project ideas" workspace under `pm-tool/`. Conversation history is in Reliquary as `pm-tool-kickoff-2026-09-15`.

---

## 1. Roles and how to work

- **Nyx is the PM and the sponsor.** She owns scope, priorities and acceptance. You implement.
- **You don't decide scope.** If something isn't covered here, stop and ask. Any implementation choice that changes what she will see or use goes back to her as a question.
- **Never write "you decided X" without a verbatim quote and a date.** Provenance has three states: *specified* (she said it), *agreed* (she approved a summary), *unobjected* (proposed, no reply). Only the first two count as decisions.
- **She's learning the role through this build.** Explain your approach before implementing, name tradeoffs, and make sure she can follow what you did. Finished code doesn't replace her understanding it.
- **Final review:** Archon reviews once, when Nyx thinks it's done, and says whether it's right. Code should be readable to him: C++/Qt is his home stack.

## 2. The product

A **personal, installable desktop app, like ProjectMan**, that turns a project manager's workflow into guided input forms. It has three jobs:

1. **Study guide.** Every field explains where it comes from in the standard, why a PM fills it in, and what goes wrong when nobody does.
2. **Practical application.** Nyx runs real projects in it, starting with this one.
3. **Future organization.** It becomes where her projects are tracked.

Framework: **hybrid**. Predictive phases (Initiating → Planning → Executing → Monitoring & Controlling → Closing), with agile sprints inside Executing. Screens are built in lifecycle order.

## 3. Approved decisions (all dated 2026-09-15)

| # | Decision | Evidence |
|---|---|---|
| D3 | Main purpose is learning the role by building its workflow as input forms | "build the virtual workflow as an imput field and learn how to use it" |
| D5 | Hybrid framework | "hybrid sounds like the best option for us at the current moment" |
| D6 | Three jobs: study guide, practical application, future organization | "a product that acts as a study guide, practical application, and future orginization" |
| D7 | Every field is sourced to the standard and explains itself | "yes exatly" |
| D8 | Industry vocabulary; lifecycle build order; this project is record #1; charter entries as drafted | "yes to all proposals" |
| D9 | Personal, installable software, modeled on ProjectMan | "personal installable software- like... project man" |
| D10 | Schedule: M2 Sep 15 · M3 Sep 16 · M4 week of Sep 21 · M5 closeout Oct 15 | milestones message |
| D13 | Archon does one final review | "he does a review when i think we are done, and he tells me if it is right or not" |
| D14 | Charter approved | "charter approved" |
| D15 | Stack: C++/Qt like ProjectMan, with field definitions in readable data files | "go with recommended stack" |

**Out of scope (whole project):** budget/cost accounting beyond one simple field · multi-user permissions (each person runs their own install) · replacing ProjectMan's git tracking · resource leveling / Gantt math.

## 4. Scope of this work package (M2)

Build the **Initiating** screens:

1. **App shell:** a project list (with New project), one tab per screen, and a help panel.
2. **Project Charter** screen: 16 fields (spec in §7).
3. **Stakeholder Register** screen: register table plus engagement notes (spec in §7).
4. **Help ("why") panel:** when a field has focus, show its label, whether it's *standard* or *her own*, its source, why a PM fills it in, what happens if it's left blank, and an example. When no field has focus, show the screen intro and source.
5. **Provenance on every entry:** each field, and each table row, has a source tag (untagged / specified / agreed / unobjected; nothing else) and an evidence line (quote and date).
6. **Record #1:** this project's own approved charter, pre-loaded on first run. Its content is in `01-charter.md`. Nyx fills in Archon's power/interest/engagement herself; leave those blank.
7. **Installer:** per-user install with a Start Menu shortcut and an uninstall entry.

Not in M2: requirements, WBS, schedule, risk register (M3); sprint board, change log, status reports (M4); closeout (M5); the decision log as its own screen (not scheduled yet; ask).

## 5. Open PM questions: ask, don't assume

- Working title: none yet. Use "PM Tool (working name)". No GitHub repo until it's named.

## 6. Technical constraints

- **Stack:** C++17, CMake, **Qt 6.8.3 msvc2022_64** at `C:\Qt\6.8.3\msvc2022_64`, MSVC 2022 Build Tools (installed). Generator `Visual Studio 17 2022 -A x64` (no Ninja on this machine).
- **Shape, following ProjectMan:** a core that reads definitions and records and knows nothing about windows, plus a separate GUI. ProjectMan (`Locke-Werks/ProjectMan`) is the reference: Qt-free `src/core`, with `cli` / `gui` / `mcp` front ends.
- **Field definitions are data, not code:** one readable file per screen, holding everything in §7. Adding a screen should mostly mean adding a file. **No field is hard-coded.** These files *are* the study guide, so Nyx has to be able to read them.
- **Records:** one human-readable file per project that Archon can diff. On this machine, Documents redirects to OneDrive: `C:\Users\trist\OneDrive\Documents`.
- **Packaging:** Forge. Tools are in `C:\Users\trist\Downloads\LockeWerks\` (`lwforge.exe`, `lwstub.exe`). Recipe that works on this machine:
  `lwforge.exe build --config <toml> --payload <Release dir> --stub lwstub.exe --out <Setup.exe> --dev`
  Use `scope = "user"` with `elevation = "on-demand"`. Examples: `C:\Users\trist\projects\Archon's\Parse\forge\parse.toml`, and ProjectMan's `installer.toml` on GitHub. Don't include `*.lib` in the payload.
- **Gotchas:** run `windeployqt.exe --release <exe>` before launching or packaging, because a bare Qt exe won't start. `--sign` isn't available (no Azure signing credentials), so builds are `--dev`, unsigned.
- **Code location:** a folder under `C:\Users\trist\projects\`. Confirm the folder name with Nyx.

## 7. Field specification (the study-guide content)

Source basis: PMBOK Guide 6th ed., processes 4.1 Develop Project Charter and 13.1 Identify Stakeholders. Engagement levels come from the Stakeholder Engagement Assessment Matrix (13.2). Origin `yours` marks a field that's Nyx's own, not from the standard.

#### Project Charter (Initiating)

*Screen intro:* The charter is the document that formally authorizes a project to exist. It names the project, says why it exists, draws its boundaries and gives the project manager authority to spend resources on it. Nothing downstream (requirements, schedule, risks) should be started until the sponsor signs it.

*Screen source:* PMBOK Guide, 6th ed., process 4.1 Develop Project Charter (Initiating process group). The field list below follows that process's charter contents. Later editions (7th onward) teach principles rather than fixed lists; this list is still the recognized checklist.

| # | id | Label | Type | Origin | Source | Why a PM fills it in | If left blank | Example |
|---|---|---|---|---|---|---|---|---|
| 1 | `purpose` | Project title and purpose | text | standard | Charter content: project purpose. | It's the 'why are we doing this' that every later choice gets checked against. | Scope drifts, because nothing can be ruled out. | Build a PM product that works as a study guide, practical application and future organization. |
| 2 | `business_case` | Business case / justification | text | standard | Business documents (business case, benefits management plan), which feed the charter. | It answers 'why is this worth the time and money compared to anything else.' | The project has no defense when priorities shift, and sponsors cancel projects that can't say why they matter. | No existing tool holds project-level truth: charter, scope, decisions, risks and status against a plan. |
| 3 | `success_criteria` | Measurable objectives and success criteria | text | standard | Charter content: measurable project objectives and related success criteria. | It's how you'll know the project worked, which is a different question from whether it shipped. 'Measurable' is the point: each criterion should be something you can check as true or false. | The project can't succeed or fail. It just stops, and nobody can say whether it was worth it. | Every field in the tool has a source tag and a 'why' explanation. |
| 4 | `high_level_requirements` | High-level requirements | text | standard | Charter content: high-level requirements. Detailed requirements come later, in Planning (5.2 Collect Requirements). | Just enough to size the work and catch a wrong direction early. | Nobody can tell whether the project is sized right until the build is already underway. Too much detail here is also a failure: it's Planning work done before the project is authorized. | Guided input forms for each lifecycle artifact. |
| 5 | `description_deliverables` | High-level description and key deliverables | text | standard | Charter content: high-level project description, boundaries and key deliverables. | It says what the project will actually hand over when it's done. | The team builds whatever seems obvious, and each person's version of obvious is different. | Personal, installable desktop software covering the full project lifecycle. |
| 6 | `out_of_scope` | Boundaries: out of scope | text | standard | Charter content: project boundaries. Formalized later in the scope statement's exclusions (5.3 Define Scope). | Writing down what the project will NOT do prevents more scope creep than any in-scope list. | Every good idea becomes an implied requirement, and the project never finishes. | Multi-user permissions. Gantt / resource-leveling math. |
| 7 | `overall_risk` | Overall project risk | text | standard | Charter content: overall project risk. The full risk register comes in Planning (11.2 Identify Risks). | It sets the project's risk posture at the start, so bad news later isn't a surprise. | Risks are discovered as problems instead of being watched as possibilities. | A single late review gate: defects found at the most expensive moment. |
| 8 | `milestones` | Summary milestone schedule | table: Milestone, Target date, Status (Not started / In progress / Reached / Missed / Rebaselined), Notes | standard | Charter content: summary milestone schedule. The detailed schedule comes in Planning (6.5 Develop Schedule). | Milestones are dated checkpoints, not tasks. They turn 'in progress' into something that can be on time or late. | 'In progress' never ends, and nobody notices a slip until it's large. | M1 Charter approved \| Sep 15 \| 'hopefully today' |
| 9 | `budget` | Pre-approved financial resources | line | standard | Charter content: preapproved financial resources. | It sets how much can be spent before anyone has to go back to the sponsor. | Either every purchase needs a conversation, or spending grows unchecked. | $0 beyond existing subscriptions and hosting. |
| 10 | `key_stakeholders` | Key stakeholder list | text | standard | Charter content: key stakeholder list. Expanded in the Stakeholder Register (13.1 Identify Stakeholders). | Everyone who is affected by the project, or can affect it, gets named up front. | An unlisted stakeholder shows up late with an objection nobody planned for. | Sponsor/PM/user; final reviewer; delivery resource. Details on the Stakeholder Register tab. |
| 11 | `approval_requirements` | Project approval requirements | text | standard | Charter content: project approval requirements (what constitutes success, who decides, who signs off). | It defines what 'done and accepted' means, and who has the authority to say so. | 'Done' gets argued about at the end, when it's most expensive to disagree. | You accept each milestone; Archon gives the final verdict on whether it's right. |
| 12 | `exit_criteria` | Project exit criteria | text | standard | Charter content: project exit criteria (the conditions to close or cancel the project or phase). | It sets the conditions for closing the project, or for cancelling it on purpose. | The project never ends on purpose. It just fades out, and nothing is learned from it. | Cancel if no screen is used on a real project for 30 days. |
| 13 | `pm_authority` | Assigned project manager, responsibility and authority | text | standard | Charter content: assigned project manager, responsibility and authority level. | It says who can make which calls without asking. | Every decision becomes a negotiation, or someone makes a call they didn't have the authority to make. | PM: you. Delivery resource implements but does not decide. |
| 14 | `sponsor` | Sponsor and authority | line | standard | Charter content: name and authority of the sponsor or other person(s) authorizing the charter. | The sponsor funds the project, protects it and signs the charter. | Nobody has the standing to approve, defend or end the project. | You. |
| 15 | `decision_rule` | Decision provenance rule | text | yours | Not in the standard. Your rule: a decision is only attributed to you with a quote and a date. | A decision record is only trustworthy if it shows where each decision came from. The four states: specified (you said it), agreed (you approved a summary), unobjected (proposed, no reply). Only the first two are decisions. | 'You decided X' gets said about things nobody decided. | Every decision-log entry carries one of the four states plus evidence. |
| 16 | `approval` | Charter sign-off | line | standard | Output of 4.1 Develop Project Charter: the charter is approved by the sponsor. | The signature is what turns the draft into an authorized project. | Planning work starts on something nobody authorized. | Approved by the sponsor, 2026-09-15. |

#### Stakeholder Register (Initiating)

*Screen intro:* The stakeholder register lists everyone who can affect the project or is affected by it, and records how much each one matters and how engaged they are now compared with how engaged they need to be.

*Screen source:* PMBOK Guide, 6th ed., process 13.1 Identify Stakeholders (Initiating process group). The engagement levels come from the Stakeholder Engagement Assessment Matrix used in 13.2 Plan Stakeholder Engagement.

| # | id | Label | Type | Origin | Source | Why a PM fills it in | If left blank | Example |
|---|---|---|---|---|---|---|---|---|
| 1 | `register` | Stakeholders | table: Name, Role in project, Expectations / requirements, Power (High / Low), Interest (High / Low), Current engagement (Unaware / Resistant / Neutral / Supportive / Leading), Desired engagement (Unaware / Resistant / Neutral / Supportive / Leading) | standard | Stakeholder register contents: identification information, assessment information and classification. | You can't manage expectations you haven't written down. Power and interest decide how much attention each person gets; the gap between current and desired engagement is the work. | The person with the most power over the outcome is the one you forgot to ask. | Archon \| Final reviewer \| Says whether it's right \| High \| High \| Neutral \| Supportive |
| 2 | `strategy` | Engagement notes | text | standard | 13.2 Plan Stakeholder Engagement: strategies to move stakeholders from current to desired engagement. | For every stakeholder whose current engagement is below the desired level, this says what you'll do about it. | The gaps in the table stay gaps. | Give the final reviewer the charter up front, so 'right' is judged against written criteria. |

## 8. Acceptance criteria for M2

The delivery team checks:
- [ ] Installs per-user from the Setup.exe, adds a Start Menu shortcut, and uninstalls cleanly
- [ ] Launches to the project list with record #1 present
- [ ] The Charter tab shows all 16 fields and the Stakeholder tab shows the register and notes, **all generated from the definition files**
- [ ] Changing a label or "why" in a definition file changes the app with no code change
- [ ] Focusing any field shows its source, why, if-blank and example; fields marked `yours` are visibly labeled as her own
- [ ] Every field and every table row has a source tag (three states plus untagged) and an evidence line
- [ ] Table rows can be added and removed; choice columns offer only their listed values
- [ ] Save writes a readable record file; unsaved changes are shown and you're warned on close
- [ ] New project creates an empty record

The PM checks (this is what accepts M2):
- [ ] Nyx opens record #1, clicks into every field, and understands what the panel says


## 9. What to hand back

When you finish or get blocked, give Nyx a report she can file in the project record:
1. What was built, mapped to §4 and the §8 checklist (met / not met)
2. Any question you had to resolve, phrased as a question for her approval, not as a decision
3. Anything that differs from this spec, and why
4. How she runs the acceptance check herself
