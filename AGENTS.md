# Working on POSIM

Read the instructions and checks for the files you change. State uncertain
assumptions before editing, keep changes focused on the request, and verify the
result. Do not mix unrelated refactoring with documentation work.

## Public product documentation

POSIM is public product source. Keep dated experiment reports, comparison results,
V&V captures, investigation scripts and development history in a separate
development archive. Retain product implementation, reusable automated tests and
current user/contributor documentation here. Describe how a feature works;
preserve investigation and repair history in the private record.

POSIM is the simulation engine. WWW-POSIM is the application package that uses it.
For public README files, website guides and generated pages, follow
[website/README.md](website/README.md) and the revision process below. Keep all
eight supported language editions aligned, regenerate the website and run its
documentation checks.

## Documentation revision is required

Every new or changed document must receive a language-aware revision pass before
it is considered finished. This includes root and package READMEs, public guides
in `website/content/`, and reader-facing strings in `website/i18n/`. Improve
unclear, repetitive or unnatural wording; leave clear prose unchanged rather
than rewriting it to meet a quota.

1. Identify the audience, document language, existing register and technical
   source of truth. Select the language from the document, not from the user's
   request. POSIM's public site supports `en`, `ko`, `ja`, `zh`, `de`, `fr`, `es`
   and `it`.
2. Use the matching profile from the
   [multilingual slop-rewrite guide](https://github.com/woensug-choi/slop-rewrite/blob/6b9dc8369d69a9c131bd2ba0b284dd4b1ca53ea8/multilingual/README.md).
   Read its language-specific instructions before revising. If the tool is
   unavailable, perform the same editorial and fidelity review manually and
   disclose that automated checks were not run. It is an editorial aid, not a
   runtime dependency or a substitute for reading the text.
3. Preserve claims, technical names, prerequisites, limitations, negation,
   conditions, units, numbers and certainty. Do not turn an optional capability
   into a default or a possibility into a guarantee. Keep code, commands,
   identifiers, tables, quotations, citations, link targets, template tokens and
   heading anchors unchanged unless the requested change requires otherwise.
   Check any intentional change against the source and explain it in the PR.
4. Review the result in its own language. Keep English spelling conventions;
   Korean particles, spacing and sentence endings; Japanese particles and
   politeness; Chinese script and modifier order; German case and verb order;
   French agreement and register; Spanish address forms and agreement; and
   Italian agreement and prepositions natural and consistent. Do not impose one
   language's sentence patterns on another.
5. Compare the complete diff with the original and check translated passages
   against their technical source. Use the tool's explicit `--lang` and `--orig`
   options when available. A clean scan does not prove semantic fidelity or
   translation quality. Keep the eight language editions and root READMEs
   aligned when facts change.
6. Follow `website/README.md` for document structure, build, link and browser
   checks. Report the scope reviewed, meaningful edits, retained wording, checks
   actually run and any unresolved questions in the PR. Keep local review notes,
   plans and validation artifacts outside the repository. Do not claim success
   for skipped or blocked checks.

Describe the current product in public guides. Preserve attribution and license
notices, and do not add implementation history or unsupported promotional claims.
