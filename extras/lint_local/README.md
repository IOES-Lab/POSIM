# Check formatting locally

Run these commands from the POSIM repository root on Ubuntu. The
[CI lint workflow](../../.github/workflows/lint.yml) runs the same helper.
Install Python tools in a virtual environment rather than the system Python.

## Prepare the tools

```bash
sudo apt-get update
sudo apt-get install -y python3-venv libxml2-utils shellcheck
python3 -m venv "$HOME/.venvs/posim-lint"
source "$HOME/.venvs/posim-lint/bin/activate"
python -m pip install black==24.10.0 flake8==7.1.1 clang-format==19.1.6 \
  cmakelang==0.6.13 yamllint==1.35.1
```

## Run the checks

```bash
bash extras/lint_local/lint_local.sh
```

The helper checks Python, C/C++ (including `.hh` headers), XML/SDF, shell, YAML
and CMake files without rewriting them. It runs every selected check and returns
a nonzero status if any check fails or a required tool is missing.

Only Git-tracked files are checked, including unstaged edits. Use `git add` on
new source files before checking them. Build output, installed packages and
untracked local files are excluded. To run one check, append its name, such as
`black`, `clang-format` or `shellcheck`. In a new terminal, activate the same
virtual environment first.

For the documentation site, use the separate
[website build and checks](../../website/README.md).
