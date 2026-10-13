import { readFile, readdir, writeFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
const root = fileURLToPath(new URL('../../', import.meta.url));
const sources = JSON.parse(await readFile(new URL('../sources.json', import.meta.url), 'utf8'));
const worlds = (await readdir(root + 'models/posim_worlds/worlds')).filter(f => f.endsWith('.world')).sort();
const objects = (await readdir(root + 'models/posim_object_models/description', { withFileTypes: true }))
  .filter(f => f.isDirectory()).map(f => f.name).sort();
await writeFile(new URL('../catalog.json', import.meta.url), JSON.stringify({ sourceRevision: sources.sourceRevision, worlds, objects }, null, 2) + '\n');
console.log(`Saved ${worlds.length} worlds and ${objects.length} object descriptions.`);
