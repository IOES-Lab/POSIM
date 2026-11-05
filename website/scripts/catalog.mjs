import { readdir, writeFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
const root = fileURLToPath(new URL('../../', import.meta.url));
const worlds = (await readdir(root + 'models/posim_worlds/worlds')).filter(f => f.endsWith('.world')).sort();
const objects = (await readdir(root + 'models/posim_object_models/description', { withFileTypes: true }))
  .filter(f => f.isDirectory()).map(f => f.name).sort();
await writeFile(new URL('../catalog.json', import.meta.url), JSON.stringify({ worlds, objects }, null, 2) + '\n');
console.log(`Saved ${worlds.length} worlds and ${objects.length} object descriptions.`);
