import { createHash } from 'node:crypto';
import { mkdir, readFile, writeFile } from 'node:fs/promises';
import { gunzipSync } from 'node:zlib';

const base = 'https://raw.githubusercontent.com/saularandia/front_project/viajecitos-clean-528/.viajecitos-clean/5.2.8/payload';
const names = ['00a', '00b', '01', '02', '03a', '03b', '04', '05'];
let encoded = '';
for (const name of names) {
  const response = await fetch(`${base}/${name}.txt`, { cache: 'no-store' });
  if (!response.ok) throw new Error(`No se puede cargar el fragmento ${name}: HTTP ${response.status}`);
  encoded += (await response.text()).trim();
}
if (encoded.length !== 22519) throw new Error(`Payload incompleto: ${encoded.length}`);
if (createHash('sha256').update(encoded).digest('hex') !== 'db5f3186b6491f25b252847d03e5dfa94c3b8510e30b45e510494e87cad47f0b') throw new Error('El payload no coincide con la versión auditada.');
const padded = encoded + '='.repeat((4 - encoded.length % 4) % 4);
const buffer = gunzipSync(Buffer.from(padded.replace(/-/g, '+').replace(/_/g, '/'), 'base64'));
let offset = 0;
const extracted = [];
while (offset + 512 <= buffer.length) {
  const header = buffer.subarray(offset, offset + 512);
  if (header.every((byte) => byte === 0)) break;
  const name = header.subarray(0, 100).toString().replace(/\0.*$/, '');
  const size = Number.parseInt(header.subarray(124, 136).toString().replace(/\0.*$/, '').trim() || '0', 8) || 0;
  const type = String.fromCharCode(header[156] || 48);
  offset += 512;
  if (type === '0' || type === '\0') {
    const slash = name.lastIndexOf('/');
    if (slash >= 0) await mkdir(name.slice(0, slash), { recursive: true });
    await writeFile(name, buffer.subarray(offset, offset + size));
    extracted.push(name);
  }
  offset += Math.ceil(size / 512) * 512;
}
for (const required of ['build.mjs', 'enhancements.js', 'enhancements.css']) if (!extracted.includes(required)) throw new Error(`Falta ${required}.`);

let enhancementSource = await readFile('enhancements.js', 'utf8');
const declarationBefore = '  let suppressClickUntil = 0;\n  let activeSwipe = null;';
const declarationAfter = '  let suppressClickUntil = 0;\n  let performingSwipeAction = false;\n  let activeSwipe = null;';
if (!enhancementSource.includes(declarationBefore)) throw new Error('No se localiza la declaración de control de gestos.');
enhancementSource = enhancementSource.replace(declarationBefore, declarationAfter);
const actionBefore = `      setTimeout(() => {\n        const action = shouldDelete ? item.querySelector('[data-action="delete-item"]') : item.querySelector(kind === 'task' ? '[data-action="toggle-task"]' : '[data-action="toggle-packing"]');\n        action?.click();\n        resetSwipe(item);\n      }, 135);`;
const actionAfter = `      setTimeout(() => {\n        const action = shouldDelete ? item.querySelector('[data-action="delete-item"]') : item.querySelector(kind === 'task' ? '[data-action="toggle-task"]' : '[data-action="toggle-packing"]');\n        performingSwipeAction = true;\n        try { action?.click(); } finally { performingSwipeAction = false; }\n        resetSwipe(item);\n      }, 135);`;
if (!enhancementSource.includes(actionBefore)) throw new Error('No se localiza la ejecución del gesto.');
enhancementSource = enhancementSource.replace(actionBefore, actionAfter);
const guardBefore = "      if (Date.now() < suppressClickUntil && event.target.closest('.editable-card')) {";
const guardAfter = "      if (!performingSwipeAction && Date.now() < suppressClickUntil && event.target.closest('.editable-card')) {";
if (!enhancementSource.includes(guardBefore)) throw new Error('No se localiza el bloqueo del clic posterior al gesto.');
enhancementSource = enhancementSource.replace(guardBefore, guardAfter);
await writeFile('enhancements.js', enhancementSource);

let buildSource = await readFile('build.mjs', 'utf8');
const oldCheck = 'check(original.length === 55546, `JavaScript estable alterado: ${original.length} bytes`);';
const newCheck = 'check(Buffer.byteLength(original, "utf8") === 55546, `JavaScript estable alterado: ${Buffer.byteLength(original, "utf8")} bytes`);';
if (!buildSource.includes(oldCheck)) throw new Error('No se localiza la comprobación de bytes que debe corregirse.');
buildSource = buildSource.replace(oldCheck, newCheck).replaceAll('appBytes: originalApp.length', 'appBytes: Buffer.byteLength(originalApp, "utf8")');
await writeFile('build.mjs', buildSource);
await import('./build.mjs');
