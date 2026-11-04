import { test, expect } from '@playwright/test';
import { pages } from '../scripts/pages.mjs';

test('desktop navigation, search, copy, language and shared emblem', async ({ page, context }, info) => {
  const errors = [];
  page.on('pageerror', error => errors.push(error.message));
  await context.grantPermissions(['clipboard-read', 'clipboard-write']);
  await page.goto('/');
  const emblem = page.locator('.docs-brand img');
  await expect(emblem).toHaveAttribute('src', 'assets/app-icon.svg');
  expect(await emblem.evaluate(image => image.complete && image.naturalWidth > 0)).toBe(true);
  const group = page.locator('.docs-sidebar h2').first();
  const style = await group.evaluate(node => ({ size: parseFloat(getComputedStyle(node).fontSize), color: getComputedStyle(node).color }));
  expect(style.color).toBe('rgb(255, 255, 255)');
  expect(style.size).toBeGreaterThan(await page.locator('.docs-sidebar nav a').first().evaluate(node => parseFloat(getComputedStyle(node).fontSize)));
  await expect.poll(() => page.locator('main img').evaluateAll(images => images.every(image => image.complete && image.naturalWidth > 0))).toBe(true);
  await page.locator('main img').evaluateAll(images => Promise.all(images.map(image => image.decode())));
  await page.screenshot({ path: info.outputPath('desktop-home.png'), fullPage: true });
  const search = page.locator('.docs-sidebar input[type=search]');
  await search.fill('sea pressure');
  await expect(page.locator('.docs-sidebar .docs-search-results a').first()).toContainText('Sea pressure');
  await search.press('Enter');
  await expect(page).toHaveURL(/\/pressure.html$/);
  const code = await page.locator('.docs-code-block pre code').first().textContent();
  await page.locator('.docs-code-copy').first().click();
  await expect(page.locator('.docs-code-copy').first()).toHaveText('Copied');
  expect((await page.evaluate(() => navigator.clipboard.readText())).replaceAll('\r\n', '\n')).toBe(code.replaceAll('\r\n', '\n'));
  await page.locator('[data-language-select]').selectOption('ko/pressure.html');
  await expect(page).toHaveURL(/\/ko\/pressure.html$/);
  await expect(page.locator('html')).toHaveAttribute('lang', 'ko');
  await expect(page.locator('.docs-brand img')).toHaveAttribute('src', '../assets/app-icon.svg');
  await page.screenshot({ path: info.outputPath('desktop-korean-reference.png'), fullPage: true });
  expect(errors).toEqual([]);
});

test('mobile menu, search, language switching and all page widths', async ({ page }, info) => {
  await page.setViewportSize({ width: 390, height: 844 });
  await page.goto('/');
  await page.locator('.docs-mobile-menu summary').click();
  await expect(page.locator('.docs-mobile-menu')).toHaveAttribute('open', '');
  await page.screenshot({ path: info.outputPath('mobile-menu.png') });
  await page.locator('.docs-mobile-menu nav a[href="sonar.html"]').click();
  await expect(page).toHaveURL(/\/sonar.html$/);
  await expect(page.locator('.docs-mobile-menu')).not.toHaveAttribute('open', '');
  await page.locator('[data-language-select]').selectOption('ko/sonar.html');
  await expect(page).toHaveURL(/\/ko\/sonar.html$/);
  await page.screenshot({ path: info.outputPath('mobile-korean-sonar.png') });
  for (const lang of ['', 'ko/']) {
    for (const entry of pages) {
      await page.goto(`/${lang}${entry.slug}.html`);
      expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth + 1), `${lang}${entry.slug} overflows`).toBe(true);
      await expect(page.locator('main h1')).toBeVisible();
      await expect(page.locator('.docs-brand img')).toBeVisible();
    }
  }
});

test('theme controls persist and follow system appearance on desktop and mobile', async ({ page }, info) => {
  await page.goto('/');
  const theme = page.locator('[data-theme-select]');
  await theme.selectOption('dark');
  await expect(page.locator('html')).toHaveAttribute('data-theme', 'dark');
  await page.reload();
  await expect(theme).toHaveValue('dark');
  await expect(page.locator('html')).toHaveAttribute('data-theme', 'dark');
  await page.screenshot({ path: info.outputPath('desktop-dark.png') });
  await page.setViewportSize({ width: 390, height: 844 });
  await expect(theme).toBeVisible();
  await expect(page.locator('.docs-brand img')).toBeVisible();
  await page.screenshot({ path: info.outputPath('mobile-dark.png') });
  await theme.selectOption('system');
  await page.emulateMedia({ colorScheme: 'light' });
  await expect(page.locator('html')).toHaveAttribute('data-theme', 'light');
  await page.emulateMedia({ colorScheme: 'dark' });
  await expect(page.locator('html')).toHaveAttribute('data-theme', 'dark');
  await theme.selectOption('light');
  await expect(page.locator('html')).toHaveAttribute('data-theme', 'light');
});

test('empty search, unavailable search, and denied clipboard remain usable', async ({ page }) => {
  await page.goto('/');
  const search = page.locator('.docs-sidebar input');
  await search.fill('zzno-such-posim-guidezz');
  await expect(page.locator('.docs-sidebar .docs-search-results')).toHaveText('No matching guides.');
  await search.press('Escape');
  await expect(search).toHaveValue('');
  await expect(page.locator('.docs-sidebar .docs-search-results')).toBeHidden();
  await page.route('**/search.en.json', route => route.abort());
  await page.reload();
  await search.fill('camera');
  await expect(page.locator('.docs-sidebar .docs-search-results')).toHaveText('Select a guide from the documentation menu.');
  await page.locator('.docs-sidebar nav a[href="camera.html"]').click();
  await page.evaluate(() => { navigator.clipboard.writeText = async () => { throw new DOMException('Denied', 'NotAllowedError'); }; });
  await page.locator('.docs-code-copy').first().click();
  await expect(page.locator('.docs-code-copy').first()).toHaveText('Text selected');
  expect((await page.evaluate(() => getSelection().toString())).trimEnd()).toBe((await page.locator('pre code').first().textContent()).trimEnd());
});

for (const lang of ['en', 'ko']) {
  test(`${lang} video delayed load, playback, pause, seek and retry`, async ({ page }, info) => {
    if (lang === 'ko') await page.setViewportSize({ width: 390, height: 844 });
    let release;
    const gate = new Promise(resolve => { release = resolve; });
    await page.route('**/currents-0e3c9419.mp4', async route => { await gate; await route.continue(); });
    await page.goto(lang === 'en' ? '/currents.html' : '/ko/currents.html');
    const video = page.locator('video').first();
    const feedback = page.locator('.docs-video-feedback').first();
    await video.scrollIntoViewIfNeeded();
    await video.evaluate(element => { element.muted = true; });
    const bounds = await video.boundingBox();
    // Click Chromium's native play control, including the narrow mobile layout.
    await video.click({ position: { x: 25, y: bounds.height - 48 } });
    await expect(feedback).toBeVisible();
    await expect(feedback).toContainText(lang === 'en' ? 'Loading video' : '영상을 불러오는 중');
    await page.screenshot({ path: info.outputPath(`${lang}-video-loading.png`) });
    release();
    await expect.poll(() => video.evaluate(element => element.currentTime), { timeout: 30000 }).toBeGreaterThan(0);
    await expect(feedback).toBeHidden();
    await video.evaluate(element => element.pause());
    await expect(feedback).toBeHidden();
    await video.evaluate(element => { element.currentTime = Math.min(5, element.duration / 2); });
    await expect.poll(() => video.evaluate(element => element.seeking)).toBe(false);
    await page.screenshot({ path: info.outputPath(`${lang}-video-paused.png`) });
    await page.unroute('**/currents-0e3c9419.mp4');
    await page.route('**/currents-0e3c9419.mp4', route => route.fulfill({ status: 404, body: 'Not found' }));
    await page.reload();
    await page.locator('video').first().evaluate(element => { element.muted = true; element.play().catch(() => {}); });
    const retry = feedback.getByRole('button', { name: lang === 'en' ? 'Retry' : '다시 시도' });
    await expect(retry).toBeVisible();
    await page.screenshot({ path: info.outputPath(`${lang}-video-error.png`) });
    await page.unroute('**/currents-0e3c9419.mp4');
    await retry.click();
    await expect.poll(() => video.evaluate(element => element.currentTime), { timeout: 30000 }).toBeGreaterThan(0);
    await expect(feedback).toBeHidden();
  });
}

test('preview serves seekable MP4 data and rejects invalid byte ranges', async ({ request }) => {
  const url = '/assets/media/notion/currents-0e3c9419.mp4';
  const response = await request.get(url, { headers: { Range: 'bytes=0-31' } });
  expect(response.status()).toBe(206);
  expect(response.headers()['content-type']).toBe('video/mp4');
  expect(response.headers()['content-range']).toMatch(/^bytes 0-31\/[0-9]+$/);
  expect((await response.body()).length).toBe(32);
  expect((await request.get(url, { headers: { Range: 'bytes=900000000-' } })).status()).toBe(416);
  expect((await request.head(url)).status()).toBe(200);
});
