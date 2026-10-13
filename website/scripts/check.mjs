import { access, readFile, readdir } from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { createHash } from 'node:crypto';
import { pages } from './pages.mjs';
import { checkPublicWording } from './public-wording.mjs';
const root = fileURLToPath(new URL('../', import.meta.url));
const repository = path.resolve(root, '..');
const output = path.join(root, 'dist');
const fail = message => { throw new Error(message); };
const exists = async file => access(file).then(() => true).catch(() => false);
const htmlFiles = pages.flatMap(page => [page.slug + '.html', 'ko/' + page.slug + '.html']);
const documents = new Map();
const sources = JSON.parse(await readFile(path.join(root, 'sources.json'), 'utf8'));
const catalog = JSON.parse(await readFile(path.join(root, 'catalog.json'), 'utf8'));
const mediaSources = JSON.parse(await readFile(path.join(root, 'media-sources.json'), 'utf8'));
if (catalog.sourceRevision !== sources.sourceRevision) fail('Catalog/source revision mismatch');
let links = 0, commands = 0;
const haveSource = await exists(path.join(repository, 'models/posim_worlds/worlds'));
if (haveSource) {
  for (const file of ['README.md', 'examples/posim_demos/README.md', 'extras/surface/README.md', 'gazebo/posim_gz_multibeam_sonar/README.md']) {
    if (await exists(path.join(repository, file))) {
      checkPublicWording(await readFile(path.join(repository, file), 'utf8'), '../' + file);
    }
  }
}
for (const file of htmlFiles) {
  const html = await readFile(path.join(output, file), 'utf8');
  documents.set(file, html);
  const lang = file.startsWith('ko/') ? 'ko' : 'en';
  if (!html.includes(`<html lang="${lang}">`)) fail(`${file}: wrong language`);
  if ((html.match(/<h1\b/g) || []).length !== 1) fail(`${file}: expected one h1`);
  if (!html.includes('aria-current="page"')) fail(`${file}: missing active navigation`);
  const ids = [...html.matchAll(/\bid="([^"]+)"/g)].map(match => match[1]);
  if (new Set(ids).size !== ids.length) fail(`${file}: duplicate heading/element IDs`);
  if (/notion-file-block:|<mention-page|\{\{[A-Z_]+\}\}/.test(html)) fail(`${file}: unresolved source markup`);
  for (const image of html.matchAll(/<img\b[^>]*>/g)) {
    if (!/\balt="[^"]+"/.test(image[0])) fail(`${file}: image needs descriptive alt text`);
  }
  if (/dave_(?:demos|interfaces|robot|sensor|world|gz|ros)|dave_(?:world|robot|sensor)\.launch/.test(html)) fail(`${file}: stale package/launch name`);
  const markdown = await readFile(path.join(root, 'content', lang, path.basename(file, '.html') + '.md'), 'utf8');
  checkPublicWording(markdown, `content/${lang}/${path.basename(file, '.html')}.md`);
  if (haveSource) {
    for (const match of markdown.matchAll(/ros2 launch (posim_[a-z_]+) ([a-z_]+\.launch\.py)/g)) {
      const roots = { posim_demos: 'examples/posim_demos', posim_multibeam_sonar_demo: 'gazebo/posim_gz_multibeam_sonar/multibeam_sonar_demo' };
      if (!roots[match[1]] || !await exists(path.join(repository, roots[match[1]], 'launch', match[2]))) fail(`${file}: missing launch ${match[0]}`);
      commands++;
    }
    for (const match of markdown.matchAll(/posim_interfaces\/(srv|msg)\/([A-Za-z0-9]+)/g)) {
      if (!await exists(path.join(repository, 'posim_interfaces', match[1], match[2] + '.' + match[1]))) fail(`${file}: unknown interface ${match[0]}`);
    }
    for (const match of markdown.matchAll(/world_name:=([\w.]+)/g)) {
      if (match[1] !== 'empty.sdf' && !await exists(path.join(repository, 'models/posim_worlds/worlds', match[1] + '.world'))) fail(`${file}: unknown world ${match[1]}`);
    }
    for (const match of markdown.matchAll(/namespace:=([\w]+)/g)) {
      if (match[1] === 'my_robot') continue; // Explicit custom-model tutorial placeholder.
      const robot = path.join(repository, 'models/posim_robot_models/description', match[1], 'model.sdf');
      const sensor = path.join(repository, 'models/posim_sensor_models/description', match[1], 'model.sdf');
      const object = path.join(repository, 'models/posim_object_models/description', match[1], 'model.sdf');
      if (!await exists(robot) && !await exists(sensor) && !await exists(object)) fail(`${file}: unknown descriptor ${match[1]}`);
    }
  }
}
for (const media of mediaSources) {
  if (!media.source_page?.startsWith('https://caring-dibble-be5.notion.site/') || !media.block || !media.title) {
    fail('Media registry entry needs its source page, block and original title');
  }
  if (media.audience === 'engineering') {
    if (await exists(path.join(output, 'assets/media/notion', path.basename(media.file)))) {
      fail(`Engineering figure leaked into the public site: ${media.file}`);
    }
    continue;
  }
  if (media.audience !== 'public' || !media.file.startsWith('media/notion/')) fail(`Unknown media audience/path: ${media.file}`);
  const bytes = await readFile(path.join(output, 'assets', media.file));
  if (createHash('sha256').update(bytes).digest('hex') !== media.sha256) fail(`Source media changed: ${media.file}`);
  for (const lang of ['en', 'ko']) {
    const prefix = lang === 'ko' ? '../assets/' : 'assets/';
    if (![...documents].some(([file, html]) => file.startsWith('ko/') === (lang === 'ko') && html.includes(`src="${prefix}${media.file}"`))) {
      fail(`${lang}: source media missing from guides: ${media.file}`);
    }
  }
}
for (const [file, html] of documents) {
  for (const match of html.matchAll(/\b(?:href|src)="([^"]+)"/g)) {
    const reference = match[1].replaceAll('&amp;', '&');
    if (/^(https?:|mailto:)/.test(reference)) {
      if (haveSource) {
        const source = reference.match(/^https:\/\/github\.com\/IOES-Lab\/POSIM\/(?:blob|tree|edit)\/main\/(.+)$/i);
        if (source && !await exists(path.join(repository, decodeURIComponent(source[1])))) fail(`${file}: missing repository link ${reference}`);
      }
      continue;
    }
    const resolved = new URL(reference, 'https://posim.invalid/' + file);
    const target = decodeURIComponent(resolved.pathname).slice(1);
    if (!await exists(path.join(output, target))) fail(`${file}: broken local link ${reference}`);
    if (resolved.hash) {
      const other = documents.get(target) || await readFile(path.join(output, target), 'utf8');
      const id = decodeURIComponent(resolved.hash.slice(1));
      if (!other.includes(`id="${id}"`)) fail(`${file}: missing anchor ${reference}`);
    }
    links++;
  }
}
for (const lang of ['en', 'ko']) {
  const index = JSON.parse(await readFile(path.join(output, 'assets', `search.${lang}.json`), 'utf8'));
  if (index.length !== pages.length || new Set(index.map(p => p.url)).size !== pages.length) fail(`${lang}: incomplete search index`);
  for (const page of index) if (!page.title || !page.description || !page.text || !documents.has((lang === 'ko' ? 'ko/' : '') + page.url)) fail(`${lang}: invalid search entry`);
}
if (haveSource) {
  const worlds = (await readdir(path.join(repository, 'models/posim_worlds/worlds'))).filter(f => f.endsWith('.world')).sort();
  const objects = (await readdir(path.join(repository, 'models/posim_object_models/description'), { withFileTypes: true })).filter(f => f.isDirectory()).map(f => f.name).sort();
  if (JSON.stringify(worlds) !== JSON.stringify(catalog.worlds) || JSON.stringify(objects) !== JSON.stringify(catalog.objects)) fail('Catalog is stale; run pnpm catalog');
}
console.log(`Checked ${htmlFiles.length} pages, ${links} local links/anchors, ${mediaSources.filter(m => m.audience === 'public').length} bilingual source media, bilingual search${haveSource ? ` and ${commands} source launch references` : ' (standalone website build)'}.`);
