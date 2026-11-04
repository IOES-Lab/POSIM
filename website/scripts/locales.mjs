import { readFileSync } from 'node:fs';

const codes = ['en', 'es', 'fr', 'de', 'it', 'ko', 'ja', 'zh'];
export const messages = Object.fromEntries(codes.map(code => [code,
  JSON.parse(readFileSync(new URL(`../i18n/${code}.json`, import.meta.url), 'utf8')),
]));
export const languages = codes.map(code => ({
  code, label: messages[code].name, directory: code === 'en' ? '' : `${code}/`,
}));

for (const code of codes) {
  for (const group of ['ui', 'pages']) {
    for (const key of Object.keys(messages.en[group])) {
      if (!messages[code][group][key]) throw new Error(`${code}: missing ${group}.${key}`);
    }
  }
  if (messages[code].sections.length !== messages.en.sections.length) {
    throw new Error(`${code}: incomplete navigation groups`);
  }
}
