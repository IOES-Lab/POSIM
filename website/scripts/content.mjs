import { readFile } from 'node:fs/promises';
import { messages } from './locales.mjs';

const canonical = new Map();
const english = slug => {
  if (!canonical.has(slug)) canonical.set(slug,
    readFile(new URL(`../content/en/${slug}.md`, import.meta.url), 'utf8'));
  return canonical.get(slug);
};
const esc = value => String(value).replace(/[&<>"']/g,
  character => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[character]));

// Executable examples have one source; translations retain their own prose.
export async function readContent(language, slug) {
  let text = await readFile(new URL(`../content/${language}/${slug}.md`, import.meta.url), 'utf8');
  for (const match of [...text.matchAll(/\{\{CODE:([a-z-]+):(\d+)\}\}/g)]) {
    const blocks = [...(await english(match[1])).matchAll(/^```[^\n]*\n[\s\S]*?^```/gm)];
    const code = blocks[Number(match[2])]?.[0];
    if (!code) throw new Error(`${language}/${slug}: unknown example ${match[0]}`);
    text = text.replaceAll(match[0], code);
  }
  for (const match of [...text.matchAll(/\{\{MEDIA:([a-z-]+)(?::(\d+))?\}\}/g)]) {
    const figures = (await english(match[1])).match(/<figure\b[\s\S]*?<\/figure>/g) || [];
    const captions = messages[language].media[match[1]];
    if (!captions || captions.length !== figures.length) {
      throw new Error(`${language}/${slug}: missing localized captions for ${match[1]}`);
    }
    const localized = figures.map((figure, index) => figure
      .replace(/\balt="[^"]*"/g, `alt="${esc(captions[index])}"`)
      .replace(/<figcaption>[\s\S]*?<\/figcaption>/g, `<figcaption>${esc(captions[index])}</figcaption>`)
      .replace(/(<source\b[^>]*src="([^"]+)"[^>]*>)[\s\S]*?<\/video>/g,
        (_, source, url) => `${source}<a href="${url}">${esc(messages[language].ui.openVideo)}</a></video>`));
    const selected = match[2] === undefined ? localized.join('\n') : localized[Number(match[2])];
    if (!selected) throw new Error(`${language}/${slug}: unknown figure ${match[0]}`);
    text = text.replaceAll(match[0], selected);
  }
  if (text.includes('{{OVERVIEW}}')) {
    let figure = (await english('index')).match(/<figure\b[\s\S]*?<\/figure>/)?.[0];
    if (!figure || !messages[language].overview) throw new Error(`${language}: missing overview caption`);
    figure = figure.replace(/<a href="([a-z-]+)\.html" aria-label="[^"]+">([\s\S]*?)<\/a>/g,
      (_, destination, contents) => {
        const label = esc(messages[language].pages[destination]);
        return `<a href="${destination}.html" aria-label="${label}">${contents
          .replace(/\balt="[^"]*"/g, `alt="${label}"`)
          .replace(/<span>[\s\S]*?<\/span>/g, `<span>${label}</span>`)}</a>`;
      }).replace(/<figcaption>[\s\S]*?<\/figcaption>/,
        `<figcaption>${esc(messages[language].overview)}</figcaption>`);
    text = text.replaceAll('{{OVERVIEW}}', figure);
  }
  if (text.includes('{{INTRO_VIDEO}}')) {
    const embed = (await english('index')).match(/<iframe\b[\s\S]*?<\/iframe>/)?.[0];
    if (!embed) throw new Error('Missing introduction video');
    text = text.replaceAll('{{INTRO_VIDEO}}', embed.replace(/title="[^"]*"/, 'title="WWW-POSIM"'));
  }
  if (/\{\{(?:CODE|MEDIA|OVERVIEW|INTRO_VIDEO)\b/.test(text)) {
    throw new Error(`${language}/${slug}: unresolved shared content`);
  }
  return text;
}
