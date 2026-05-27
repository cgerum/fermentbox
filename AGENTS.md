# Agent Workflow Notes

## Validation After Changes

- After making code changes, always run the relevant lint command before finishing.
- If only frontend files changed, run `cd fermentbox-frontend && yarn lint`.
- If multiple areas changed, run lint checks for each changed area where lint tooling exists.
- Report lint results in your final response. If lint fails, either fix issues or clearly explain what remains.
