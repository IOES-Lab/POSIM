/* Local search and documentation controls; no simulator connection required. */
const ko = document.documentElement.lang === 'ko';
let searchIndex;
const indexPromise = () => searchIndex ||= fetch(document.body.dataset.searchIndex)
  .then(response => { if (!response.ok) throw new Error('Search unavailable'); return response.json(); })
  .catch(error => { searchIndex = undefined; throw error; });

document.querySelectorAll('.docs-search input').forEach(input => {
  const container = input.closest('.docs-sidebar, .docs-mobile-menu');
  const results = container.querySelector('.docs-search-results');
  let revision = 0;
  input.addEventListener('input', async () => {
    const current = ++revision;
    const query = input.value.trim().toLocaleLowerCase();
    results.replaceChildren(); results.hidden = !query;
    if (!query) return;
    try {
      const index = await indexPromise();
      if (current !== revision) return;
      const words = query.split(/\s+/);
      const matches = index.map(page => ({ page, score: words.reduce((score, word) =>
        score + (page.title.toLocaleLowerCase().includes(word) ? 10 : 0)
        + Math.min(8, page.text.toLocaleLowerCase().split(word).length - 1), 0) }))
        .filter(item => words.every(word => `${item.page.title} ${item.page.text}`.toLocaleLowerCase().includes(word)))
        .sort((a, b) => b.score - a.score).slice(0, 8);
      if (!matches.length) results.textContent = ko ? '검색 결과가 없습니다.' : 'No matching guides.';
      for (const { page } of matches) {
        const link = document.createElement('a'); link.href = page.url;
        const title = document.createElement('strong'); title.textContent = page.title;
        const description = document.createElement('span'); description.textContent = page.description;
        link.append(title, description); results.append(link);
      }
    } catch { if (current === revision) results.textContent = ko ? '문서 메뉴에서 가이드를 선택해 주세요.' : 'Select a guide from the documentation menu.'; }
  });
  input.addEventListener('keydown', event => {
    if (event.key === 'Escape') { input.value = ''; input.dispatchEvent(new Event('input')); }
    if (event.key === 'ArrowDown' || event.key === 'Enter') {
      const first = results.querySelector('a');
      if (first) { event.preventDefault(); if (event.key === 'Enter') first.click(); else first.focus(); }
    }
  });
});
document.querySelectorAll('.docs-mobile-menu nav a, .docs-mobile-menu .docs-search-results').forEach(element =>
  element.addEventListener('click', event => { if (event.target.closest('a')) element.closest('details').removeAttribute('open'); }));

document.querySelectorAll('.docs-content pre > code').forEach(code => {
  const pre = code.parentElement;
  const block = document.createElement('div'); block.className = 'docs-code-block'; pre.before(block); block.append(pre);
  const button = document.createElement('button'); button.type = 'button'; button.className = 'docs-code-copy';
  const label = ko ? '복사' : 'Copy'; button.textContent = label;
  button.setAttribute('aria-label', ko ? '코드 복사' : 'Copy code');
  const status = document.createElement('span'); status.className = 'docs-copy-status'; status.setAttribute('role', 'status');
  block.append(button, status); let reset;
  button.addEventListener('click', async () => {
    clearTimeout(reset);
    try {
      await navigator.clipboard.writeText(code.textContent);
      button.textContent = ko ? '복사됨' : 'Copied'; status.textContent = button.textContent;
    } catch {
      const selection = window.getSelection(), range = document.createRange(); range.selectNodeContents(code);
      selection.removeAllRanges(); selection.addRange(range);
      button.textContent = ko ? '텍스트 선택됨' : 'Text selected'; status.textContent = ko ? '복사 단축키를 눌러 주세요.' : 'Press your copy shortcut.';
    }
    reset = setTimeout(() => { button.textContent = label; status.textContent = ''; }, 2500);
  });
});
