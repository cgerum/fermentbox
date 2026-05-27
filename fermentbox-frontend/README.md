# fermentbox

## Project setup
```
yarn install
```

### Compiles and hot-reloads for development
```
yarn serve
```

### Compiles and minifies for production
```
yarn build
```

### Lints and fixes files
```
yarn lint
```

### Runs unit tests
```
yarn test
```

### Runs unit tests in watch mode
```
yarn test:watch
```

### Runs unit tests with coverage report
```
yarn test:coverage
```

### API contract workflow
The API contract source of truth is `../api/fermentbox.openapi.yaml`.

Validate and regenerate API artifacts:
```
yarn api:check
```

Ensure generated API files are committed and in sync:
```
yarn api:verify-fresh
```

When adding or changing endpoints:
1. Update the OpenAPI file.
2. Run `yarn api:check`.
3. Update frontend usage code if needed.
4. Run tests and commit both source and generated artifacts.

### Customize configuration
See [Configuration Reference](https://cli.vuejs.org/config/).
