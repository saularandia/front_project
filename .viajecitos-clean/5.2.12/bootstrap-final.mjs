import { createHash } from 'node:crypto';
import { writeFile } from 'node:fs/promises';

const RELEASE = '5.2.12-performance';
const ROOT = 'https://raw.githubusercontent.com/saularandia/front_project/viajecitos-clean-528/.viajecitos-clean';
const FILES = {
  base: {
    url: `${ROOT}/5.2.10/bootstrap-final.mjs`,
    gitSha: '27a811aa4d0101a5bae9a1104b3da9938a04525e',
  },
  runtime: {
    url: `${ROOT}/5.2.12/feature-5212.mjs`,
    gitSha: 'ae2df67be58048a063639dc53cdbb8a71239b0c0',
  },
  css: {
    url: `${ROOT}/5.2.12/feature-5212.css`,
    gitSha: '7dd7c5397e3f27358924943fffb40bbb502417f1',
  },
  test: {
    url: `${ROOT}/5.2.12/feature-5212-test.mjs`,
    gitSha: '145460180cc02f1e3c8d06dd4de9bf915fcd3031',
  },
  patch: {
    url: `${ROOT}/5.2.12/performance-patch.mjs`,
    gitSha: '69fa4c1055e4c7b64bd2092b5b1347158a3a255c',
  },
  lifecycle: {
    url: `${ROOT}/5.2.12/lifecycle-fix.mjs`,
    gitSha: 'c47db2a401d6083b16f180bc5d7c6337a98d5da7',
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

const [baseSource, runtimeSource, performanceCss, performanceTest, patchSource, lifecycleSource] = await Promise.all(
  Object.entries(FILES).map(([name, spec]) => fetchVerified(name, spec)),
);

if (!runtimeSource.includes(`const RELEASE = '${RELEASE}'`)) throw new Error('La extensión de rendimiento tiene una versión incorrecta.');
if (runtimeSource.includes('new MutationObserver')) throw new Error('La extensión intenta instalar un observador duplicado.');
if (!runtimeSource.includes('shared-batched-observer') || !runtimeSource.includes('data-vjt-trip-select-all')) throw new Error('Faltan el observador compartido o la selección global de miembros.');
if (!performanceCss.includes('content-visibility: auto') || !performanceCss.includes('will-change: transform')) throw new Error('Faltan las optimizaciones de renderizado y gestos.');
if (!patchSource.includes('patchPayloadBridge') || !patchSource.includes('navigationPreload') || !patchSource.includes('minify: true')) throw new Error('La canalización de rendimiento está incompleta.');
if (!lifecycleSource.includes('patchLifecycleSafety') || !lifecycleSource.includes('pagehide')) throw new Error('Falta la protección del ciclo de vida del observador.');

await writeFile('feature-5212.mjs', runtimeSource);
await writeFile('feature-5212.css', performanceCss);
await writeFile('feature-5212-test.mjs', performanceTest);
await writeFile('performance-patch.mjs', patchSource);
await writeFile('lifecycle-fix.mjs', lifecycleSource);

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
  `const { patchEnhancementSource, patchEnhancementCss, patchBuildSource } = await import('./performance-patch.mjs');
const { patchLifecycleSafety } = await import('./lifecycle-fix.mjs');
enhancementSource = patchLifecycleSafety(patchEnhancementSource(enhancementSource, VERSION));
enhancementSource += \`\\n\\n\${await readFile('feature-5212.mjs', 'utf8')}\\n\`;
const combinedObserverCount = (enhancementSource.match(/new MutationObserver\\(/g) || []).length;
if (combinedObserverCount !== 1) throw new Error(\`La aplicación final debe tener un solo MutationObserver; tiene \${combinedObserverCount}.\`);
if (!enhancementSource.includes("globalThis.addEventListener?.('pagehide'")) throw new Error('La aplicación no desconecta el observador al abandonar la página.');
await writeFile('enhancements.js', enhancementSource);`,
  'la escritura del JavaScript optimizado',
);
replaceOnce(
  "await writeFile('enhancements.css', enhancementCss);",
  `enhancementCss = patchEnhancementCss(enhancementCss, VERSION);
enhancementCss += \`\\n\\n\${await readFile('feature-5212.css', 'utf8')}\\n\`;
await writeFile('enhancements.css', enhancementCss);`,
  'la escritura del CSS optimizado',
);
replaceOnce(
  "await writeFile('build.mjs', buildSource);",
  `buildSource = patchBuildSource(buildSource, VERSION);
await writeFile('build.mjs', buildSource);`,
  'la escritura de la compilación optimizada',
);
replaceOnce(
  "await import('./build.mjs');",
  `const { spawnSync } = await import('node:child_process');
const performanceTestRun = spawnSync(process.execPath, ['feature-5212-test.mjs'], { encoding: 'utf8' });
if (performanceTestRun.stdout) process.stdout.write(performanceTestRun.stdout);
if (performanceTestRun.stderr) process.stderr.write(performanceTestRun.stderr);
if (performanceTestRun.status !== 0) throw new Error('Han fallado las pruebas aisladas de rendimiento.');
console.log('VJT_PERFORMANCE_PIPELINE_READY', JSON.stringify({
  version: VERSION,
  singleObserver: true,
  lifecycleSafeObserver: true,
  fetchDoubleParse: false,
  swipeRaf: true,
  expenseRaf: true,
  linkPreviewRaf: true,
  adaptiveSync: true,
  contentVisibility: true,
  navigationPreload: true,
  modulePreload: true,
  minifiedAssets: true,
  minifiedApi: true,
}));
await import('./build.mjs');`,
  'la ejecución final con pruebas de rendimiento',
);

if (!patched.includes(`const VERSION = '${RELEASE}';`)) throw new Error('No se ha actualizado la versión final.');
if (!patched.includes('patchEnhancementSource') || !patched.includes('patchBuildSource') || !patched.includes('patchLifecycleSafety')) throw new Error('No se ha conectado la canalización completa de optimización.');
if (!patched.includes('feature-5212-test.mjs')) throw new Error('No se han incorporado las pruebas de rendimiento.');
await writeFile('bootstrap-5212-patched.mjs', patched);
await import('./bootstrap-5212-patched.mjs');
