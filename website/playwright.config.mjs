import { defineConfig } from '@playwright/test';

export default defineConfig({
  testDir: './tests',
  workers: 1,
  timeout: 60000,
  expect: { timeout: 10000 },
  outputDir: './.validation/browser-results',
  reporter: [['list'], ['json', { outputFile: './.validation/browser-results.json' }]],
  use: {
    baseURL: 'http://127.0.0.1:4184',
    viewport: { width: 1440, height: 1000 },
    screenshot: 'only-on-failure',
    trace: 'retain-on-failure',
  },
  webServer: {
    command: 'node scripts/preview.mjs',
    url: 'http://127.0.0.1:4184',
    env: { PORT: '4184' },
    reuseExistingServer: false,
  },
});
