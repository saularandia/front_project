import { createHash } from 'node:crypto';
import { writeFile } from 'node:fs/promises';

const sourceUrl = 'https://raw.githubusercontent.com/saularandia/front_project/viajecitos-clean-528/.viajecitos-clean/5.2.12/bootstrap-final.mjs';
const expectedBlobSha = '6403de6a8f8b017a4f60d629b1b5d7a936620cad';
const response = await fetch(sourceUrl, { cache: 'no-store' });
if (!response.ok) throw new Error(`No se puede cargar el build auditado: HTTP ${response.status}`);
const source = await response.text();
const blobSha = createHash('sha1')
  .update(`blob ${Buffer.byteLength(source, 'utf8')}\0`)
  .update(source)
  .digest('hex');
if (blobSha !== expectedBlobSha) throw new Error(`El build auditado no coincide con la huella esperada: ${blobSha}`);
await writeFile('bootstrap-final.mjs', source);
await import('./bootstrap-final.mjs');
