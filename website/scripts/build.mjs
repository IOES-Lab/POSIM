import { cp, mkdir, readFile, readdir, rm, writeFile } from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { Marked } from 'marked';
import { pages, sections } from './pages.mjs';

const root = fileURLToPath(new URL('../', import.meta.url));
const output = path.join(root, 'dist');
const repository = path.resolve(root, '..');
const sources = JSON.parse(await readFile(path.join(root, 'sources.json'), 'utf8'));
const esc = value => String(value).replace(/[&<>"']/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
const entities = { amp: '&', lt: '<', gt: '>', quot: '"', apos: "'", nbsp: ' ' };
const plain = html => html.replace(/<[^>]*>/g, ' ')
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
await mkdir(path.join(output, 'ko'), { recursive: true });
await cp(path.join(root, 'public'), path.join(output, 'assets'), { recursive: true });

for (const lang of ['en', 'ko']) {
  const ko = lang === 'ko';
  const asset = ko ? '../assets/' : 'assets/';
  const index = [];
  for (const page of pages) {
    let text = await readFile(path.join(root, 'content', lang, page.slug + '.md'), 'utf8');
    text = text.replace('{{WORLD_CATALOG}}', worldRows).replace('{{OBJECT_CATALOG}}', objectRows);
    const toc = [], ids = new Map();
    const marked = new Marked();
    marked.use({ renderer: { heading({ tokens, depth }) {
      const content = this.parser.parseInline(tokens);
      const base = plain(content).normalize('NFKC').toLowerCase().replace(/[^\p{L}\p{N}]+/gu, '-').replace(/^-|-$/g, '') || 'section';
      const count = ids.get(base) || 0; ids.set(base, count + 1);
      const id = count ? `${base}-${count}` : base;
      if (depth === 2 || depth === 3) toc.push({ id, title: plain(content), depth });
      return `<h${depth} id="${esc(id)}">${content}</h${depth}>\n`;
    } } });
    let body = marked.parse(text);
    body = body.replace(/href="([a-z-]+)\.md(#[^"]*)?"/g, 'href="$1.html$2"');
    const title = page[lang];
    const firstParagraph = body.match(/<p>([\s\S]*?)<\/p>/)?.[1] || title;
    const description = plain(firstParagraph).slice(0, 220);
    const nav = sections.map(section => `<section><h2>${esc(section[lang])}</h2>${section.pages.map(([slug, en, kr]) => `<a href="${slug}.html"${slug === page.slug ? ' aria-current="page"' : ''}>${esc(ko ? kr : en)}</a>`).join('')}</section>`).join('');
    const sourceLinks = page.sources.map(key => `<a href="${esc(sources.notion[key].url)}">${esc(sources.notion[key].title.replace(/^POSIM\s*[—–]?\s*/, ''))}</a>`).join(' · ');
    const alternate = ko ? `../${page.slug}.html` : `ko/${page.slug}.html`;
    const position = pages.indexOf(page);
    const previous = pages[position - 1], next = pages[position + 1];
    const navigation = `<nav class="docs-page-nav" aria-label="${ko ? '이전·다음 문서' : 'Previous and next guides'}">${previous ? `<a href="${previous.slug}.html"><small>${ko ? '이전' : 'Previous'}</small>${esc(previous[lang])}</a>` : '<span></span>'}${next ? `<a href="${next.slug}.html"><small>${ko ? '다음' : 'Next'}</small>${esc(next[lang])}</a>` : ''}</nav>`;
    const html = `<!doctype html><html lang="${lang}"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><meta name="description" content="${esc(description)}"><meta name="document-audience" content="public-posim-library"><title>${esc(title)} · POSIM</title><link rel="icon" type="image/svg+xml" href="${asset}favicon.svg"><link rel="alternate" hreflang="${ko ? 'en' : 'ko'}" href="${alternate}"><link rel="stylesheet" href="${asset}docs.css"><script defer src="${asset}docs.js"></script></head><body data-search-index="${asset}search.${lang}.json"><a class="skip-link" href="#main">${ko ? '본문으로 이동' : 'Skip to content'}</a>
<header class="docs-header"><a class="docs-brand" href="index.html">POSIM</a><span class="docs-product">${ko ? 'Platform for Ocean Simulation · 문서' : 'Platform for Ocean Simulation · Documentation'}</span><nav aria-label="${ko ? '바로가기' : 'Utility navigation'}"><a class="docs-language" href="${alternate}" lang="${ko ? 'en' : 'ko'}">${ko ? 'English' : '한국어'}</a><a href="https://github.com/IOES-Lab/POSIM">GitHub</a></nav></header>
<details class="docs-mobile-menu"><summary>${ko ? '문서 메뉴' : 'Documentation menu'}</summary><label class="docs-search">${ko ? '문서 검색' : 'Search documentation'}<input type="search" placeholder="${ko ? '제목·명령·주제 검색' : 'Titles, commands, topics'}" autocomplete="off"></label><div class="docs-search-results" aria-live="polite" hidden></div><nav aria-label="${ko ? '모바일 문서' : 'Mobile documentation'}">${nav}</nav></details>
<div class="docs-layout"><aside class="docs-sidebar"><label class="docs-search">${ko ? '문서 검색' : 'Search documentation'}<input type="search" placeholder="${ko ? '제목·명령·주제 검색' : 'Titles, commands, topics'}" autocomplete="off"></label><div class="docs-search-results" aria-live="polite" hidden></div><nav aria-label="${ko ? '문서' : 'Documentation'}">${nav}</nav><a class="docs-lab" href="https://lab.wschoi.com">IOES-Lab · KMOU</a></aside>
<main id="main" class="docs-content"><div class="docs-breadcrumb">POSIM <span>/</span> ${esc(ko ? page.sectionKo : page.sectionEn)}</div>${body}${navigation}<footer class="docs-footer"><span>POSIM · ROS 2 Lyrical · Gazebo Jetty</span><a href="https://github.com/IOES-Lab/POSIM/edit/main/website/content/${lang}/${page.slug}.md">${ko ? '이 문서 편집' : 'Edit this page'}</a><div class="docs-source-note">${ko ? '문서 기준' : 'Documentation basis'}: <a href="https://github.com/IOES-Lab/POSIM/tree/${sources.sourceRevision}">${esc(sources.sourceRevision.slice(0, 7))}</a>${sourceLinks ? ` · Notion: ${sourceLinks}` : ''}</div></footer></main>
<aside class="docs-toc" aria-label="${ko ? '이 페이지의 내용' : 'On this page'}"><strong>${ko ? '이 페이지의 내용' : 'On this page'}</strong>${toc.map(item => `<a class="level-${item.depth}" href="#${esc(item.id)}">${esc(item.title)}</a>`).join('')}</aside></div></body></html>`;
    await writeFile(path.join(output, ko ? 'ko' : '', page.slug + '.html'), html);
    index.push({ title, url: page.slug + '.html', section: ko ? page.sectionKo : page.sectionEn, description, text: plain(body) });
  }
  await writeFile(path.join(output, 'assets', `search.${lang}.json`), JSON.stringify(index));
}
const urls = pages.flatMap(page => [`/${page.slug === 'index' ? '' : page.slug + '.html'}`, `/ko/${page.slug === 'index' ? '' : page.slug + '.html'}`]);
const siteUrl = process.env.SITE_URL;
if (siteUrl) {
  const site = new URL(siteUrl);
  if (!['https:', 'http:'].includes(site.protocol)) throw new Error('SITE_URL must be an HTTP(S) address');
  const origin = site.origin + site.pathname.replace(/\/$/, '');
  await writeFile(path.join(output, 'sitemap.xml'), '<?xml version="1.0" encoding="UTF-8"?><urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">' + urls.map(url => `<url><loc>${esc(origin + url)}</loc></url>`).join('') + '</urlset>');
  await writeFile(path.join(output, 'robots.txt'), `User-agent: *\nAllow: /\nSitemap: ${origin}/sitemap.xml\n`);
}
console.log(`Built ${pages.length * 2} English/Korean pages from local Markdown. No Notion connection is needed at runtime.`);
