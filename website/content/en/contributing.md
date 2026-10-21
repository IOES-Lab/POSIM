# Contributing

Contributions to POSIM's code and documentation are welcome. Start with a focused change that others can reproduce, and keep models, controller settings and guides consistent.

## Work in a branch

```bash
git clone https://github.com/IOES-Lab/POSIM.git
cd POSIM
git switch -c my-change
```

Use the matching [Ubuntu](install.md) or [Docker](docker.md) environment for simulation changes. Keep generated build/install trees and local credentials out of the commit.

## Validate the behavior you change

Choose a minimal relevant example. Confirm resource resolution, model spawn, advancing simulation time, received payloads, controller behavior when applicable and orderly shutdown. Include the source revision, renderer, architecture and command with the result.

Sensor and dynamics changes need an appropriate numerical comparison. A screenshot shows appearance; it does not establish a physical model's accuracy. Preserve upstream source attribution when editing inherited implementations.

## Update documentation

English is the default language at `/`; Korean pages live at `/ko/`. Edit paired Markdown files under `website/content/en` and `website/content/ko`. The site maintains a common page order and generates the world/object catalogs from the source tree.

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
