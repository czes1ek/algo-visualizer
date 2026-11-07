# CLAUDE.md — AlgoVisualizer

Project rules for anyone (human or AI) working in this repo. SPEC.md is what the project does, ARCHITECTURE.md is how it's built, and ROADMAP.md is what's left.

1. **No AI attribution, ever.** No `Co-Authored-By` trailers, "Generated with" lines or session links in commits, PRs, tags or releases. `.claude/settings.json` turns attribution off, and `.githooks/commit-msg` rejects it. Run `git config core.hooksPath .githooks` once per clone.
2. **Public repo, so nothing personal.** No phone numbers, home addresses, personal emails, security clearance or military details, and no absolute home paths. The only contact is `contact@casperbien.com`. Credit: Casper Bien · https://casperbien.com · https://github.com/czes1ek.
3. **No invented facts.** The README and release notes say only what exists and was measured.
4. **Code.** C++20. Zero warnings with `-Wall -Wextra -Wpedantic -Wconversion -Wshadow`; CI sets `-DALGOVIZ_WARNINGS_AS_ERRORS=ON`. `core/` never includes SFML or Emscripten headers. No raw `new`/`delete`, no globals. Format with `.clang-format`. New behaviour gets a test.
5. **Verify, don't claim.** Build, `ctest`, and check CI (`gh run list`) before saying something works.
6. **Git.** Conventional commits. Never force-push `main` or rewrite pushed history. Update CHANGELOG.md when you tag.
