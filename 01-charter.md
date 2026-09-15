# PM Tool: Project Charter (v1.0, approved)

**Status:** **v1.0, approved by the sponsor 2026-09-15** ("charter approved"). Working title still open. Changes from here go through the change log. **Record #1** in the tool, per P5.
**Framework:** Hybrid (D5)

## How to read this

This file is both the **spec for the tool's first screen** and **this project's actual charter**. Each section heading is a field. Under each one:

- **Source** — where the field comes from.
- **Why** — why a PM fills it in, and what breaks when nobody does.
- **This project** — the entry for this project, with its provenance.

The field list follows the charter contents in PMBOK Guide 6th edition, "Develop Project Charter." Later editions (7th onward) moved away from prescriptive lists toward principles. Treat the 6th-edition list as the recognized checklist, not the only valid one. Fields the standard doesn't have are marked **your own**.

Status tags: ✅ filled from your words · ☑️ proposed and agreed by you (09-15, "yes to all proposals") · ⬜ empty, yours to fill in

---

### 1. Project title and purpose
**Source:** Initiating; charter, "project purpose."
**Why:** It's the "why are we doing this" everything else gets checked against. Without it, scope drifts, because nothing can be ruled out.
**This project:** ✅ Build a PM product that works as **a study guide, practical application, and future organization** (your words, 2026-09-15). Working title: none yet ⬜

### 2. Business case / justification
**Source:** Initiating; business documents, which feed the charter.
**Why:** It answers "why is this worth the time compared to anything else." Sponsors kill projects that don't have one.
**This project:** ☑️ You do PM work by intuition and want to understand how it fits into a PM job, which you're actively pursuing. No existing tool you have holds project-level truth: charter, scope, decisions, risks, and status against a plan.

### 3. Measurable objectives and success criteria
**Source:** Initiating; charter.
**Why:** It's how you'll know the project *worked*, which is a different question from whether it *shipped*. "Measurable" is the whole point. "Understand the role better" can't be tested.
**This project:** ☑️ Agreed criteria:
- ☑️ Every field in the tool has a source tag and a "why" explanation (P1, P2)
- ☑️ This project runs start to finish inside the tool, charter through closeout
- ☑️ At least one other real project is managed in it after this one
- ☑️ You can explain each PM artifact and when it's used without opening the tool

### 4. High-level requirements
**Source:** Initiating; charter. Detailed requirements come later, in Planning.
**Why:** Just enough to size the work and catch a wrong direction early. Detail at this stage is wasted effort.
**This project:** ☑️
- Guided input forms for each lifecycle artifact (P4)
- Built-in explanation for each field (P2)
- Decision log with a four-state source field (**your own**)
- Hybrid structure: predictive phases, with sprints inside Executing (D5)

### 5. High-level description, boundaries, key deliverables
**Source:** Initiating; charter.
**Why:** It draws the line around the project. **Boundaries (what's *out* of scope) prevent more scope creep than the in-scope list does.**
**This project:**
- Deliverable: ✅ **personal, installable desktop software, "like project man"** (your words, 09-15), covering the full lifecycle. Each person runs their own install.
- In scope: ☑️ the lifecycle artifacts in §4 and the record plan, one lifecycle phase at a time
- **Out of scope:** ☑️ ☑️ budget/cost accounting beyond one simple field · ☑️ multi-user permissions · ☑️ replacing ProjectMan's git tracking · ☑️ resource leveling / Gantt math

### 6. Overall project risk
**Source:** Initiating; charter. The full risk register comes in Planning.
**Why:** It sets the project's risk posture at the start, so bad news later isn't a surprise.
**This project:** See the project record §7: R1 learning your own tool instead of the job · R2 a fifth tracker · R4 scope creep.

### 7. Summary milestone schedule
**Source:** Initiating; charter.
**Why:** It sets dates for checkpoints, not tasks. Without them, "in progress" never ends.
**This project:** ✅ Schedule baseline (your dates, 09-15). Overall target: *"I want to TRY to get this done by October 15th."* The hedges are yours and are kept on purpose; these are targets, not commitments.
| Milestone | Target | Your words |
|---|---|---|
| M1 Charter approved | Tue Sep 15 | "hopefully today" |
| M2 Initiating screens work (charter, stakeholders) | Tue Sep 15 | "Hopefully today" |
| M3 Planning screens work (requirements, WBS, schedule, risks) | Wed Sep 16 | "tomorrow?" |
| M4 Executing works (sprint board, change log, status) | week of Sep 21 | "Next week?" |
| M5 Closeout; this project closes inside its own tool | **Thu Oct 15** | "Closeout- October 15" |

About three weeks of slack sit between M4 and M5. That's where success criterion #3 (managing a second real project in the tool) actually happens, so it isn't spare time.

### 8. Pre-approved financial resources
**Source:** Initiating; charter.
**Why:** It caps what can be spent without going back to the sponsor.
**This project:** ☑️ $0 beyond existing subscriptions and hosting.

### 9. Key stakeholder list
**Source:** Initiating; charter. Feeds the stakeholder register.
**Why:** Anyone affected who isn't listed turns into a surprise objection later.
**This project:**
| Stakeholder | Role | Source |
|---|---|---|
| You | Sponsor, PM, primary user | ✅ |
| Archon | **Final reviewer, with the verdict on whether it's right** (one review, when you think it's done); possible user on his own install | ✅ "Archon may use it, but he will definately review it" |
| Delivery team | Builds from the PM's handoffs | ✅ D18 |

Because the software is a personal install, Archon using it means running his own copy. Multi-user permissions stay out of scope.

### 10. Project approval requirements
**Source:** Initiating; charter.
**Why:** It defines what "done and accepted" means and **who signs off.**
**This project:** ☑️ You accept each milestone. Acceptance means you used the screen on this project and understood every field.
✅ **Final review by Archon** (your words, 09-15): *"he does a review when i think we are done, and he tells me if it is right or not."* This is a single gate at the end, before closeout, and his verdict decides whether the product is right.
🔶 Proposed: his review happens **once M4 is reached, not on Oct 15**, so there's time to fix what he finds. And he gets this charter up front, so "right" is judged against written criteria rather than discovered at review.

### 11. Project exit criteria
**Source:** Initiating; charter.
**Why:** It sets the conditions for closing *or cancelling* the project. Most personal projects never end on purpose. They just fade out.
**This project:** ✅ Approved ("exit criteria approved", 09-15):
- Close: M5 reached, and all four §3 success criteria met
- Cancel or re-charter: no screen used on a real project for 30 days, or the build pulls time off paid PM work for more than two weeks running

### 12. Assigned PM, responsibility and authority level
**Source:** Initiating; charter.
**Why:** It says who can make which calls without asking.
**This project:** ✅ PM: you. The delivery team builds from your handoffs. It implements; it doesn't decide (D18).

### 13. Sponsor and authority
**Source:** Initiating; charter.
**Why:** The sponsor funds the project, protects it, and approves the charter.
**This project:** ✅ You.

### 14. Decision provenance rule — **your own**
**Source:** Not in the standard. It comes from your 2026-08-27 rule about decisions being attributed to you without evidence.
**Why:** A decision record is only trustworthy if it shows where each decision came from.
**This project:** ✅ Three states: specified / agreed / unobjected. Only specified and agreed are decisions.
