;(() => {
  'use strict';

  const RELEASE = '5.2.12-performance';
  const SPOTIFY_SELECTOR = '.vjt-platform-icon.vjt-platform-spotify';
  const SPOTIFY_SVG = `
    <svg viewBox="0 0 24 24" aria-hidden="true" focusable="false">
      <circle cx="12" cy="12" r="10" fill="currentColor"></circle>
      <path d="M6.1 8.9c4.1-1.2 8.6-.7 12.1 1.4" fill="none" stroke="#191414" stroke-width="1.75" stroke-linecap="round"></path>
      <path d="M6.8 12.3c3.5-.9 7.2-.5 10.3 1.1" fill="none" stroke="#191414" stroke-width="1.6" stroke-linecap="round"></path>
      <path d="M7.5 15.5c2.8-.7 5.7-.4 8.2.8" fill="none" stroke="#191414" stroke-width="1.45" stroke-linecap="round"></path>
    </svg>`;

  function collect(root, selector) {
    const nodes = [];
    if (root?.nodeType === 1 && root.matches?.(selector)) nodes.push(root);
    if (root?.querySelectorAll) nodes.push(...root.querySelectorAll(selector));
    return nodes;
  }

  function polishSpotifyIcons(root = document) {
    for (const icon of collect(root, SPOTIFY_SELECTOR)) {
      if (icon.dataset.vjtSpotifyPolished === RELEASE) continue;
      icon.replaceChildren();
      icon.insertAdjacentHTML('afterbegin', SPOTIFY_SVG);
      icon.dataset.vjtSpotifyPolished = RELEASE;
      icon.setAttribute('aria-hidden', 'true');
      icon.removeAttribute('aria-label');
    }
  }

  function tripInputs(container) {
    return [...(container?.querySelectorAll('input[name="participants"]') || [])];
  }

  function refreshTripSelectAll(container) {
    const fieldset = container?.closest('.trip-participants-fieldset');
    const control = fieldset?.querySelector('[data-vjt-trip-select-all]');
    if (!control) return;

    const inputs = tripInputs(container);
    const selectable = inputs.filter((input) => !input.disabled);
    const selectedSelectable = selectable.filter((input) => input.checked).length;
    const selectedTotal = inputs.filter((input) => input.checked).length;
    const allSelected = selectable.length > 0 && selectedSelectable === selectable.length;

    control.checked = allSelected;
    control.indeterminate = selectedSelectable > 0 && !allSelected;
    control.disabled = selectable.length === 0;
    control.setAttribute('aria-checked', control.indeterminate ? 'mixed' : String(allSelected));

    const label = fieldset.querySelector('[data-vjt-trip-select-all-label]');
    const count = fieldset.querySelector('[data-vjt-trip-selection-count]');
    if (label) label.textContent = allSelected ? 'Deseleccionar todos' : 'Seleccionar todos';
    if (count) count.textContent = `${selectedTotal}/${inputs.length}`;
  }

  function enhanceTripMemberSelector(root = document) {
    for (const container of collect(root, '#trip-participants')) {
      const inputs = tripInputs(container);
      if (!inputs.length) continue;
      const fieldset = container.closest('.trip-participants-fieldset');
      if (!fieldset) continue;

      let row = fieldset.querySelector('[data-vjt-trip-select-all-row]');
      if (!row) {
        row = document.createElement('div');
        row.className = 'vjt-trip-select-all-row';
        row.dataset.vjtTripSelectAllRow = '';
        row.innerHTML = `
          <label class="expense-select-all vjt-trip-select-all">
            <input type="checkbox" data-vjt-trip-select-all aria-label="Seleccionar o deseleccionar todos los miembros">
            <span class="expense-select-all-copy">
              <strong data-vjt-trip-select-all-label>Seleccionar todos</strong>
              <small><span data-vjt-trip-selection-count>0/0</span> perfiles</small>
            </span>
          </label>`;
        fieldset.insertBefore(row, container);
      }
      refreshTripSelectAll(container);
    }
  }

  function enhance(root = document) {
    polishSpotifyIcons(root);
    enhanceTripMemberSelector(root);
  }

  document.addEventListener('vjt:enhance', (event) => {
    const roots = Array.isArray(event.detail?.roots) ? event.detail.roots : [document];
    for (const root of roots) enhance(root);
  });

  document.addEventListener('change', (event) => {
    const target = event.target;
    if (!(target instanceof HTMLInputElement)) return;

    if (target.matches('[data-vjt-trip-select-all]')) {
      const fieldset = target.closest('.trip-participants-fieldset');
      const container = fieldset?.querySelector('#trip-participants');
      if (!container) return;
      const desired = target.checked;
      for (const input of tripInputs(container)) {
        if (!input.disabled) input.checked = desired;
      }
      refreshTripSelectAll(container);
      return;
    }

    if (target.matches('#trip-participants input[name="participants"]')) {
      refreshTripSelectAll(target.closest('#trip-participants'));
    }
  });

  enhance(document);
  document.documentElement.dataset.vjtPerformance = RELEASE;

  window.__VJT_5212 = Object.freeze({
    release: RELEASE,
    observer: 'shared-batched-observer',
    spotify: 'black-square-green-disc',
    selectAllMembers: true,
    deselectAllKeepsOwner: true,
    polishSpotifyIcons,
    enhanceTripMemberSelector,
    refreshTripSelectAll,
  });
})();
