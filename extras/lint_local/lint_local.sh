#!/usr/bin/env bash
# Check tracked source files with the same commands locally and in CI.
set -uo pipefail

usage() {
  printf '%s\n' \
    'Usage: bash extras/lint_local/lint_local.sh [CHECK]' \
    'CHECK: all (default), black, flake8, clang-format, xml, shellcheck, yamllint, cmake-lint' \
    'Checks are read-only. Stage new files with git add to include them.'
}

if (( $# > 1 )); then
  usage >&2
  exit 2
fi
selected=${1:-all}
case "$selected" in
  -h|--help) usage; exit 0 ;;
  all|black|flake8|clang-format|xml|shellcheck|yamllint|cmake-lint) ;;
  *) usage >&2; exit 2 ;;
esac

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd) || exit 2
repo_root=$(git -C "$script_dir" rev-parse --show-toplevel) || exit 2
cd -- "$repo_root" || exit 2

python_files=() cpp_files=() xml_files=() shell_files=() yaml_files=() cmake_files=()
while IFS= read -r -d '' file; do
  # Deleted tracked files are absent from the worktree. Prefix paths so a
  # filename beginning with '-' cannot become a linter option.
  [[ -f "$file" ]] || continue
  case "$file" in
    tools/code_check/*) ;;
    *.py) python_files+=("./$file") ;;
  esac
  case "$file" in
    *.c|*.cc|*.cpp|*.cxx|*.h|*.hh|*.hpp|*.hxx|*.m|*.mm) cpp_files+=("./$file") ;;
    *.urdf|*.sdf|*.xacro|*.xml|*.launch|*.world) xml_files+=("./$file") ;;
    *.sh) shell_files+=("./$file") ;;
    *.yaml|*.yml) yaml_files+=("./$file") ;;
    CMakeLists.txt|*/CMakeLists.txt|*.cmake) cmake_files+=("./$file") ;;
  esac
done < <(git ls-files -z)

result=0
run_check() {
  local name=$1 count=$2
  shift 2
  [[ "$selected" == all || "$selected" == "$name" ]] || return 0
  printf '\n%s (%s tracked files)\n' "$name" "$count"
  if (( count == 0 )); then
    return 0
  fi
  if ! command -v "$1" >/dev/null 2>&1; then
    printf 'Missing tool: %s. See extras/lint_local/README.md.\n' "$1" >&2
    result=1
  elif ! "$@"; then
    result=1
  fi
}

run_check black "${#python_files[@]}" \
  black --check --diff --line-length 99 "${python_files[@]}"
run_check flake8 "${#python_files[@]}" \
  flake8 --ignore=E203,W503,E501 "${python_files[@]}"
run_check clang-format "${#cpp_files[@]}" \
  clang-format --dry-run --Werror --fallback-style=none "${cpp_files[@]}"
run_check xml "${#xml_files[@]}" xmllint --nonet --noout "${xml_files[@]}"
run_check shellcheck "${#shell_files[@]}" shellcheck "${shell_files[@]}"
run_check yamllint "${#yaml_files[@]}" yamllint \
  -d '{extends: default, rules: {line-length: {max: 120}, new-line-at-end-of-file: disable}}' \
  "${yaml_files[@]}"
run_check cmake-lint "${#cmake_files[@]}" cmake-lint "${cmake_files[@]}"

if (( result == 0 )); then
  printf '\nAll selected checks passed.\n'
else
  printf '\nOne or more checks failed.\n' >&2
fi
exit "$result"
