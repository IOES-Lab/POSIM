import { test, expect, devices } from '@playwright/test';
import { pages } from '../scripts/pages.mjs';
import { languages, messages } from '../scripts/locales.mjs';

// Scrollbars must remain visible in headless screenshots for navigation review.
test.use({ launchOptions: { ignoreDefaultArgs: ['--hide-scrollbars'] } });

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
  expect(style.color).toBe(await page.locator('body').evaluate(node => getComputedStyle(node).color));
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

test('mobile menu, search and language switching', async ({ page }, info) => {
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
});

test('mobile header keeps the brand and controls on one row in every language', async ({ page }, info) => {
  for (const { code, directory } of languages) {
    await page.goto(`/${directory}index.html`);
    for (const width of [320, 360, 390, 600, 768, 960]) {
      await page.setViewportSize({ width, height: 844 });
      for (const theme of ['system', 'light', 'dark']) {
        await page.locator('[data-theme-select]').selectOption(theme);
        const layout = await page.locator('.docs-header').evaluate(header => {
          const bounds = header.getBoundingClientRect();
          const controls = [...header.querySelectorAll('.docs-brand, .docs-control, nav>a')];
          const canvas = document.createElement('canvas');
          const context = canvas.getContext('2d');
          return {
            width: bounds.width,
            height: bounds.height,
            right: bounds.right,
            boxes: controls.map(node => {
              const rect = node.getBoundingClientRect();
              return { left: rect.left, right: rect.right, height: rect.height, center: rect.y + rect.height / 2 };
            }),
            labels: [...header.querySelectorAll('select')].map(node => {
              const style = getComputedStyle(node);
              context.font = style.font;
              return {
                text: node.selectedOptions[0].textContent,
                available: node.clientWidth - parseFloat(style.paddingLeft) - parseFloat(style.paddingRight),
                needed: context.measureText(node.selectedOptions[0].textContent).width,
              };
            }),
          };
        });
        const description = `${code}, ${width}px, ${theme}`;
        expect(layout.boxes, description).toHaveLength(4);
        expect(layout.height, description).toBe(58);
        for (const [index, box] of layout.boxes.entries()) {
          expect(Math.abs(box.center - layout.boxes[0].center), description).toBeLessThanOrEqual(1);
          expect(box.height, description).toBeGreaterThanOrEqual(24);
          expect(box.left, description).toBeGreaterThanOrEqual(0);
          expect(box.right, description).toBeLessThanOrEqual(layout.right);
          if (index) expect(box.left, description).toBeGreaterThanOrEqual(layout.boxes[index - 1].right);
        }
        for (const label of layout.labels) {
          expect(label.available + 1, `${description}: ${label.text} is clipped`).toBeGreaterThanOrEqual(label.needed);
        }
        expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth), description).toBe(true);
        await expect(page.locator('.docs-mobile-menu')).toHaveCSS('top', '58px');
        if ((code === 'en' && width === 390 && theme !== 'system') ||
            (code === 'ja' && width === 320 && theme === 'light') ||
            (code === 'ko' && width === 390 && theme === 'light')) {
          await page.screenshot({ path: info.outputPath(`header-${code}-${width}-${theme}.png`) });
        }
      }
    }
  }
});

test('compact mobile header preserves keyboard order and sticky menu position', async ({ page }) => {
  await page.setViewportSize({ width: 320, height: 844 });
  await page.goto('/ko/index.html');
  const brand = page.locator('.docs-brand');
  const language = page.locator('[data-language-select]');
  const theme = page.locator('[data-theme-select]');
  const github = page.locator('.docs-header nav>a');
  await brand.focus();
  for (const control of [language, theme, github]) {
    await page.keyboard.press('Tab');
    await expect(control).toBeFocused();
    await expect(control).toHaveCSS('outline-style', 'solid');
  }
  await expect(language).toHaveAttribute('aria-label', messages.ko.ui.documentationLanguage);
  await expect(theme).toHaveAttribute('aria-label', messages.ko.ui.colorTheme);
  await expect(github).toHaveAttribute('href', 'https://github.com/IOES-Lab/POSIM');
  await theme.selectOption('dark');
  await expect(page.locator('html')).toHaveAttribute('data-theme', 'dark');
  await language.selectOption('../ja/index.html');
  await expect(page.locator('html')).toHaveAttribute('lang', 'ja');
  await expect(page.locator('html')).toHaveAttribute('data-theme', 'dark');
  await page.evaluate(() => scrollTo(0, 500));
  const header = await page.locator('.docs-header').boundingBox();
  const summary = page.locator('.docs-mobile-menu summary');
  expect(header.y).toBe(0);
  expect((await summary.boundingBox()).y).toBe(header.height);
  await summary.press('Enter');
  await expect(page.locator('.docs-mobile-menu')).toHaveAttribute('open', '');
  await summary.press('Enter');
  await expect(page.locator('.docs-mobile-menu')).not.toHaveAttribute('open', '');
});

for (const theme of ['light', 'dark']) {
  test(`${theme} navigation blends into the page and keeps native scrolling`, async ({ page }, info) => {
    await page.goto('/');
    await page.locator('[data-theme-select]').selectOption(theme);
    const palette = await page.locator('body').evaluate(node => {
      const style = getComputedStyle(node);
      return { page: style.backgroundColor, ink: style.color };
    });
    const sidebar = page.locator('.docs-sidebar');
    await expect(sidebar).toHaveCSS('background-color', 'rgba(0, 0, 0, 0)');
    await expect(sidebar.locator('h2').first()).toHaveCSS('color', palette.ink);
    await expect(sidebar.locator('input[type=search]')).toHaveCSS('color', palette.ink);

    const checkScrollbar = async scroller => {
      const style = await scroller.evaluate(node => {
        const computed = getComputedStyle(node);
        const webkit = CSS.supports('selector(::-webkit-scrollbar)');
        return {
          overflow: node.scrollHeight > node.clientHeight,
          horizontalOverflow: node.scrollWidth > node.clientWidth,
          gutter: computed.scrollbarGutter,
          width: webkit ? getComputedStyle(node, '::-webkit-scrollbar').width : computed.scrollbarWidth,
          track: webkit ? getComputedStyle(node, '::-webkit-scrollbar-track').backgroundColor : computed.scrollbarColor.split(') ').pop(),
          radius: webkit ? getComputedStyle(node, '::-webkit-scrollbar-thumb').borderRadius : null,
          webkit,
        };
      });
      expect(style.overflow).toBe(true);
      expect(style.horizontalOverflow).toBe(false);
      expect(style.gutter).toBe('stable');
      expect(style.width).toBe(style.webkit ? '10px' : 'thin');
      expect(style.track).toBe('rgba(0, 0, 0, 0)');
      if (style.webkit) expect(style.radius).toBe('999px');
    };
    await checkScrollbar(sidebar);
    await page.screenshot({ path: info.outputPath(`navigation-desktop-${theme}.png`) });
    await sidebar.hover();
    await page.mouse.wheel(0, 500);
    await expect.poll(() => sidebar.evaluate(node => node.scrollTop)).toBeGreaterThan(0);
    expect(await page.evaluate(() => scrollY)).toBe(0);
    const lastDesktopLink = sidebar.locator('nav a').last();
    await page.keyboard.press('Tab');
    await lastDesktopLink.focus();
    await expect(lastDesktopLink).toBeInViewport();
    await expect(lastDesktopLink).toBeFocused();
    await expect(lastDesktopLink).toHaveCSS('outline-style', 'solid');

    await page.setViewportSize({ width: 390, height: 844 });
    await page.goto('/');
    const menu = page.locator('.docs-mobile-menu');
    const summary = menu.locator('summary');
    await summary.press('Enter');
    await expect(menu).toHaveAttribute('open', '');
    await expect(menu).toHaveCSS('background-color', palette.page);
    await expect(menu.locator('h2').first()).toHaveCSS('color', palette.ink);
    await checkScrollbar(menu.locator('nav'));
    await page.screenshot({ path: info.outputPath(`navigation-mobile-${theme}.png`) });
    const lastMobileLink = menu.locator('nav a').last();
    await lastMobileLink.focus();
    await expect(lastMobileLink).toBeFocused();
    await expect(lastMobileLink).toBeInViewport();
    await expect(lastMobileLink).toHaveCSS('outline-style', 'solid');
    await expect.poll(() => menu.locator('nav').evaluate(node => node.scrollTop)).toBeGreaterThan(0);
    // Repeated keyboard toggles leave navigation available, with no page overflow.
    await summary.press('Enter');
    await expect(menu).not.toHaveAttribute('open', '');
    await summary.press('Enter');
    await expect(menu).toHaveAttribute('open', '');
    expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(true);
  });
}

test('forced colors retains native navigation scrollbars and keyboard focus', async ({ page }) => {
  await page.emulateMedia({ forcedColors: 'active' });
  await page.goto('/');
  for (const width of [1440, 390]) {
    await page.setViewportSize({ width, height: 844 });
    if (width === 390) await page.locator('.docs-mobile-menu summary').press('Enter');
    const scroller = page.locator(width === 390 ? '.docs-mobile-menu nav' : '.docs-sidebar');
    await expect(scroller).toHaveCSS('scrollbar-color', 'auto');
    await expect(scroller).toHaveCSS('scrollbar-width', 'auto');
    await expect(scroller).toHaveCSS('scrollbar-gutter', 'auto');
    const lastLink = scroller.locator('a').last();
    await page.keyboard.press('Tab');
    await lastLink.focus();
    await expect(lastLink).toBeInViewport();
    await expect(lastLink).toHaveCSS('outline-style', 'solid');
  }
});

for (const { code, directory } of languages) {
  test(`${code} translated controls, section search and all mobile page widths`, async ({ page }, info) => {
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    await page.goto(`/${directory}index.html`);
    const versions = page.locator('main > ul').first().locator(':scope > li');
    await expect(versions).toHaveCount(2);
    await expect(versions.nth(0)).toContainText('ROS 2 Lyrical');
    await expect(versions.nth(1)).toContainText('Gazebo Jetty');
    for (const item of await versions.all()) {
      await expect(item).toContainText('LTS');
      await expect(item).toContainText('2031');
    }
    await page.goto('/pressure.html');
    await page.locator('[data-language-select]').selectOption(`${directory}pressure.html`);
    await expect(page).toHaveURL(new RegExp(`/${directory}pressure.html$`));
    await expect(page.locator('html')).toHaveAttribute('lang', code);
    await expect(page.locator('[data-language-select] option')).toHaveCount(languages.length);
    await expect(page.locator('main h1')).toHaveText(messages[code].pages.pressure);
    await expect(page.locator('.docs-code-copy').first()).toHaveText(messages[code].ui.copy);

    const search = page.locator('.docs-sidebar input[type=search]');
    const results = page.locator('.docs-sidebar .docs-search-results');
    // This SDF parameter appears in the page body, outside navigation labels.
    await search.fill('noiseFreq');
    const result = results.locator(`a[lang="${code}"][href^="currents.html#"]`).first();
    await expect(result).toBeVisible();
    await expect(results.locator(`a:not([lang="${code}"])`)).toHaveCount(0);
    await result.click();
    await expect(page).toHaveURL(new RegExp(`/${directory}currents.html#`));
    await expect(page.locator('html')).toHaveAttribute('lang', code);
    // Inline prose names terminals A/B; replacing it with the page title breaks the sentence.
    await expect(page.locator('main a[href="terminals.html"]').first()).toContainText(/A.*B/);
    await page.evaluate(() => scrollTo(0, 0));
    await page.screenshot({ path: info.outputPath(`${code}-desktop.png`) });

    await page.setViewportSize({ width: 390, height: 844 });
    await page.goto(`/${directory}requirements.html`);
    await page.screenshot({ path: info.outputPath(`${code}-mobile-installation.png`), fullPage: true });
    for (const entry of pages) {
      await page.goto(`/${directory}${entry.slug}.html`);
      expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth + 1), `${directory}${entry.slug} overflows`).toBe(true);
      await expect(page.locator('main h1')).toBeVisible();
      await expect(page.locator('.docs-brand img')).toBeVisible();
      const mismatchedCode = await page.locator('main pre code').evaluateAll(nodes => nodes.filter(node =>
        getComputedStyle(node).fontSize !== getComputedStyle(node.closest('main')).fontSize).length);
      expect(mismatchedCode, `${directory}${entry.slug} code size differs from body text`).toBe(0);
    }
    expect(errors).toEqual([]);
  });
}

test('mobile device code and highlighted tokens use the body text size', async ({ browser }, info) => {
  for (const name of ['iPhone 13', 'Pixel 7']) {
    const context = await browser.newContext({ ...devices[name] });
    try {
      const page = await context.newPage();
      await page.goto('http://127.0.0.1:4184/ko/terminals.html');
      const code = page.locator('pre code').filter({ hasText: 'export POSIM_WS=' }).first();
      await code.scrollIntoViewIfNeeded();
      const size = await page.locator('main p').first().evaluate(node => getComputedStyle(node).fontSize);
      const sizes = await code.evaluate(node => [node, ...node.querySelectorAll('span')].map(element => getComputedStyle(element).fontSize));
      expect([...new Set(sizes)]).toEqual([size]);
      expect(await page.evaluate(() => getComputedStyle(document.documentElement).webkitTextSizeAdjust)).toBe('100%');
      await page.screenshot({ path: info.outputPath(`${name.replaceAll(' ', '-')}-code.png`) });
    } finally {
      await context.close();
    }
  }
});

test('search from a translated page links to English and a third language', async ({ page }) => {
  for (const [query, language, destination] of [
    ['Sea pressure', 'en', '/pressure.html'],
    ['관성', 'ko', '/ko/'],
  ]) {
    await page.goto('/ja/index.html');
    await page.locator('.docs-sidebar [data-search-other]').check();
    await page.locator('.docs-sidebar input[type=search]').fill(query);
    const result = page.locator(`.docs-sidebar .docs-search-results a[lang="${language}"]`).first();
    await expect(result).toBeVisible();
    await result.click();
    await expect(page.locator('html')).toHaveAttribute('lang', language);
    expect(new URL(page.url()).pathname.startsWith(destination)).toBe(true);
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
  const search = page.locator('.docs-sidebar input[type=search]');
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

test('section search finds code and Korean text with language opt-in and keyboard navigation', async ({ page }, info) => {
  await page.goto('/');
  const search = page.locator('.docs-sidebar input[type=search]');
  const results = page.locator('.docs-sidebar .docs-search-results');
  for (const query of ['ixx', 'ros2 topic echo', 'WSL2']) {
    await search.fill(query);
    await expect(results.locator('a').first()).toBeVisible();
    await expect(results.locator('a[lang=ko]')).toHaveCount(0);
  }
  await search.fill('관성');
  await expect(results).toHaveText('No matching guides.');
  await page.locator('.docs-sidebar [data-search-other]').check();
  await expect(results.locator('a[lang=ko]').first()).toBeVisible();
  await expect(results.locator('a[lang=ko] mark').first()).toHaveText('관성');
  await results.locator('a[lang=ko]').first().click();
  await expect(page.locator('html')).toHaveAttribute('lang', 'ko');
  expect(new URL(page.url()).hash).not.toBe('');

  await page.goto('/');
  await page.keyboard.press('Control+k');
  await expect(search).toBeFocused();
  await search.fill('max_step_size');
  await expect(results.locator('a[href^="custom-worlds.html#"]').first()).toBeVisible();
  await expect(results.locator('a').first().locator('mark').first()).toBeInViewport();
  await page.screenshot({ path: info.outputPath('section-search.png') });
  const destination = await results.locator('a').first().getAttribute('href');
  await search.press('ArrowDown');
  await expect(results.locator('a').first()).toBeFocused();
  await page.keyboard.press('Enter');
  await expect.poll(() => {
    const url = new URL(page.url());
    return url.pathname + decodeURIComponent(url.hash);
  }).toBe('/' + destination);
  await expect(page.locator('[id="' + destination.split('#')[1] + '"]')).toBeInViewport();

  await page.setViewportSize({ width: 390, height: 844 });
  await page.keyboard.press('Control+k');
  await expect(page.locator('.docs-mobile-menu')).toHaveAttribute('open', '');
  await expect(page.locator('.docs-mobile-menu input[type=search]')).toBeFocused();
  await page.locator('.docs-mobile-menu input[type=search]').fill('max_step_size');
  await expect(page.locator('.docs-mobile-menu .docs-search-results a').first()).toBeVisible();
  await page.screenshot({ path: info.outputPath('mobile-section-search.png') });
});

test('moved tutorial bookmarks open the corresponding English and Korean sections', async ({ page }) => {
  for (const [oldPath, newPath] of [
    ['/ros.html#subscribe-from-python', '/python-control.html#subscribe-from-python'],
    ['/ros.html#visualization-and-recording', '/visualization.html#visualization-and-recording'],
    ['/quickstart.html#2-spawn-rexrov', '/quickstart.html#1-start-the-robot-and-world'],
    ['/ko/ros.html#python에서-구독', '/ko/python-control.html#python에서-구독'],
    ['/ko/quickstart.html#3-카메라-영상-받기', '/ko/visualization.html#카메라-영상-받기'],
  ]) {
    await page.goto(oldPath);
    await expect.poll(() => {
      const url = new URL(page.url());
      return url.pathname + decodeURIComponent(url.hash);
    }).toBe(newPath);
    await expect(page.locator('[id="' + newPath.split('#')[1] + '"]')).toBeInViewport();
  }
});

test('invalid and unmapped bookmarks leave documentation controls usable', async ({ page }) => {
  const errors = [];
  page.on('pageerror', error => errors.push(error.message));
  for (const directory of ['', 'ko/']) {
    for (const fragment of ['%', '%E0%A4%A', 'constructor', '__proto__']) {
      const target = `/${directory}quickstart.html#${fragment}`;
      await page.goto(target);
      expect(errors).toEqual([]);
      await expect(page.locator('.docs-code-copy').first()).toBeVisible();
      expect(new URL(page.url()).pathname + new URL(page.url()).hash).toBe(target);
      await page.locator('[data-language-select]').selectOption(
        directory ? '../quickstart.html' : 'ko/quickstart.html');
      await expect(page.locator('html')).toHaveAttribute('lang', directory ? 'en' : 'ko');
    }
  }
  expect(errors).toEqual([]);
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
