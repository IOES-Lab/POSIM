# POSIM documentation website

Static English/Korean documentation for the POSIM library. The layout and
typography follow the WWW-POSIM documentation: a light reading surface,
grouped navigation, an on-page contents list, copyable commands and responsive
mobile navigation. No simulator, account server or Notion API runs on the host.

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
No Vercel account, simulator, server secret or paid hosting plan is needed.

Links, assets, search and language switching use relative paths so they work
under `/POSIM/`. `SITE_URL=https://ioes-lab.github.io/POSIM` retains that path
in the generated sitemap. Publish only the built `dist/`, never the repository
root or internal validation records.

## Optional Vercel deployment

1. Import `IOES-Lab/POSIM` as a new Vercel project and select the documentation
   branch for a preview, or `main` after the documentation is merged.
2. Set **Root Directory** to `website` and **Framework Preset** to `Other`.
3. Use Node.js **24.x**. The checked-in `vercel.json` sets the pinned pnpm
   installation, build/check command and `dist` output directory.
4. Deploy. Test `/`, `/ko/`, a plugin page, search and code copying over HTTPS.
5. Optionally set `SITE_URL` to the production origin and redeploy to generate
   `sitemap.xml` and `robots.txt` for that address.

The committed source catalog lets `website/` build independently. Vercel does
not need source files outside the Root Directory, model meshes or a ROS build.
No secret/environment credential is required. Future Git pushes are built by
the connected Vercel project according to its deployment settings.

See [Vercel build configuration](https://vercel.com/docs/builds/configure-a-build)
and [Node.js versions](https://vercel.com/docs/functions/runtimes/node-js/node-js-versions).
This configuration prepares deployment; it does not provision a Vercel project.

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
- `public/`: stylesheet, browser controls and vector favicon.

Use relative Markdown links such as `[Camera](camera.md)`. The builder converts
them to HTML links. Keep headings unique and descriptive. Code fences get a
localized copy button in the browser. If clipboard access is denied, the code
is selected and the UI prompts the user to copy with the keyboard.

`pnpm check` checks both languages, local files/anchors, search coverage,
unresolved Notion markup and old command identifiers. In a full repository it
also checks POSIM launch files, named worlds/descriptors, interface definitions,
repository source links and catalog consistency. A website-only deployment
performs the same site checks without requiring the ROS source tree.

## Source reconciliation and review record

The content adapts the supplied public IOES-Lab POSIM Notion Wiki (21 pages,
reviewed 2026-10-06) to POSIM source `144d55e`. The source pages remain linked
in the relevant page footers. The adaptation:

- uses current `posim_*` package, namespace and launch names;
- describes merged external waves and the ARM64 WAM-V integration;
- separates source builds from older, differently named candidate images;
- checks topic/interface units against implementation;
- describes the current Santorini Fuel include rather than assuming the old
  TIFF is directly loaded by that world;
- keeps citations and inherited source attribution;
- excludes unavailable Notion attachment URLs and historical captures from
  new-run evidence.

Public pages explain using the current library. Maintainer build/deploy details
and evidence boundaries stay in this file and pull-request validation records.
WWW-POSIM accounts, installers, courses and leaderboards are separate product
documentation. This documentation change does not execute a new ROS/Gazebo
physics acceptance matrix; runnable instructions are source-checked rather
than advertised as a newly performed hardware validation.
