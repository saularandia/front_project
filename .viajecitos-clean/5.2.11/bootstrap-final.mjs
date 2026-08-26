import { createHash } from 'node:crypto';
import { writeFile } from 'node:fs/promises';

const RELEASE = '5.2.11-spotify-members';
const ROOT = 'https://raw.githubusercontent.com/saularandia/front_project/viajecitos-clean-528/.viajecitos-clean';
const FILES = {
  base: {
    url: `${ROOT}/5.2.10/bootstrap-final.mjs`,
    gitSha: '27a811aa4d0101a5bae9a1104b3da9938a04525e',
  },
  feature: {
    url: `${ROOT}/5.2.11/feature-5211.mjs`,
    gitSha: '6f0ffb0a4546516264b3516307e3ecd54e24391b',
  },
  css: {
    url: `${ROOT}/5.2.11/feature-5211.css`,
    gitSha: '9c34eee3fbd6b5baf932c658bbc50c7fb14b3ab5',
  },
  test: {
    url: `${ROOT}/5.2.11/feature-5211-test.mjs`,
    gitSha: '61f5d506733bcb611b365b434a3e795d56d0a9b2',
  },
};

function gitBlobSha(content) {
  const bytes = Buffer.byteLength(content, 'utf8');
  return createHash('sha1').update(`blob ${bytes}\0`).update(content).digest('hex');
}

async function fetchVerified(name, spec) {
  const response = await fetch(spec.url, { cache: 'no-store' });
  if (!response.ok) throw new Error(`No se puede cargar ${name}: HTTP ${response.status}`);
  const content = await response.text();
  const actual = gitBlobSha(content);
  if (actual !== spec.gitSha) throw new Error(`${name} no coincide con la versión auditada: ${actual}`);
  return content;
}

const [baseSource, featureSource, featureCss, featureTest] = await Promise.all(
  Object.entries(FILES).map(([name, spec]) => fetchVerified(name, spec)),
);

if (!featureSource.includes("const RELEASE = '5.2.11-spotify-members'")) throw new Error('La mejora de miembros tiene una versión incorrecta.');
if (!featureSource.includes('data-vjt-trip-select-all')) throw new Error('Falta el selector global de miembros.');
if (!featureSource.includes('stroke="#191414"') || !featureSource.includes('fill="currentColor"')) throw new Error('El nuevo grafismo de Spotify está incompleto.');
if (!featureCss.includes('background: #191414') || !featureCss.includes('color: #1ed760')) throw new Error('Faltan los colores oficiales del grafismo de Spotify.');

await writeFile('feature-5211.mjs', featureSource);
await writeFile('feature-5211.css', featureCss);
await writeFile('feature-5211-test.mjs', featureTest);

let patched = baseSource;
const replaceOnce = (before, after, label) => {
  const first = patched.indexOf(before);
  if (first < 0) throw new Error(`No se localiza ${label}.`);
  if (patched.indexOf(before, first + before.length) >= 0) throw new Error(`${label} aparece más de una vez.`);
  patched = `${patched.slice(0, first)}${after}${patched.slice(first + before.length)}`;
};

replaceOnce(
  "const VERSION = '5.2.10-swipe-text-only';",
  `const VERSION = '${RELEASE}';`,
  'la versión base',
);
replaceOnce(
  "await writeFile('enhancements.js', enhancementSource);",
  "enhancementSource += `\\n\\n${await readFile('feature-5211.mjs', 'utf8')}\\n`;\nawait writeFile('enhancements.js', enhancementSource);",
  'la escritura de JavaScript incremental',
);
replaceOnce(
  "await writeFile('enhancements.css', enhancementCss);",
  "enhancementCss += `\\n\\n${await readFile('feature-5211.css', 'utf8')}\\n`;\nawait writeFile('enhancements.css', enhancementCss);",
  'la escritura de CSS incremental',
);
replaceOnce(
  "await import('./build.mjs');",
  `const { spawnSync } = await import('node:child_process');
const extensionTest = spawnSync(process.execPath, ['feature-5211-test.mjs'], { encoding: 'utf8' });
if (extensionTest.stdout) process.stdout.write(extensionTest.stdout);
if (extensionTest.stderr) process.stderr.write(extensionTest.stderr);
if (extensionTest.status !== 0) throw new Error('Han fallado las pruebas aisladas de Spotify o miembros.');
console.log('VJT_SPOTIFY_MEMBERS_READY', JSON.stringify({
  version: VERSION,
  spotify: 'black-square-green-disc',
  selectAllMembers: true,
  deselectAllKeepsOwner: true,
  preservesSwipeTextOnly: true,
}));
await import('./build.mjs');`,
  'la ejecución final del build',
);

if (!patched.includes(`const VERSION = '${RELEASE}';`)) throw new Error('No se ha actualizado la versión final.');
if (!patched.includes("feature-5211-test.mjs")) throw new Error('No se han incorporado las pruebas de la nueva versión.');
await writeFile('bootstrap-5211-patched.mjs', patched);
await import('./bootstrap-5211-patched.mjs');
