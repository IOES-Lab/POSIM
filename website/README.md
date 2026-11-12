# POSIM documentation website

Static documentation for the POSIM library in English, Korean, Spanish,
French, German, Italian, Japanese and Chinese.

## Build and preview

Requirements: Node.js 24 and pnpm 10.11.0. From this directory:

```sh
pnpm install --frozen-lockfile
pnpm build
pnpm check
pnpm preview
```

Open `http://127.0.0.1:4174` for English. Other languages use `/ko/`, `/es/`,
`/fr/`, `/de/`, `/it/`, `/ja/` and `/zh/`. Output is in `dist/`.

Run the browser checks after building:

```sh
pnpm exec playwright install chromium
pnpm test:browser
```

Playwright checks desktop and mobile navigation, language switching, search,
clipboard fallback and video loading/retry. Screenshots and the JSON report
are written under `.validation/`. The preview server supports MP4 byte ranges
for seeking. The two current-demo MP4s are tracked in `public/media/notion/`
and published with the site; playback uses browser-native controls with local
loading, buffering and retry feedback.

## Publish on GitHub Pages

The public documentation is published at
<https://ioes-lab.github.io/POSIM/>. English uses the root path; the other seven
languages use the same prefixes as the local preview.

The repository's Pages publishing source is **GitHub Actions**. The
`Documentation` workflow builds and checks all eight languages, then uploads only
`website/dist` and publishes it on pushes to `main`. Pull requests build and
check the site without publishing. The workflow can also be run manually.

Links, assets, search and language switching use relative paths so they work
under `/POSIM/`. `SITE_URL=https://ioes-lab.github.io/POSIM` retains that path
in the generated sitemap. Publish the built `dist/` directory.

## Other static hosts

Publish only `dist/`, preserving the directory structure. Pages are ordinary
HTML files; no SPA rewrites are needed. Each language has its own `index.html`.
Search reads a local JSON asset and does not need an external search service.

## Maintain the content

The header reads the POSIM release version from `package.json`. Update that
version when preparing a release and use the matching `v<version>` Git tag.
Follow the [release checklist](../docs/releases/README.md) to keep package
metadata, release notes and the changelog aligned.

Every documentation change needs the language-aware revision and fidelity review
described in [AGENTS.md](../AGENTS.md).

- `content/<language>/*.md`: complete guides for `en`, `ko`, `es`, `fr`, `de`,
  `it`, `ja` and `zh`; English is the default.
- `scripts/pages.mjs`: shared navigation order and reference mapping.
- `i18n/<language>.json`: page/group titles, interface text and media captions.
- `scripts/locales.mjs`: language definitions and translation completeness.
- `scripts/content.mjs`: shared executable examples and media expansion.
- `sources.json`: reference Notion pages for guide attribution.
- `catalog.json`: deployable world/object inventory; regenerate from a full
  POSIM checkout with `pnpm catalog` when catalog files change.
- `public/`: stylesheet, browser controls, vector favicon and local guide media.
- `media-sources.json`: Notion page/block provenance, original filenames and
  SHA-256 digests for guide media. Figures have localized captions.
  Keep signed download URLs out of this
  registry; deployed guides use the local files.

Use relative Markdown links such as `[Camera](camera.md)`. The builder converts
them to HTML links. Keep headings unique and descriptive. Mark code fences as
`xml` or `sdf` for SDF, `bash` or `sh` for terminal commands, and `python` for
Python examples. Syntax highlighting is generated with the HTML. Plain text
and unknown languages retain escaped, unhighlighted code. Code fences get a
localized copy button in the browser. If clipboard access is denied, the code
is selected and the UI prompts the user to copy with the keyboard.

Use `{{CODE:camera:0}}` to share the first fenced example from an English page;
indices start at zero. Update references when source blocks are reordered.
`{{MEDIA:camera}}` shares figures with the selected language's captions.
Write the surrounding prose in each language; missing pages fail the build.

Describe the current product in short sentences. Put values and limits in
bullets or tables. Keep all eight languages aligned, including links, units,
commands and supported behaviour. Use established robotics terminology from
relevant official documentation and write natural prose for each language.
Keep the root `README.md` and `README.<language>.md` files aligned as well.
Retain prerequisites, limitations and attribution.
Keep development notes, plans and review records outside the repository;
use GitHub issues and pull requests for change discussions. Regenerate the
site and check desktop/mobile readability after content updates.

`pnpm check` checks all eight languages, local files/anchors, search coverage,
public wording, unresolved Notion markup and old command identifiers. In a full repository it
also checks POSIM launch files, named worlds/descriptors, interface definitions,
repository source links and catalog consistency. A website-only deployment
performs the same site checks without requiring the ROS source tree.
