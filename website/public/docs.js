/* Local search and documentation controls; no simulator connection required. */
const localization = JSON.parse(document.querySelector('#docs-localization').textContent);
const ui = localization.ui;
const searchIndexes = new Map();
const language = document.documentElement.lang;
const normalize = value => value.normalize('NFKC').toLocaleLowerCase();
const indexPromise = lang => {
  if (!searchIndexes.has(lang)) searchIndexes.set(lang,
    fetch(document.body.dataset.searchIndex.replace(/search\.[a-z]+\.json$/, `search.${lang}.json`))
      .then(response => { if (!response.ok) throw new Error('Search unavailable'); return response.json(); })
      .catch(error => { searchIndexes.delete(lang); throw error; }));
  return searchIndexes.get(lang);
};
function highlight(target, value, words) {
  const text = value.normalize('NFKC');
  const pattern = new RegExp(words.map(word => word.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')).join('|'), 'giu');
  let offset = 0;
  for (const match of text.matchAll(pattern)) {
    target.append(document.createTextNode(text.slice(offset, match.index)));
    const mark = document.createElement('mark'); mark.textContent = match[0]; target.append(mark);
    offset = match.index + match[0].length;
  }
  target.append(document.createTextNode(text.slice(offset)));
}

document.querySelectorAll('.docs-search input').forEach(input => {
  const container = input.closest('.docs-sidebar, .docs-mobile-menu');
  const results = container.querySelector('.docs-search-results');
  const other = container.querySelector('[data-search-other]');
  let revision = 0;
  const search = async () => {
    const current = ++revision;
    const query = normalize(input.value).trim();
    results.replaceChildren(); results.hidden = !query;
    if (!query) return;
    try {
      const index = (await Promise.all((other.checked ? localization.languages : [language]).map(indexPromise))).flat();
      if (current !== revision) return;
      const words = [...new Set(query.split(/\s+/))];
      const matches = index.map(page => ({ page, score: words.reduce((score, word) =>
        score + (normalize(page.title).includes(word) ? 10 : 0)
        + (normalize(page.heading).includes(word) ? 8 : 0)
        + Math.min(2, normalize(page.text).split(word).length - 1), 0)
        + (normalize(page.title) === query && !page.url.includes('#') ? 30 : 0) }))
        .filter(item => words.every(word => normalize(`${item.page.title} ${item.page.heading} ${item.page.text}`).includes(word)))
        .sort((a, b) => b.score - a.score || (b.page.language === language) - (a.page.language === language)).slice(0, 10);
      if (!matches.length) results.textContent = ui.noMatches;
      for (const { page } of matches) {
        const link = document.createElement('a'); link.lang = page.language;
        link.href = (page.language === language ? '' : (language === 'en' ? '' : '../') + (page.language === 'en' ? '' : page.language + '/')) + page.url;
        const title = document.createElement('strong'); highlight(title, page.title, words); link.append(title);
        if (page.heading !== page.title) {
          const heading = document.createElement('span'); heading.className = 'docs-search-heading';
          highlight(heading, page.heading, words); link.append(heading);
        }
        const positions = words.map(word => normalize(page.text).indexOf(word)).filter(position => position >= 0);
        const start = Math.max(0, (positions.length ? Math.min(...positions) : 0) - 20);
        const snippet = page.text.slice(start, start + 170);
        const description = document.createElement('span');
        highlight(description, (start ? '…' : '') + snippet + (start + snippet.length < page.text.length ? '…' : ''), words);
        link.append(description);
        if (page.language !== language) {
          const badge = document.createElement('small'); badge.textContent = page.language.toUpperCase(); link.append(badge);
        }
        results.append(link);
      }
    } catch { if (current === revision) results.textContent = ui.searchUnavailable; }
  };
  input.addEventListener('input', search);
  other.addEventListener('change', search);
  input.addEventListener('keydown', event => {
    if (event.key === 'Escape') { input.value = ''; input.dispatchEvent(new Event('input')); }
    if (event.key === 'ArrowDown' || event.key === 'Enter') {
      const first = results.querySelector('a');
      if (first) { event.preventDefault(); if (event.key === 'Enter') first.click(); else first.focus(); }
    }
  });
  results.addEventListener('keydown', event => {
    const links = [...results.querySelectorAll('a')], active = links.indexOf(document.activeElement);
    if (event.key === 'ArrowDown' && links.length) { event.preventDefault(); links[(active + 1) % links.length].focus(); }
    if (event.key === 'ArrowUp') { event.preventDefault(); if (active <= 0) input.focus(); else links[active - 1].focus(); }
    if (event.key === 'Escape') { input.value = ''; search(); input.focus(); }
  });
});
document.addEventListener('keydown', event => {
  if (!(event.key.toLowerCase() === 'k' && (event.ctrlKey || event.metaKey))) return;
  event.preventDefault();
  const sidebar = document.querySelector('.docs-sidebar');
  const menu = document.querySelector('.docs-mobile-menu');
  const mobile = getComputedStyle(sidebar).display === 'none';
  if (mobile) menu.open = true;
  (mobile ? menu : sidebar).querySelector('.docs-search input').focus();
});
const redirectData = document.querySelector('#docs-anchor-redirects');
if (redirectData) {
  const redirects = JSON.parse(redirectData.textContent);
  const follow = () => {
    let fragment;
    try { fragment = decodeURIComponent(location.hash.slice(1)); } catch { return; }
    if (Object.hasOwn(redirects, fragment)) location.replace(redirects[fragment]);
  };
  follow(); window.addEventListener('hashchange', follow);
}
document.querySelectorAll('.docs-mobile-menu nav a, .docs-mobile-menu .docs-search-results').forEach(element =>
  element.addEventListener('click', event => { if (event.target.closest('a')) element.closest('details').removeAttribute('open'); }));

document.querySelectorAll('.docs-content pre > code').forEach(code => {
  const pre = code.parentElement;
  const block = document.createElement('div'); block.className = 'docs-code-block'; pre.before(block); block.append(pre);
  const button = document.createElement('button'); button.type = 'button'; button.className = 'docs-code-copy';
  const label = ui.copy; button.textContent = label;
  button.setAttribute('aria-label', ui.copyCode);
  const status = document.createElement('span'); status.className = 'docs-copy-status'; status.setAttribute('role', 'status');
  block.append(button, status); let reset;
  button.addEventListener('click', async () => {
    clearTimeout(reset);
    try {
      await navigator.clipboard.writeText(code.textContent);
      button.textContent = ui.copied; status.textContent = button.textContent;
    } catch {
      const selection = window.getSelection(), range = document.createRange(); range.selectNodeContents(code);
      selection.removeAllRanges(); selection.addRange(range);
      button.textContent = ui.textSelected; status.textContent = ui.copyShortcut;
    }
    reset = setTimeout(() => { button.textContent = label; status.textContent = ''; }, 2500);
  });
});

// Keep the native playback controls and add feedback during network/decode waits.
document.querySelectorAll('.docs-content video').forEach(video => {
  const frame = document.createElement('div'); frame.className = 'docs-video';
  video.before(frame); frame.append(video);
  const overlay = document.createElement('div'); overlay.className = 'docs-video-feedback'; overlay.hidden = true;
  const spinner = document.createElement('span'); spinner.className = 'docs-video-spinner'; spinner.setAttribute('aria-hidden', 'true');
  const status = document.createElement('span'); status.setAttribute('role', 'status');
  const progress = document.createElement('progress'); progress.max = 1;
  progress.setAttribute('aria-label', ui.videoBuffered);
  const retry = document.createElement('button'); retry.type = 'button'; retry.hidden = true;
  retry.textContent = ui.retry;
  const open = document.createElement('a'); open.textContent = ui.openVideo;
  open.href = video.currentSrc || video.querySelector('source')?.src || video.src;
  open.hidden = true;
  overlay.append(spinner, status, progress, retry, open); frame.append(overlay);
  let failed = false;
  const updateProgress = () => {
    if (Number.isFinite(video.duration) && video.duration > 0 && video.buffered.length) {
      progress.value = Math.min(1, video.buffered.end(video.buffered.length - 1) / video.duration);
    } else progress.removeAttribute('value');
  };
  const clear = () => {
    if (failed) return;
    overlay.hidden = true; status.textContent = ''; video.removeAttribute('aria-busy');
  };
  const buffering = () => {
    if (failed || video.paused || video.ended) return;
    overlay.hidden = false; spinner.hidden = false; progress.hidden = false;
    status.textContent = video.currentTime > 0
      ? (ui.buffering)
      : (ui.loading);
    video.setAttribute('aria-busy', 'true'); updateProgress();
  };
  const failure = () => {
    failed = true; overlay.hidden = false; spinner.hidden = true; progress.hidden = true;
    retry.hidden = false; open.hidden = false; video.removeAttribute('aria-busy');
    status.textContent = ui.videoFailed;
  };
  for (const name of ['play', 'loadstart', 'seeking', 'stalled']) {
    video.addEventListener(name, () => { if (video.readyState < 3) buffering(); });
  }
  video.addEventListener('waiting', buffering);
  for (const name of ['playing', 'pause', 'ended', 'emptied']) video.addEventListener(name, clear);
  video.addEventListener('canplay', clear);
  for (const name of ['progress', 'durationchange']) video.addEventListener(name, updateProgress);
  video.addEventListener('error', failure);
  video.querySelectorAll('source').forEach(source => source.addEventListener('error', failure));
  retry.addEventListener('click', async () => {
    failed = false; retry.hidden = true; open.hidden = true;
    video.load();
    try { await video.play(); }
    catch (error) {
      // Pause, navigation and autoplay policy are not download failures.
      if (error.name === 'AbortError' || error.name === 'NotAllowedError') clear();
      else failure();
    }
  });
  if (video.error) failure();
});

// Each option points to the same guide in an available language.
document.querySelectorAll('[data-language-select]').forEach(select => {
  select.addEventListener('change', () => { window.location.assign(select.value); });
});
