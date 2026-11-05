# Contributing

Contributions to POSIM's code and documentation are welcome. Start with a focused change that others can reproduce, and keep models, controller settings and guides consistent.

## Work in a branch

```bash
git clone https://github.com/IOES-Lab/POSIM.git
cd POSIM
git switch -c my-change
```

For simulation changes, use the [installation route](requirements.md) and [prepared terminal](terminals.md) for your system. Keep generated build/install trees and local credentials out of the commit.

## Validate the behavior you change

Choose a minimal relevant example. Confirm resource resolution, model spawn, advancing simulation time, received payloads, controller behavior when applicable and orderly shutdown. Include the source revision, renderer, architecture and command with the result.

Sensor and dynamics changes need an appropriate numerical comparison. A screenshot shows appearance; it does not establish a physical model's accuracy. Preserve upstream source attribution when editing inherited implementations.

## Update documentation

The site supports English, Korean, Spanish, French, German, Italian, Japanese and Chinese. English lives at `/`; the other languages use `/ko/`, `/es/`, `/fr/`, `/de/`, `/it/`, `/ja/` and `/zh/`. Keep the corresponding Markdown files in `website/content/<language>` aligned, including commands, units, prerequisites and limits. Use established robotics terminology and natural prose in each language.

Executable examples and media can use the shared `CODE` and `MEDIA` placeholders. The site requires a complete page set for every language and has no English body fallback. Page order is shared; world/object catalogs are generated from the source tree.

For a public capability, update the [feature catalogue](features.md) and its usage guide alongside the source. Document installation/activation, defaults, units, inputs/outputs, configuration scope, an observable example and model limits. Include opt-in effects and adapters; a package README alone does not make them discoverable in the site. Add new pages to `website/scripts/pages.mjs` and their titles to `website/i18n/<language>.json` so navigation and search include every language. Keep the repository's language-specific README files aligned too.

Describe the current product, its usage and limitations. Keep development notes, staged plans and validation reports outside the repository. Record change discussions and test results in GitHub issues and pull requests.

From `website`, with Node.js 24 and pnpm 10.11.0:

```bash
pnpm install --frozen-lockfile
pnpm build
pnpm check
pnpm preview
```

Open `http://127.0.0.1:4174`. Check links, command copying, search, language switching and the mobile navigation. Website hosting and maintainer details are in [website/README.md](https://github.com/IOES-Lab/POSIM/blob/main/website/README.md).

## Open a pull request

Explain the concrete problem, resulting behavior and relevant validation. Include commands/logs for simulation changes and a desktop/mobile preview for layout changes. State which checks you ran and which remain untested.

Use [GitHub issues](https://github.com/IOES-Lab/POSIM/issues) for reproducible defects and [pull requests](https://github.com/IOES-Lab/POSIM/pulls) for review. IOES-Lab maintains the project at [Korea Maritime & Ocean University](https://lab.wschoi.com).
