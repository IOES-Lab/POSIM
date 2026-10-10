# POSIM documentation website

Static English/Korean documentation for the POSIM library.

## Build and preview

Requirements: Node.js 24 and pnpm 10.11.0. From this directory:

```sh
pnpm install --frozen-lockfile
pnpm build
pnpm check
pnpm preview
```

Open `http://127.0.0.1:4174` for English, or
`http://127.0.0.1:4174/ko/` for Korean. Output is in `dist/`.

## Publish on GitHub Pages

The public documentation is published at
<https://ioes-lab.github.io/POSIM/> (English) and
<https://ioes-lab.github.io/POSIM/ko/> (Korean).

The repository's Pages publishing source is **GitHub Actions**. The
`Documentation` workflow builds and checks both languages, then uploads only
`website/dist` and publishes it on pushes to `main`. Pull requests build and
check the site without publishing. The workflow can also be run manually.

Links, assets, search and language switching use relative paths so they work
under `/POSIM/`. `SITE_URL=https://ioes-lab.github.io/POSIM` retains that path
in the generated sitemap. Publish only the built `dist/`, never the repository
root or internal validation records.

## Other static hosts

Publish only `dist/`, preserving the directory structure. Pages are ordinary
HTML files; no SPA rewrites are needed. Each language has its own `index.html`.
Search reads a local JSON asset and does not need an external search service.

## Maintain the content

- `content/en/*.md`: default English guides.
- `content/ko/*.md`: matching Korean guides.
- `scripts/pages.mjs`: shared navigation order, titles and reference mapping.
- `sources.json`: reference Notion pages and reviewed POSIM source revision.
- `catalog.json`: deployable world/object inventory; regenerate from a full
  POSIM checkout with `pnpm catalog` when catalog files change.
- `public/`: stylesheet, browser controls, vector favicon and local guide media.
- `media-sources.json`: Notion page/block provenance, original filenames, SHA-256
  digests and the public/engineering audience for each source figure. Public
  figures must appear in both languages. Keep signed download URLs out of this
  registry; deployed guides use the local files.

Use relative Markdown links such as `[Camera](camera.md)`. The builder converts
them to HTML links. Keep headings unique and descriptive. Code fences get a
localized copy button in the browser. If clipboard access is denied, the code
is selected and the UI prompts the user to copy with the keyboard.

Follow [AGENTS.md](AGENTS.md) for public wording. Explain current behaviour in
short sentences, and put values and limits in bullets or tables. Keep release
history and review notes outside the user guides. Update English and Korean
together. The wording check catches known regressions; review meaning and
desktop/mobile readability as well.

`pnpm check` checks both languages, local files/anchors, search coverage,
public wording, unresolved Notion markup and old command identifiers. In a full repository it
also checks POSIM launch files, named worlds/descriptors, interface definitions,
repository source links and catalog consistency. A website-only deployment
performs the same site checks without requiring the ROS source tree.
