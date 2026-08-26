# Viajecitos 5.2.12-performance

Versión de rendimiento construida sobre la interfaz móvil estable y sobre todas las funciones validadas hasta 5.2.11.

## Optimizaciones

- Un único `MutationObserver`, compartido, agrupado por fotograma y limitado a las raíces modificadas.
- Desconexión segura del observador al abandonar la página.
- Eliminación del segundo parseo JSON de cada respuesta de `/api/rumbo`.
- Gestos limitados a un repintado por `requestAnimationFrame`.
- Cálculo de repartos y previsualización de enlaces agrupados por fotograma.
- Reutilización de instancias de `Intl.NumberFormat`.
- Sincronización adaptativa: 20 s en uso normal, 60 s con ahorro de datos o 2G y actualización inmediata al recuperar foco, conexión o BFCache.
- `content-visibility`, `contain` y `will-change` solo durante interacciones activas.
- JavaScript y CSS finales minificados con hash de contenido.
- API minificada y con tree-shaking.
- `modulepreload` del JavaScript principal y precarga del icono.
- Service worker con Navigation Preload, caché de activos inmutables y actualización stale-while-revalidate del manifiesto.

## Funciones preservadas

- Seis perfiles, incluida Sandra 🦖, en login y todos los selectores.
- Seleccionar todos/ninguno al crear o editar un viaje, conservando siempre al creador.
- Reparto de gastos igual, exacto, porcentual o por partes.
- Gestos móviles con texto únicamente: «Completar» y «Eliminar».
- Eliminación lateral de actividades, gastos y enlaces.
- Encuestas sin gesto destructivo.
- Detección e iconos de Spotify, YouTube, Instagram, Google Maps y fallback web.
- Grafismo corregido de Spotify.

## Puerta de calidad

La compilación se interrumpe si falla cualquiera de las pruebas de interfaz, API, almacenamiento aislado, perfiles, repartos, gestos, iconos o rendimiento.

Preview validado:

- Deployment: `dpl_8wUkhbktwGFaps3kongaf6nAqdKe`
- Inspector: `https://vercel.com/me-51a3/rumbo-viajes-amigos/8wUkhbktwGFaps3kongaf6nAqdKe`
- Estado: `READY`

Resultados principales del build:

- JavaScript: 86.530 → 74.989 bytes tras minificación.
- CSS: 46.639 → 45.775 bytes tras minificación.
- Un único observador de mutaciones.
- Sin doble parseo de respuestas API.
- Frontend autocontenido y sin dependencias de deployments anteriores durante la ejecución.

El entrypoint reproducible es `production-loader.mjs`, que valida la huella Git del build antes de ejecutarlo.
