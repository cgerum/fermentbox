const fs = require("fs");
const path = require("path");
const YAML = require("yaml");

const rootDir = path.resolve(__dirname, "../..");
const specPath = path.join(rootDir, "api", "fermentbox.openapi.yaml");
const outputPath = path.resolve(__dirname, "../src/services/api-generated.js");

function normalizePath(rawPath) {
  return rawPath.startsWith("/") ? rawPath.slice(1) : rawPath;
}

function buildOperations(spec) {
  const operations = {};

  Object.entries(spec.paths || {}).forEach(([apiPath, methods]) => {
    Object.entries(methods).forEach(([method, operation]) => {
      const methodUpper = method.toUpperCase();
      const operationId = operation.operationId;

      if (!operationId) {
        throw new Error(`Missing operationId for ${methodUpper} ${apiPath}`);
      }

      const queryParameters = (operation.parameters || [])
        .filter((parameter) => parameter.in === "query")
        .map((parameter) => parameter.name);

      const hasBody = !!operation.requestBody;

      operations[operationId] = {
        path: normalizePath(apiPath),
        method: methodUpper,
        queryParameters,
        hasBody
      };
    });
  });

  return operations;
}

function generateFileContent(operations) {
  return `/* eslint-disable */\n// This file is auto-generated from api/fermentbox.openapi.yaml\n// Do not edit manually.\n\nconst operations = ${JSON.stringify(operations, null, 2)};\n\nexport default operations;\n`;
}

function main() {
  const specRaw = fs.readFileSync(specPath, "utf8");
  const spec = YAML.parse(specRaw);

  const operations = buildOperations(spec);
  const content = generateFileContent(operations);

  fs.writeFileSync(outputPath, content);
  process.stdout.write(`Generated ${path.relative(process.cwd(), outputPath)}\n`);
}

main();
