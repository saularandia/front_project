function replaceOnce(source, before, after, label) {
  const first = source.indexOf(before);
  if (first < 0) throw new Error(`No se localiza ${label}.`);
  if (source.indexOf(before, first + before.length) >= 0) throw new Error(`${label} aparece más de una vez.`);
  return `${source.slice(0, first)}${after}${source.slice(first + before.length)}`;
}

function replaceRange(source, startMarker, endMarker, replacement, label) {
  const start = source.indexOf(startMarker);
  const end = source.indexOf(endMarker, start + startMarker.length);
  if (start < 0 || end < 0) throw new Error(`No se localiza ${label}.`);
  return `${source.slice(0, start)}${replacement}${source.slice(end)}`;
}

export function patchEnhancementSource(input, version) {
  let source = input;

  source = replaceOnce(
    source,
    "  const cache = { profiles: [], session: null, trips: [], trip: null };",
    "  const cache = { profiles: [], session: null, trips: [], trip: null };\n  const moneyFormats = new Map();",
    'la caché de formatos monetarios',
  );

  source = replaceRange(
    source,
    '  const nativeFetch = globalThis.fetch?.bind(globalThis);',
    '  function escapeHtml(value) {',
    `  globalThis.addEventListener('vjt:api-payload', (event) => updateCache(event.detail));\n\n`,
    'el puente antiguo que duplicaba el parseo de respuestas',
  );

  const oldMoney = `  function formatMoney(cents, currency = cache.trip?.currency || 'EUR') {\n    try {\n      return new Intl.NumberFormat('es-ES', { style: 'currency', currency }).format((Number(cents) || 0) / 100);\n    } catch {\n      return \`\${((Number(cents) || 0) / 100).toFixed(2)} \${currency}\`;\n    }\n  }`;
  const newMoney = `  function formatMoney(cents, currency = cache.trip?.currency || 'EUR') {\n    try {\n      const code = String(currency || 'EUR');\n      let formatter = moneyFormats.get(code);\n      if (!formatter) {\n        formatter = new Intl.NumberFormat('es-ES', { style: 'currency', currency: code });\n        moneyFormats.set(code, formatter);\n      }\n      return formatter.format((Number(cents) || 0) / 100);\n    } catch {\n      return \`\${((Number(cents) || 0) / 100).toFixed(2)} \${currency}\`;\n    }\n  }`;
  source = replaceOnce(source, oldMoney, newMoney, 'el formateador monetario');

  const oldLinkPreview = `        const render = () => {\n          const platform = detectPlatform(input.value);\n          preview.innerHTML = \`<span class="vjt-platform-icon vjt-platform-\${platform}">\${platformSvg(platform)}</span><span><strong>\${escapeHtml(PLATFORM_META[platform].label)}</strong><small>\${platform === 'web' ? 'Se utilizará el icono genérico' : 'Aplicación detectada automáticamente'}</small></span>\`;\n        };\n        input.addEventListener('input', render);\n        render();`;
  const newLinkPreview = `        let previewFrame = 0;\n        let renderedPlatform = '';\n        const render = () => {\n          previewFrame = 0;\n          const platform = detectPlatform(input.value);\n          if (platform === renderedPlatform) return;\n          renderedPlatform = platform;\n          preview.innerHTML = \`<span class="vjt-platform-icon vjt-platform-\${platform}">\${platformSvg(platform)}</span><span><strong>\${escapeHtml(PLATFORM_META[platform].label)}</strong><small>\${platform === 'web' ? 'Se utilizará el icono genérico' : 'Aplicación detectada automáticamente'}</small></span>\`;\n        };\n        const scheduleRender = () => {\n          if (!previewFrame) previewFrame = requestAnimationFrame(render);\n        };\n        input.addEventListener('input', scheduleRender, { passive: true });\n        render();`;
  source = replaceOnce(source, oldLinkPreview, newLinkPreview, 'la previsualización optimizada de enlaces');

  const calculationAnchor = "    panel.querySelectorAll('[data-vjt-split-mode]').forEach((button) => {";
  const calculationScheduler = `    let calculationFrame = 0;\n    const scheduleCalculate = () => {\n      if (calculationFrame) return;\n      calculationFrame = requestAnimationFrame(() => {\n        calculationFrame = 0;\n        calculate();\n      });\n    };\n    form.__vjtCalculate = () => {\n      if (calculationFrame) {\n        cancelAnimationFrame(calculationFrame);\n        calculationFrame = 0;\n      }\n      calculate();\n    };\n\n`;
  source = replaceOnce(source, calculationAnchor, `${calculationScheduler}${calculationAnchor}`, 'el planificador de cálculos de gastos');
  source = source.replaceAll("input.addEventListener('input', calculate)", "input.addEventListener('input', scheduleCalculate, { passive: true })");
  source = replaceOnce(source, "    amountInput.addEventListener('input', calculate);", "    amountInput.addEventListener('input', scheduleCalculate, { passive: true });", 'el cálculo del importe del gasto');
  source = replaceOnce(
    source,
    "    if (form?.matches?.('form[data-form=\"expense\"]') && form.dataset.vjtSplitValid !== 'true') {",
    "    if (form?.matches?.('form[data-form=\"expense\"]')) form.__vjtCalculate?.();\n    if (form?.matches?.('form[data-form=\"expense\"]') && form.dataset.vjtSplitValid !== 'true') {",
    'la validación síncrona del reparto al guardar',
  );

  source = replaceOnce(
    source,
    '  let performingSwipeAction = false;\n  let activeSwipe = null;',
    '  let performingSwipeAction = false;\n  let swipeFrame = 0;\n  let activeSwipe = null;',
    'el fotograma de los gestos',
  );
  source = replaceOnce(
    source,
    '      activeSwipe.item.style.transform = `translateX(${activeSwipe.dx}px)`;',
    "      if (!swipeFrame) swipeFrame = requestAnimationFrame(() => {\n        swipeFrame = 0;\n        if (activeSwipe) activeSwipe.item.style.transform = `translateX(${activeSwipe.dx}px)`;\n      });",
    'el repintado por fotograma del gesto',
  );
  source = replaceOnce(
    source,
    '      const gesture = activeSwipe;\n      activeSwipe = null;',
    '      if (swipeFrame) { cancelAnimationFrame(swipeFrame); swipeFrame = 0; }\n      const gesture = activeSwipe;\n      activeSwipe = null;',
    'la cancelación del fotograma del gesto',
  );

  const enhanceBlock = `  const pendingEnhanceRoots = new Set();\n\n  function enhanceRoot(root = document) {\n    const scope = root?.querySelectorAll ? root : document;\n    enhanceExpenseForms(scope);\n    enhanceLinks(scope);\n    enhanceSwipeRows(scope);\n  }\n\n  function compactEnhanceRoots() {\n    const roots = [...pendingEnhanceRoots].filter((root) => root === document || root?.isConnected);\n    pendingEnhanceRoots.clear();\n    if (!roots.length || roots.includes(document) || roots.includes(document.body)) return [document];\n    return roots.filter((root, index) => !roots.some((other, otherIndex) => otherIndex !== index && other?.contains?.(root)));\n  }\n\n  function queueEnhance(root = document) {\n    const target = root === document || root?.nodeType === 1 ? root : document;\n    pendingEnhanceRoots.add(target);\n    if (enhanceQueued) return;\n    enhanceQueued = true;\n    requestAnimationFrame(() => {\n      enhanceQueued = false;\n      const roots = compactEnhanceRoots();\n      for (const scope of roots) enhanceRoot(scope);\n      installSwipeDelegation();\n      document.dispatchEvent(new CustomEvent('vjt:enhance', { detail: { roots } }));\n    });\n  }\n\n`;
  source = replaceRange(source, '  function enhanceAll() {', "  document.addEventListener('submit',", enhanceBlock, 'el observador global no segmentado');

  const observerBlock = `  const observer = new MutationObserver((records) => {\n    for (const record of records) queueEnhance(record.target);\n  });\n  const root = document.body || appRoot();\n  if (root) observer.observe(root, { childList: true, subtree: true });\n  globalThis.matchMedia?.('(max-width: 720px), (pointer: coarse)')?.addEventListener?.('change', () => queueEnhance(document));\n  queueEnhance(document);\n\n`;
  source = replaceRange(source, '  const observer = new MutationObserver(queueEnhance);', '  globalThis.__VJT_CLEAN_ENHANCEMENTS__', observerBlock, 'la instalación del observador segmentado');

  const observerCount = (source.match(/new MutationObserver\(/g) || []).length;
  if (observerCount !== 1) throw new Error(`La capa principal debe usar un solo MutationObserver; usa ${observerCount}.`);
  if (source.includes('globalThis.fetch =')) throw new Error('Permanece el parche global de fetch que duplicaba el parseo JSON.');
  if (!source.includes("globalThis.addEventListener('vjt:api-payload'")) throw new Error('Falta el puente de datos sin doble parseo.');
  if (!source.includes('pendingEnhanceRoots') || !source.includes("new CustomEvent('vjt:enhance'")) throw new Error('Falta el observador compartido y segmentado.');
  if (!source.includes('swipeFrame = requestAnimationFrame')) throw new Error('El gesto no está limitado a un repintado por fotograma.');
  if (!source.includes('moneyFormats.get')) throw new Error('No se reutilizan los formateadores monetarios.');
  return source;
}

export function patchEnhancementCss(input, version) {
  const css = input.replaceAll('5.2.8-clean', version);
  if (!css.includes('.vjt-swipe-shell')) throw new Error('El CSS base de gestos no está disponible.');
  return css;
}

function optimizedServiceWorkerFunction() {
  return `function serviceWorker(appPath, cssPath) {\n  const cache = \`viajecitos-\${VERSION}\`;\n  const core = ['/', '/index.html', \`/\${appPath}\`, \`/\${cssPath}\`, '/assets/icon.7cd2ceccdf41.svg', '/manifest.webmanifest'];\n  return \`const CACHE=\${JSON.stringify(cache)};const CORE=\${JSON.stringify(core)};const ASSET=/\\\\/assets\\\\/.*\\\\.[a-f0-9]{12}\\\\./;async function put(key,response){if(response?.ok){const cache=await caches.open(CACHE);await cache.put(key,response.clone())}return response}self.addEventListener('install',event=>event.waitUntil(caches.open(CACHE).then(cache=>cache.addAll(CORE)).then(()=>self.skipWaiting())));self.addEventListener('activate',event=>event.waitUntil(Promise.all([caches.keys().then(keys=>Promise.all(keys.filter(key=>key!==CACHE).map(key=>caches.delete(key)))),self.registration.navigationPreload?.enable?.().catch(()=>{})]).then(()=>self.clients.claim())));self.addEventListener('message',event=>{if(event.data==='SKIP_WAITING'||event.data?.type==='SKIP_WAITING')self.skipWaiting()});self.addEventListener('fetch',event=>{const request=event.request,url=new URL(request.url);if(request.method!=='GET'||url.origin!==self.location.origin||url.pathname.startsWith('/api/'))return;if(request.mode==='navigate'){event.respondWith((async()=>{try{const response=await event.preloadResponse||await fetch(request,{cache:'no-store'});return await put('/index.html',response)}catch{return await caches.match('/index.html')||Response.error()}})());return}if(ASSET.test(url.pathname)){event.respondWith(caches.match(request,{ignoreSearch:true}).then(cached=>cached||fetch(request).then(response=>put(request,response))));return}if(url.pathname==='/manifest.webmanifest'){event.respondWith(caches.match(request,{ignoreSearch:true}).then(cached=>{const update=fetch(request,{cache:'no-cache'}).then(response=>put(request,response)).catch(()=>null);if(cached){event.waitUntil(update);return cached}return update}));return}event.respondWith(caches.match(request,{ignoreSearch:true}).then(cached=>cached||fetch(request)))})\`;\n}\n\n`;
}

export function patchBuildSource(input, version) {
  let source = input;
  source = replaceOnce(source, "import { build } from 'esbuild';", "import { build, transform } from 'esbuild';", 'la importación del minificador');

  const performanceHelpers = `function patchPayloadBridge(source) {\n  const oldCode = '}return Ke(d)}catch(o){';\n  const newCode = '}const __vjtPayload=Ke(d);globalThis.dispatchEvent(new CustomEvent("vjt:api-payload",{detail:d}));return __vjtPayload}catch(o){';\n  check(source.includes(oldCode), 'No se localiza el retorno de datos de la API en la base estable');\n  return source.replace(oldCode, newCode);\n}\n\nfunction patchAdaptivePolling(source) {\n  const oldCode = 'function Xt(){clearInterval(Be),Be=setInterval(mt,15e3)}document.addEventListener("visibilitychange",()=>{document.hidden||mt()}),window.addEventListener("popstate",';\n  const newCode = 'function Zt(){const e=navigator.connection;return e?.saveData||/2g/.test(e?.effectiveType||"")?6e4:2e4}function Xt(){clearTimeout(Be),document.hidden||(Be=setTimeout(async()=>{await mt(),Xt()},Zt()))}document.addEventListener("visibilitychange",()=>{clearTimeout(Be),document.hidden||(mt(),Xt())}),window.addEventListener("online",()=>{mt(),Xt()}),window.addEventListener("focus",mt,{passive:!0}),window.addEventListener("pageshow",e=>{e.persisted&&(mt(),Xt())}),navigator.connection?.addEventListener?.("change",Xt),window.addEventListener("popstate",';\n  check(source.includes(oldCode), 'No se localiza el sondeo fijo de revisiones');\n  return source.replace(oldCode, newCode);\n}\n\n`;
  source = replaceOnce(source, 'function patchStableApp(original, enhancements) {', `${performanceHelpers}function patchStableApp(original, enhancements) {`, 'los parches de rendimiento del frontend estable');
  source = replaceOnce(source, '  source = patchBalanceCalculation(source);\n  source = `${enhancements.trim()}\\n${source}`;', '  source = patchBalanceCalculation(source);\n  source = patchPayloadBridge(source);\n  source = patchAdaptivePolling(source);\n  source = `${enhancements.trim()}\\n${source}`;', 'la aplicación de los parches de datos y sincronización');
  source = replaceOnce(source, "  check(!source.includes('rumbo-viajes-amigos-dy5arlex4'), 'El frontend contiene una dependencia a un deployment externo');", "  check(!source.includes('rumbo-viajes-amigos-dy5arlex4'), 'El frontend contiene una dependencia a un deployment externo');\n  check(source.includes('vjt:api-payload') && !source.includes('setInterval(mt,15e3)'), 'No se aplicaron las optimizaciones de datos y sincronización');", 'la verificación del frontend optimizado');

  source = replaceOnce(source, '  html = html.replace(\'<body>\', `<body data-vjt-release="${VERSION}">`);', '  html = html.replace(\'<body>\', `<body data-vjt-release="${VERSION}">`);\n  html = html.replace(\'</head>\', `<link rel="preload" href="/assets/icon.7cd2ceccdf41.svg" as="image" type="image/svg+xml"><link rel="modulepreload" href="/${appPath}"></head>`);', 'las pistas de carga crítica');
  source = replaceOnce(source, "  check(html.includes(`/${appPath}`) && html.includes(`/${cssPath}`), 'El HTML no referencia los activos finales');", "  check(html.includes(`/${appPath}`) && html.includes(`/${cssPath}`), 'El HTML no referencia los activos finales');\n  check(html.includes('rel=\"modulepreload\"') && html.includes('rel=\"preload\"'), 'El HTML no contiene las pistas de carga crítica');", 'la verificación de las pistas de carga');

  source = replaceRange(source, 'function serviceWorker(appPath, cssPath) {', 'function publicProfiles() {', optimizedServiceWorkerFunction(), 'el service worker optimizado');

  const oldAssets = `const finalApp = patchStableApp(originalApp, enhancementJs);\nconst finalCss = \`\${originalCss}\\n\${enhancementCss.trim()}\\n\`;\nconst appPath = \`assets/app.\${sha256(finalApp).slice(0, 12)}.js\`;\nconst cssPath = \`assets/styles.\${sha256(finalCss).slice(0, 12)}.css\`;`;
  const newAssets = `const finalAppSource = patchStableApp(originalApp, enhancementJs);\nconst finalCssSource = \`\${originalCss}\\n\${enhancementCss.trim()}\\n\`;\nconst finalApp = (await transform(finalAppSource, { minify: true, format: 'esm', target: 'es2020', charset: 'utf8', legalComments: 'none' })).code;\nconst finalCss = (await transform(finalCssSource, { loader: 'css', minify: true, charset: 'utf8', legalComments: 'none' })).code;\ncheck(Buffer.byteLength(finalApp, 'utf8') < Buffer.byteLength(finalAppSource, 'utf8'), 'El JavaScript final no se ha reducido');\ncheck(Buffer.byteLength(finalCss, 'utf8') < Buffer.byteLength(finalCssSource, 'utf8'), 'El CSS final no se ha reducido');\nconst appPath = \`assets/app.\${sha256(finalApp).slice(0, 12)}.js\`;\nconst cssPath = \`assets/styles.\${sha256(finalCss).slice(0, 12)}.css\`;`;
  source = replaceOnce(source, oldAssets, newAssets, 'la minificación de los activos finales');

  const oldBundle = "  await build({ entryPoints: [`${root}/api/rumbo.js`], outfile: destination, bundle: true, platform: 'node', format: 'cjs', target: 'node22', logLevel: 'silent' });";
  const newBundle = "  await build({ entryPoints: [`${root}/api/rumbo.js`], outfile: destination, bundle: true, minify: true, treeShaking: true, legalComments: 'none', platform: 'node', format: 'cjs', target: 'node22', logLevel: 'silent' });";
  source = replaceOnce(source, oldBundle, newBundle, 'la reducción del bundle de la API');

  source = source.replaceAll("features: ['gestos', 'reparto-avanzado', 'iconos-enlaces']", "features: ['gestos', 'reparto-avanzado', 'iconos-enlaces', 'observer-batched', 'navigation-preload', 'adaptive-sync', 'minified-assets']");
  source = replaceOnce(source, '  finalAssets: { appPath, appBytes: finalApp.length, cssPath, cssBytes: finalCss.length },', "  finalAssets: { appPath, appBytes: Buffer.byteLength(finalApp, 'utf8'), cssPath, cssBytes: Buffer.byteLength(finalCss, 'utf8'), sourceAppBytes: Buffer.byteLength(finalAppSource, 'utf8'), sourceCssBytes: Buffer.byteLength(finalCssSource, 'utf8') },", 'las métricas reales de tamaño');
  source = replaceOnce(source, "  tests: ['login', 'nuevo-viaje', 'repartos', 'gestos', 'iconos', 'API-aislada'],", "  performance: { singleObserver: true, fetchDoubleParse: false, swipeRaf: true, adaptiveSync: true, navigationPreload: true, minifiedAssets: true, minifiedApi: true },\n  tests: ['login', 'nuevo-viaje', 'repartos', 'gestos', 'iconos', 'rendimiento', 'API-aislada'],", 'el informe final de rendimiento');

  if (!source.includes("import { build, transform } from 'esbuild'")) throw new Error('No se ha habilitado la minificación de activos.');
  if (!source.includes('navigationPreload') || !source.includes('modulepreload')) throw new Error('Faltan la precarga de navegación o del módulo principal.');
  if (!source.includes('minify: true, treeShaking: true')) throw new Error('La API no se minifica.');
  if (!source.includes('patchPayloadBridge') || !source.includes('patchAdaptivePolling')) throw new Error('Faltan los parches de datos o sincronización adaptativa.');
  return source.replaceAll('5.2.8-clean', version);
}
