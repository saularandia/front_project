import { JSDOM } from 'jsdom';

const dom = new JSDOM(`<!doctype html><html><body>
  <fieldset class="trip-participants-fieldset">
    <legend>¿Quiénes van?</legend>
    <p>El creador se incluye siempre.</p>
    <div id="trip-participants" class="trip-participant-grid">
      <label><input type="checkbox" name="participants" value="saul" checked disabled></label>
      <label><input type="checkbox" name="participants" value="lumi" checked></label>
      <label><input type="checkbox" name="participants" value="boro"></label>
      <label><input type="checkbox" name="participants" value="paulus"></label>
      <label><input type="checkbox" name="participants" value="clau"></label>
      <label><input type="checkbox" name="participants" value="sandra"></label>
    </div>
  </fieldset>
  <span class="vjt-platform-icon vjt-platform-spotify"><svg><circle fill="#fff"></circle></svg></span>
</body></html>`, { url: 'https://test.local/', pretendToBeVisual: true });

Object.assign(globalThis, {
  window: dom.window,
  document: dom.window.document,
  HTMLInputElement: dom.window.HTMLInputElement,
  Event: dom.window.Event,
  CustomEvent: dom.window.CustomEvent,
});

await import(`${new URL('./feature-5212.mjs', import.meta.url).href}?test=${Date.now()}`);
await new Promise((resolve) => setTimeout(resolve, 20));

const assert = (condition, message) => {
  if (!condition) throw new Error(`VJT_5212_TEST_FAILED: ${message}`);
};

const spotify = document.querySelector('.vjt-platform-spotify');
assert(spotify.dataset.vjtSpotifyPolished === '5.2.12-performance', 'Spotify no se ha actualizado.');
assert(spotify.querySelectorAll('path').length === 3, 'El símbolo de Spotify no tiene tres ondas.');
assert(spotify.querySelector('circle')?.getAttribute('fill') === 'currentColor', 'El disco de Spotify no usa el verde de marca.');
assert(!spotify.innerHTML.includes('fill="#fff"'), 'Permanece el círculo blanco antiguo.');

const master = document.querySelector('[data-vjt-trip-select-all]');
const label = document.querySelector('[data-vjt-trip-select-all-label]');
const count = document.querySelector('[data-vjt-trip-selection-count]');
const participants = [...document.querySelectorAll('#trip-participants input[name="participants"]')];
assert(master && label && count, 'No se ha creado el selector global de miembros.');
assert(master.indeterminate === true, 'El estado parcial inicial no es indeterminado.');
assert(count.textContent === '2/6', `El recuento inicial es ${count.textContent}.`);

master.checked = true;
master.dispatchEvent(new Event('change', { bubbles: true }));
assert(participants.every((input) => input.checked), 'Seleccionar todos no marca los seis perfiles.');
assert(label.textContent === 'Deseleccionar todos', 'La etiqueta no cambia al seleccionar todos.');
assert(count.textContent === '6/6', 'El recuento total no es 6/6.');

master.checked = false;
master.dispatchEvent(new Event('change', { bubbles: true }));
assert(participants[0].checked && participants.slice(1).every((input) => !input.checked), 'Deseleccionar todos no conserva solo al creador.');
assert(label.textContent === 'Seleccionar todos', 'La etiqueta no vuelve al estado inicial.');
assert(count.textContent === '1/6', 'El recuento final no conserva al creador.');

const dynamic = document.createElement('span');
dynamic.className = 'vjt-platform-icon vjt-platform-spotify';
dynamic.innerHTML = '<svg><circle fill="#fff"></circle></svg>';
document.body.appendChild(dynamic);
document.dispatchEvent(new CustomEvent('vjt:enhance', { detail: { roots: [dynamic] } }));
assert(dynamic.dataset.vjtSpotifyPolished === '5.2.12-performance', 'El evento compartido no mejora contenido dinámico.');
assert(window.__VJT_5212?.observer === 'shared-batched-observer', 'La extensión no declara el observador compartido.');
const featureSource = String(await import('node:fs/promises').then(({ readFile }) => readFile(new URL('./feature-5212.mjs', import.meta.url), 'utf8')));
assert(!featureSource.includes('new MutationObserver'), 'La extensión instala un segundo MutationObserver.');

console.log('VJT_5212_EXTENSION_TESTS_OK', JSON.stringify({
  spotify: 'black-square-green-disc',
  spotifyWaves: 3,
  selectAll: true,
  deselectAllKeepsOwner: true,
  profiles: 6,
  sharedObserver: true,
  dynamicEnhancementEvent: true,
}));
dom.window.close();
