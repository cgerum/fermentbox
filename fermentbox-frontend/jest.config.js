module.exports = {
    testEnvironment: "jsdom",
    moduleFileExtensions: ["js", "json", "vue"],
    transform: {
        "^.+\\.vue$": "@vue/vue2-jest",
        "^.+\\.js$": "babel-jest"
    },
    moduleNameMapper: {
        "^@/(.*)$": "<rootDir>/src/$1"
    },
    setupFilesAfterEnv: ["<rootDir>/tests/unit/setup.js"],
    testMatch: ["<rootDir>/tests/unit/**/*.spec.js"],
    collectCoverageFrom: [
        "src/**/*.{js,vue}",
        "!src/main.js",
        "!src/plugins/**"
    ]
};
