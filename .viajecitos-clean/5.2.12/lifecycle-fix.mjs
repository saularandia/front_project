function replaceOnce(source, before, after, label) {
  const first = source.indexOf(before);
  if (first < 0) throw new Error(`No se localiza ${label}.`);
  if (source.indexOf(before, first + before.length) >= 0) throw new Error(`${label} aparece más de una vez.`);
  return `${source.slice(0, first)}${after}${source.slice(first + before.length)}`;
}

export function patchLifecycleSafety(input) {
  let source = input;
  source = replaceOnce(
    source,
    `  function enhanceRoot(root = document) {
    const scope = root?.querySelectorAll ? root : document;
    enhanceExpenseForms(scope);
    enhanceLinks(scope);
    enhanceSwipeRows(scope);
  }`,
    `  function enhanceRoot(root = globalThis.document) {
    const doc = globalThis.document;
    if (!doc) return;
    const scope = root?.querySelectorAll ? root : doc;
    enhanceExpenseForms(scope);
    enhanceLinks(scope);
    enhanceSwipeRows(scope);
  }`,
    'la protección del documento al mejorar una raíz',
  );
  source = replaceOnce(
    source,
    `  function compactEnhanceRoots() {
    const roots = [...pendingEnhanceRoots].filter((root) => root === document || root?.isConnected);
    pendingEnhanceRoots.clear();
    if (!roots.length || roots.includes(document) || roots.includes(document.body)) return [document];
    return roots.filter((root, index) => !roots.some((other, otherIndex) => otherIndex !== index && other?.contains?.(root)));
  }`,
    `  function compactEnhanceRoots() {
    const doc = globalThis.document;
    if (!doc) { pendingEnhanceRoots.clear(); return []; }
    const roots = [...pendingEnhanceRoots].filter((root) => root === doc || root?.isConnected);
    pendingEnhanceRoots.clear();
    if (!roots.length || roots.includes(doc) || roots.includes(doc.body)) return [doc];
    return roots.filter((root, index) => !roots.some((other, otherIndex) => otherIndex !== index && other?.contains?.(root)));
  }`,
    'la protección del documento al compactar raíces',
  );
  source = replaceOnce(
    source,
    `  function queueEnhance(root = document) {
    const target = root === document || root?.nodeType === 1 ? root : document;
    pendingEnhanceRoots.add(target);
    if (enhanceQueued) return;
    enhanceQueued = true;
    requestAnimationFrame(() => {
      enhanceQueued = false;
      const roots = compactEnhanceRoots();
      for (const scope of roots) enhanceRoot(scope);
      installSwipeDelegation();
      document.dispatchEvent(new CustomEvent('vjt:enhance', { detail: { roots } }));
    });
  }`,
    `  function queueEnhance(root = globalThis.document) {
    const doc = globalThis.document;
    if (!doc) return;
    const target = root === doc || root?.nodeType === 1 ? root : doc;
    pendingEnhanceRoots.add(target);
    if (enhanceQueued) return;
    enhanceQueued = true;
    requestAnimationFrame(() => {
      enhanceQueued = false;
      const currentDocument = globalThis.document;
      if (!currentDocument) { pendingEnhanceRoots.clear(); return; }
      const roots = compactEnhanceRoots();
      for (const scope of roots) enhanceRoot(scope);
      installSwipeDelegation();
      currentDocument.dispatchEvent(new CustomEvent('vjt:enhance', { detail: { roots } }));
    });
  }`,
    'la protección del documento al programar mejoras',
  );
  source = replaceOnce(
    source,
    `  const root = document.body || appRoot();
  if (root) observer.observe(root, { childList: true, subtree: true });
  globalThis.matchMedia?.('(max-width: 720px), (pointer: coarse)')?.addEventListener?.('change', () => queueEnhance(document));
  queueEnhance(document);`,
    `  const root = globalThis.document?.body || appRoot();
  if (root) observer.observe(root, { childList: true, subtree: true });
  globalThis.addEventListener?.('pagehide', () => observer.disconnect(), { once: true });
  globalThis.matchMedia?.('(max-width: 720px), (pointer: coarse)')?.addEventListener?.('change', () => queueEnhance(globalThis.document));
  queueEnhance(globalThis.document);`,
    'la desconexión del observador al cerrar la página',
  );
  if (!source.includes("globalThis.addEventListener?.('pagehide'")) throw new Error('No se desconecta el observador durante pagehide.');
  if (source.includes('roots.includes(document.body)')) throw new Error('Permanece una referencia insegura a document.body.');
  return source;
}
