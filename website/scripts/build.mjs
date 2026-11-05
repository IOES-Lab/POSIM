import { cp, mkdir, readFile, readdir, rm, writeFile } from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { Marked } from 'marked';
import hljs from 'highlight.js/lib/core';
import bash from 'highlight.js/lib/languages/bash';
import xml from 'highlight.js/lib/languages/xml';
import python from 'highlight.js/lib/languages/python';
import powershell from 'highlight.js/lib/languages/powershell';
import { pages, sections } from './pages.mjs';
import { anchorRedirects } from './redirects.mjs';
import { languages, messages } from './locales.mjs';
import { readContent } from './content.mjs';

hljs.registerLanguage('bash', highlighter => {
  const grammar = bash(highlighter);
  // Include the command-line tools used throughout the POSIM guides.
  grammar.keywords.built_in.push('ros2', 'gz', 'colcon', 'rosdep', 'vcs', 'docker',
    'rocker', 'pnpm', 'cmake', 'ctest', 'git', 'apt', 'apt-get', 'curl', 'wget', 'python3');
  return grammar;
});
hljs.registerLanguage('xml', xml);
hljs.registerAliases('sdf', { languageName: 'xml' });
hljs.registerAliases(['shell', 'console'], { languageName: 'bash' });
hljs.registerLanguage('python', python);
hljs.registerLanguage('powershell', powershell);


const root = fileURLToPath(new URL('../', import.meta.url));
const output = path.join(root, 'dist');
const repository = path.resolve(root, '..');
const sources = JSON.parse(await readFile(path.join(root, 'sources.json'), 'utf8'));
const esc = value => String(value).replace(/[&<>"']/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
const entities = { amp: '&', lt: '<', gt: '>', quot: '"', apos: "'", nbsp: ' ' };
const plain = html => html.replace(/<\/?span\b[^>]*>/g, '').replace(/<[^>]*>/g, ' ')
  .replace(/&(#x[0-9a-f]+|#\d+|amp|lt|gt|quot|apos|nbsp);/gi, (match, value) => {
    if (!value.startsWith('#')) return entities[value.toLowerCase()] || match;
    const number = value[1].toLowerCase() === 'x' ? parseInt(value.slice(2), 16) : Number(value.slice(1));
    return number <= 0x10ffff ? String.fromCodePoint(number) : match;
  }).replace(/\s+/g, ' ').trim();
// A committed catalog makes website/ deployable without uploading model assets.
// In a full checkout, generate from source; CI checks snapshot consistency.
const catalog = JSON.parse(await readFile(path.join(root, 'catalog.json'), 'utf8'));
const worldFiles = await readdir(path.join(repository, 'models/posim_worlds/worlds'))
  .then(files => files.filter(f => f.endsWith('.world')).sort()).catch(error => {
    if (error.code !== 'ENOENT') throw error;
    return catalog.worlds;
  });
const worldRows = worldFiles.map(f => `| [\`${f.slice(0, -6)}\`](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/${f}) |`).join('\n');
const objectFiles = await readdir(path.join(repository, 'models/posim_object_models/description'), { withFileTypes: true })
  .then(files => files.filter(f => f.isDirectory()).map(f => f.name).sort()).catch(error => {
    if (error.code !== 'ENOENT') throw error;
    return catalog.objects;
  });
const objectRows = objectFiles.map(f => `- [\`${f}\`](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_object_models/description/${f})`).join('\n');

await rm(output, { recursive: true, force: true });
for (const { directory } of languages) await mkdir(path.join(output, directory), { recursive: true });
await cp(path.join(root, 'public'), path.join(output, 'assets'), { recursive: true });

for (const { code: lang, directory } of languages) {
  const nested = lang !== 'en';
  const ui = messages[lang].ui;
  const asset = nested ? '../assets/' : 'assets/';
  const index = [];
  const rendered = [];
  for (const page of pages) {
    let text = await readContent(lang, page.slug);
    text = text.replace('{{WORLD_CATALOG}}', worldRows).replace('{{OBJECT_CATALOG}}', objectRows)
      .replaceAll('{{ASSET_PREFIX}}', asset);
    const toc = [], ids = new Map();
    const marked = new Marked();
    marked.use({ renderer: { code({ text, lang }) {
      const language = (lang || '').trim().split(/\s+/)[0].toLowerCase();
      if (!hljs.getLanguage(language)) return false;
      const code = text ? text.replace(/\n$/, '') + '\n' : '';
      const highlighted = hljs.highlight(code, { language, ignoreIllegals: true }).value;
      return `<pre><code class="hljs language-${esc(language)}">${highlighted}</code></pre>\n`;
    }, heading({ tokens, depth }) {
      const raw = this.parser.parseInline(tokens);
      const explicitId = raw.match(/\s*\{#([a-z0-9-]+)\}$/)?.[1];
      const authored = raw.replace(/\s*\{#[a-z0-9-]+\}$/, '');
      const content = depth === 1 ? esc(page[lang]) : authored;
      const base = explicitId || plain(authored).normalize('NFKC').toLowerCase().replace(/[^\p{L}\p{N}]+/gu, '-').replace(/^-|-$/g, '') || 'section';
      const count = ids.get(base) || 0; ids.set(base, count + 1);
      const id = count ? `${base}-${count}` : base;
      if (depth === 2 || depth === 3) toc.push({ id, title: plain(content), depth });
      return `<h${depth} id="${esc(id)}">${content}</h${depth}>\n`;
    } } });
    let body = marked.parse(text);
    body = body.replace(/href="([a-z-]+)\.md(#[^"]*)?"/g, 'href="$1.html$2"');
    rendered.push({ page, body, toc });
  }
  const destinations = new Map(rendered.map(({ page, toc }) => [page.slug + '.html', {
    title: page[lang], headings: new Map(toc.map(item => [item.id, item.title])),
  }]));
  for (const renderedPage of rendered) {
    const { page, toc } = renderedPage;
    let body = renderedPage.body.replace(/(<a\b[^>]*href=")([^"#]*\.html)(#[^"]*)?("[^>]*>)([\s\S]*?)(<\/a>)/g,
      (match, before, file, hash, after, label, close) => {
        if (!destinations.has(file)) return match;
        let destination = file + (hash || '');
        if (hash) destination = anchorRedirects[file.slice(0, -5)]?.[lang]?.[decodeURIComponent(hash.slice(1))] || destination;
        const [target, fragment] = destination.split('#');
        const record = destinations.get(target);
        const title = fragment ? record?.headings.get(decodeURIComponent(fragment)) : record?.title;
        if (!title) return match;
        // Link text may be inflected or part of a sentence in the authored language.
        return before + destination + after + label + close;
      });
    const redirects = anchorRedirects[page.slug]?.[lang];
    if (redirects) {
      const present = new Set([...body.matchAll(/\bid="([^"]+)"/g)].map(match => match[1]));
      body += Object.keys(redirects).filter(id => !present.has(id)).map(id => `<span id="${esc(id)}" hidden></span>`).join('');
      body += `<script type="application/json" id="docs-anchor-redirects">${JSON.stringify(redirects).replaceAll('<', '\\u003c')}</script>`;
    }
    const title = page[lang];
    const firstParagraph = body.match(/<p>([\s\S]*?)<\/p>/)?.[1] || title;
    const description = plain(firstParagraph).slice(0, 220);
    const nav = sections.map(section => `<section><h2>${esc(section[lang])}</h2>${section.pages.map(([slug]) => `<a href="${slug}.html"${slug === page.slug ? ' aria-current="page"' : ''}>${esc(messages[lang].pages[slug])}</a>`).join('')}</section>`).join('');
    const sourceLinks = page.sources.map(key => `<a href="${esc(sources.notion[key].url)}">${esc(sources.notion[key].title.replace(/^POSIM\s*[—–]?\s*/, ''))}</a>`).join(' · ');
    const alternateLinks = languages.map(locale => `<link rel="alternate" hreflang="${locale.code}" href="${nested ? '../' : ''}${locale.directory}${page.slug}.html">`).join('');
    const position = pages.indexOf(page);
    const previous = pages[position - 1], next = pages[position + 1];
    const navigation = `<nav class="docs-page-nav" aria-label="${ui.previousNext}">${previous ? `<a href="${previous.slug}.html"><small>${ui.previous}</small>${esc(previous[lang])}</a>` : '<span></span>'}${next ? `<a href="${next.slug}.html"><small>${ui.next}</small>${esc(next[lang])}</a>` : ''}</nav>`;
    const languageOptions = languages.map(locale => `<option value="${nested ? '../' : ''}${locale.directory}${page.slug}.html" lang="${locale.code}"${locale.code === lang ? ' selected' : ''}>${locale.label}</option>`).join('');
    const controls = `<span class="docs-control-wrap"><select class="docs-control" data-language-select aria-label="${ui.documentationLanguage}">${languageOptions}</select></span><span class="docs-control-wrap"><select class="docs-control" data-theme-select aria-label="${ui.colorTheme}"><option value="system">${ui.system}</option><option value="light">${ui.light}</option><option value="dark">${ui.dark}</option></select></span>`;
    const html = `<!doctype html><html lang="${lang}"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><meta name="description" content="${esc(description)}"><meta name="document-audience" content="public-posim-library"><title>${esc(title)} · POSIM</title><link rel="icon" type="image/svg+xml" href="${asset}favicon.svg">${alternateLinks}<script type="application/json" id="docs-localization">${JSON.stringify({ ui, languages: languages.map(locale => locale.code) }).replaceAll('<', '\\u003c')}</script><script src="${asset}theme.js"></script><link rel="stylesheet" href="${asset}docs.css"><script defer src="${asset}docs.js"></script></head><body data-search-index="${asset}search.${lang}.json"><a class="skip-link" href="#main">${ui.skip}</a>
<header class="docs-header"><a class="docs-brand" href="index.html"><img src="${asset}app-icon.svg" width="28" height="28" alt="" aria-hidden="true">POSIM</a><span class="docs-product">${ui.product}</span><nav aria-label="${ui.utility}">${controls}<a href="https://github.com/IOES-Lab/POSIM">GitHub</a></nav></header>
<details class="docs-mobile-menu"><summary>${ui.menu}</summary><label class="docs-search">${ui.search}<input type="search" placeholder="${ui.placeholder}" autocomplete="off"></label><label class="docs-search-other"><input type="checkbox" data-search-other>${ui.otherLanguages}</label><div class="docs-search-results" aria-live="polite" hidden></div><nav aria-label="${ui.mobileNavigation}">${nav}</nav></details>
<div class="docs-layout"><aside class="docs-sidebar"><label class="docs-search">${ui.search}<input type="search" placeholder="${ui.placeholder}" autocomplete="off"></label><label class="docs-search-other"><input type="checkbox" data-search-other>${ui.otherLanguages}</label><div class="docs-search-results" aria-live="polite" hidden></div><nav aria-label="${ui.documentation}">${nav}</nav><a class="docs-lab" href="https://lab.wschoi.com">IOES-Lab · KMOU</a></aside>
<main id="main" class="docs-content"><div class="docs-breadcrumb">POSIM <span>/</span> ${esc(page.sectionTitles[lang])}</div>${body}${navigation}<footer class="docs-footer"><span>POSIM · ROS 2 Lyrical · Gazebo Jetty</span><a href="https://github.com/IOES-Lab/POSIM/edit/main/website/content/${lang}/${page.slug}.md">${ui.edit}</a>${sourceLinks ? `<div class="docs-source-note">${ui.references}: ${sourceLinks}</div>` : ''}</footer></main>
<aside class="docs-toc" aria-label="${ui.onThisPage}"><strong>${ui.onThisPage}</strong>${toc.map(item => `<a class="level-${item.depth}" href="#${esc(item.id)}">${esc(item.title)}</a>`).join('')}</aside></div></body></html>`;
    await writeFile(path.join(output, directory, page.slug + '.html'), html);
    const searchable = body.replace(/<script\b[^>]*>[\s\S]*?<\/script>/g, '');
    const headings = [...searchable.matchAll(/<h([23]) id="([^"]+)">([\s\S]*?)<\/h\1>/g)];
    const slices = [{ heading: title, anchor: '', start: 0, end: headings[0]?.index ?? searchable.length },
      ...headings.map((heading, i) => ({ heading: plain(heading[3]), anchor: heading[2],
        start: heading.index + heading[0].length, end: headings[i + 1]?.index ?? searchable.length }))];
    for (const slice of slices) index.push({ title, heading: slice.heading, language: lang,
      url: page.slug + '.html' + (slice.anchor ? '#' + slice.anchor : ''),
      section: page.sectionTitles[lang], description, text: plain(searchable.slice(slice.start, slice.end)) });
  }
  await writeFile(path.join(output, 'assets', `search.${lang}.json`), JSON.stringify(index));
  const moved = 'features.html' + (lang === 'ko' ? '#소스-디렉토리' : lang === 'en' ? '#source-directories' : '');
  await writeFile(path.join(output, directory, 'libraries.html'), `<!doctype html><html lang="${lang}"><meta charset="utf-8"><meta http-equiv="refresh" content="0;url=${moved}"><title>POSIM</title><a href="${moved}">${ui.sourceDirectories}</a></html>`);
}
const urls = languages.flatMap(({ directory }) => pages.map(page => `/${directory}${page.slug === 'index' ? '' : page.slug + '.html'}`));
const siteUrl = process.env.SITE_URL;
if (siteUrl) {
  const site = new URL(siteUrl);
  if (!['https:', 'http:'].includes(site.protocol)) throw new Error('SITE_URL must be an HTTP(S) address');
  const origin = site.origin + site.pathname.replace(/\/$/, '');
  await writeFile(path.join(output, 'sitemap.xml'), '<?xml version="1.0" encoding="UTF-8"?><urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">' + urls.map(url => `<url><loc>${esc(origin + url)}</loc></url>`).join('') + '</urlset>');
  await writeFile(path.join(output, 'robots.txt'), `User-agent: *\nAllow: /\nSitemap: ${origin}/sitemap.xml\n`);
}
console.log(`Built ${pages.length * languages.length} pages in ${languages.length} languages from local Markdown. No Notion connection is needed at runtime.`);
