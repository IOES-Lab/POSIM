Contributions via pull requests are much appreciated. Before sending us a pull request, please ensure that:

1. Limited scope. Your PR should do one thing or one set of things. Avoid adding “random fixes” to PRs. Put those on separate PRs.
2. Give your PR a descriptive title. Add a short summary, if required.
3. Make sure the pipeline is green.
4. Don’t be afraid to request reviews from maintainers.
5. New code = new tests. If you are adding new functionality, always make sure to add some tests exercising the code and serving as live documentation of your original intention.

To send us a pull request, please:

- [ ] Fork the repository.
- [ ] Modify the source; please focus on the specific change you are contributing. If you also reformat all the code, it will be hard for us to focus on your change.
- [ ] Ensure local tests pass. Run `colcon test` from the sourced workspace and inspect `colcon test-result --verbose` for failures.
- [ ] Run the checks in the [local lint guide](https://github.com/IOES-Lab/POSIM/blob/main/extras/lint_local/README.md). For documentation changes, also follow the [website build and checks](https://github.com/IOES-Lab/POSIM/blob/main/website/README.md).
- [ ] Commit to your fork using clear commit messages.
- [ ] Send a pull request, answering any default questions in the pull request interface.
- [ ] Pay attention to any automated CI failures reported in the pull request, and stay involved in the conversation.
