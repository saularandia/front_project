import { writeFile } from 'node:fs/promises';

const sourceUrl = 'https://raw.githubusercontent.com/saularandia/front_project/viajecitos-clean-528/.viajecitos-clean/5.2.8/bootstrap-final.mjs';
const response = await fetch(sourceUrl, { cache: 'no-store' });
if (!response.ok) throw new Error(`No se puede cargar el build auditado: HTTP ${response.status}`);
const source = await response.text();
if (!source.includes('db5f3186b6491f25b252847d03e5dfa94c3b8510e30b45e510494e87cad47f0b')) {
  throw new Error('El build auditado no contiene la huella esperada.');
}
// Se escribe en la raíz porque bootstrap-final.mjs extrae e importa build.mjs de forma relativa.
await writeFile('bootstrap-final.mjs', source);
await import('./bootstrap-final.mjs');
