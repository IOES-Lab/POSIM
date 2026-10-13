# Check formatting locally

Run these commands from the POSIM repository root on Ubuntu. They use the
versions and options in the [CI lint workflow](../../.github/workflows/lint.yml).
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
black --check --diff --line-length 99 --exclude tools/code_check .
flake8 --ignore=E203,W503,E501 --exclude=tools/code_check .
git ls-files -z '*.c' '*.cc' '*.cpp' '*.h' '*.hpp' '*.m' '*.mm' | \
  xargs -0 -r clang-format --dry-run --Werror --fallback-style=none
find . \( -iname '*.urdf' -o -iname '*.sdf' -o -iname '*.xacro' \
  -o -iname '*.xml' -o -iname '*.launch' -o -iname '*.world' \) \
  -print0 | xargs -0 xmllint --noout
git ls-files -z '*.sh' | xargs -0 -r shellcheck
find . \( -iname '*.yml' -o -iname '*.yaml' \) -print0 | \
  xargs -0 yamllint -d '{extends: default, rules: {line-length: {max: 120}, new-line-at-end-of-file: disable}}'
find . \( -iname 'CMakeLists.txt' -o -iname '*.cmake' \) \
  -print0 | xargs -0 cmake-lint
```

The C/C++ command checks formatting without rewriting files. The older
`lint_local.sh` helper formats C++ files in place and uses different options;
use the commands above when comparing your result with CI. In a new terminal,
activate the same virtual environment before running them.

For the documentation site, use the separate
[website build and checks](../../website/README.md).
