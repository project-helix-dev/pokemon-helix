# Pokémon Helix — Build, Sync & Git Workflow

> Supersedes `compilation_steps.txt` at the pokegenics root. That file still has a
> longer troubleshooting/error list; this doc is the canonical process guide.

---

## 1. The Three Locations

| Location | Path | Role |
|---|---|---|
| Windows workspace | `C:\Users\mconn\OneDrive\Documents\pokegenics\pokeemerald-expansion\` | **Source of truth.** All edits (Cascade, Porymap) happen here. This is also the git repo. |
| WSL build tree | `~/pokegenics/pokeemerald-expansion/` | **Build-only copy.** Never edit here, never commit here. Builds take ~1 min natively vs 15+ min on `/mnt/c/`. |
| ROM output | `C:\Users\mconn\OneDrive\Documents\pokegenics\pokeemerald.gba` | What the emulator loads. Copied back from WSL after each build. |

**The OneDrive gotcha:** `rsync -a` compares mtime+size, and OneDrive sometimes
leaves a changed file with an *unchanged* mtime. Result: rsync silently skips it,
WSL builds stale code, and your change "doesn't work" even though the ROM built
fine. This is why the full-sync path always ends with the content-based verifier.

---

## 2. Sync → Build → Copy: The Two Paths

### Path A — Selective sync (fast; you know exactly what changed)

Use when: a coding session touched a small, known set of files (e.g. one `.c`
file and one `scripts.inc`).

```powershell
# 1. Copy each changed file explicitly (chain with &&):
wsl -e bash -c "cp /mnt/c/Users/mconn/OneDrive/Documents/pokegenics/pokeemerald-expansion/src/helix.c ~/pokegenics/pokeemerald-expansion/src/helix.c && cp /mnt/c/Users/mconn/OneDrive/Documents/pokegenics/pokeemerald-expansion/data/maps/HelixIsland/scripts.inc ~/pokegenics/pokeemerald-expansion/data/maps/HelixIsland/scripts.inc"

# 2. Build:
wsl -e bash -c "cd ~/pokegenics/pokeemerald-expansion && make -j4 2>&1 | tail -20"

# 3. Copy ROM back:
wsl -e bash -c "cp ~/pokegenics/pokeemerald-expansion/pokeemerald.gba /mnt/c/Users/mconn/OneDrive/Documents/pokegenics/pokeemerald.gba"
```

Pros: no tree scan, no spurious timestamp churn, minimal recompilation.
Cons: you must not forget a file — if a change doesn't show up in-game, a missed
file is the #1 suspect. When in doubt, `git status --short` on the Windows side
lists everything modified.

### Path B — Full sync + verify (safe; big sessions, Porymap edits, or unsure)

Use when: a long edit session touched many files, Porymap was used (it writes
maps *and* layouts), or you simply aren't certain what changed.

```powershell
# 1. Full rsync (excludes build artifacts and .git):
wsl -e bash -c "rsync -a --exclude='build/' --exclude='.git/' /mnt/c/Users/mconn/OneDrive/Documents/pokegenics/pokeemerald-expansion/ ~/pokegenics/pokeemerald-expansion/"

# 2. REQUIRED: content-based verifier (catches OneDrive mtime skips, copies mismatches):
wsl -e bash /mnt/c/Users/mconn/OneDrive/Documents/pokegenics/tools_verify_sync.sh

# 3. Build:
wsl -e bash -c "cd ~/pokegenics/pokeemerald-expansion && make -j4 2>&1 | tail -20"

# 4. Copy ROM back:
wsl -e bash -c "cp ~/pokegenics/pokeemerald-expansion/pokeemerald.gba /mnt/c/Users/mconn/OneDrive/Documents/pokegenics/pokeemerald.gba"
```

`tools_verify_sync.sh` compares every source file *by content* and copies any
mismatch, while **excluding build-generated files** (which must be produced on
the WSL side by make, not overwritten with stale Windows copies). It prints
`FIXED: <file>` per repair and `TOTAL_FIXED=n` at the end. `TOTAL_FIXED=0`
means the rsync was actually complete.

### Which path, when?

| Scenario | Path |
|---|---|
| Small code fix (1–5 known files) | A |
| Long Cascade session, many edits | B |
| Any Porymap session (save first! syncs `data/maps/` + `data/layouts/`) | B |
| New map / tileset / object events added | B |
| Build succeeded but change not visible in-game | B (a file was missed) |
| After `git pull` / merge from upstream | B |

### Build variants

```powershell
# Full error output (debugging build failures):
wsl -e bash -c "cd ~/pokegenics/pokeemerald-expansion && make -j4 2>&1"

# Targeted clean of generated map/layout files (Porymap weirdness):
wsl -e bash -c "cd ~/pokegenics/pokeemerald-expansion && make clean-assets && make -j4 2>&1 | tail -20"

# Full clean rebuild (~5–10 min, last resort):
wsl -e bash -c "cd ~/pokegenics/pokeemerald-expansion && make clean && make -j4 2>&1 | tail -20"
```

### Never hand-edit generated files

These are regenerated on every WSL build; edit the *source* instead:

| Generated file | Edit this instead |
|---|---|
| `include/constants/region_map_sections.h` | `src/data/region_map/region_map_sections.constants.json.txt` |
| `include/constants/map_groups.h`, `layouts.h`, `map_event_ids.h`, `heal_locations.h` | the relevant `map.json` / `layouts.json` / `heal_locations.json` |
| `src/data/trainers.h` | `src/data/trainers.party` |
| `src/data/wild_encounters.h` | `src/data/wild_encounters.json` |
| `data/layouts/layouts*.inc`, `data/maps/**/​*.inc` (header/events/connections) | `map.json` / `layouts.json` |

Full list: `grep AUTO_GEN_TARGETS Makefile *.mk`

---

## 3. Git Workflow

### 3.1 Current state (as of writing)

- The Windows workspace **is the git repo**. The WSL tree is a throwaway build
  copy — never run git commands there.
- Remote `origin` points at the **public upstream**:
  `https://github.com/rh-hideout/pokeemerald-expansion.git`. You cannot push
  there (no write access), and you would not want to.
- All Helix work lives on the local branch **`helix-dev`**, branched from
  upstream master at commit `c2d370f2c0` (post-1.13 era, Apr 2026).
- There is currently **one Helix commit** (`c0ba76c099 "Start Pokémon Helix
  development"`) plus a large uncommitted working set. Until this is pushed to
  a remote you control, the only copy is on this machine (OneDrive is the only
  backup).

### 3.2 One-time setup: your own remote

A GitHub *fork* of a public repo is itself always public. Both options work;
pick based on whether you want the project visible:

**Option 1 — public fork (standard):**
1. On GitHub, fork `rh-hideout/pokeemerald-expansion` to your account.
2. Rewire remotes so `origin` = yours, `upstream` = RHH:
   ```powershell
   git remote rename origin upstream
   git remote add origin https://github.com/<your-user>/pokeemerald-expansion.git
   git push -u origin helix-dev
   ```

**Option 2 — private repo (not a GitHub "fork", but same effect):**
1. On GitHub, create a new **empty private** repo (e.g. `pokemon-helix`), no README.
2. ```powershell
   git remote rename origin upstream
   git remote add origin https://github.com/<your-user>/pokemon-helix.git
   git push -u origin helix-dev
   ```

Either way, after setup: `origin` = where you push Helix work, `upstream` =
where you pull RHH updates from. Nothing about the sync/build process changes.

### 3.3 Day-to-day committing

Commit **after a feature builds and is verified in-game** — the working tree is
your only undo layer between commits, so don't let months of work pile up like
the current uncommitted set. Suggested rhythm: one commit per working feature
or fix, from the Windows side:

```powershell
git status --short                      # review what changed
git add <files...>                      # or: git add -A  after review
git commit -m "helix: <what and why>"
git push
```

Conventions for this project:
- Prefix Helix commits with `helix:` so they're trivially distinguishable from
  upstream commits in `git log` (e.g. `helix: fix intro cutscene spawn point`).
- Generated files (`map_event_ids.h`, `data/maps/**/events.inc`, etc.) are
  *tracked by upstream*, so they will show as modified. Committing them is
  fine and expected — just never hand-edit them (see §2).
- Do **not** commit: `pokeemerald.gba` (lives outside the repo anyway), `.sav`
  files, anything under `build/`.
- The `?? tools/create_helix_*.py` scripts, `data/maps/Helix*/`, `src/helix*.c`
  etc. currently untracked **should be added and committed** — they are source.

### 3.4 Versioning for this project

- **Base version pinning:** Helix is based on pokeemerald-expansion at
  `c2d370f2c0` (upstream master, between official releases). Record any future
  rebase/merge point in the commit message so "what expansion version are we
  on?" is always answerable via `git log --merges` or this doc.
- **Milestone tags:** tag playable checkpoints so you can always rebuild an
  exact ROM:
  ```powershell
  git tag -a helix-v0.1 -m "Demo: island loop + nexus runs"
  git push origin helix-v0.1
  ```
  Suggested scheme: `helix-v0.x` during the demo era, `helix-v1.0` for the
  first feature-complete build. Tag *after* a verified build, and note the
  demo-tuning state (see `// DEMO:` markers in `src/helix.c`) in the tag message.
- **Pulling upstream updates (optional, do rarely):**
  ```powershell
  git fetch upstream
  git merge upstream/master        # on helix-dev
  ```
  Expect conflicts in heavily-modified files (`pokemon.c`, `overworld.c`,
  `battle_*`, object-event graphics tables). Only worth it for major expansion
  features/fixes you actually want; each merge is a mini-project. After any
  merge: **full sync (Path B) + `make clean` rebuild.**
- **Observing progress:** `git log --oneline --graph helix-dev ^upstream/master`
  shows exactly the Helix-only commit history; `git diff upstream/master --stat`
  shows the total footprint of the hack at any moment.
