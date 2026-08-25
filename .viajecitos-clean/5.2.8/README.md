# Viajecitos 5.2.8-clean

Versión de recuperación limpia construida sobre la interfaz móvil estable restaurada (5.1.4), sin sustituirla por una reconstrucción alternativa.

## Alcance funcional

- Se conserva exactamente la base visual y de navegación estable.
- Sandra 🦖 se incorpora al catálogo de perfiles y a todos los selectores de participantes.
- Gestos móviles:
  - Tareas y equipaje: derecha para completar/pendiente; izquierda para eliminar con confirmación.
  - Actividades, gastos y enlaces: izquierda para eliminar con confirmación.
  - Encuestas: sin gesto destructivo.
- Gastos con reparto igual, cantidades exactas, porcentajes o partes.
- Iconos automáticos para Spotify, YouTube, Instagram, Google Maps y fallback web.

## Arquitectura limpia

- Frontend, API y almacenamiento funcionan bajo el mismo origen.
- No hay proxies, iframes, cargadores remotos ni dependencias de deployments antiguos durante la ejecución.
- Una sola capa de mejoras integrada en el JavaScript final.
- Service worker con una única caché versionada y eliminación de cachés anteriores.
- Los activos finales llevan hash de contenido.

## Puerta de calidad

El build se detiene antes de generar un deployment si falla cualquiera de estas comprobaciones:

- Integridad de la base estable (55.546 bytes de JavaScript y 39.940 bytes de CSS).
- Seis perfiles en login y Sandra en «Nuevo viaje».
- Cuatro modalidades de reparto y serialización exacta.
- Gestos de izquierda/derecha y ausencia de gestos en encuestas.
- Iconos de plataformas.
- API real reconstruida y probada contra un estado Blob aislado que se elimina después.
- Ausencia de dependencias externas en tiempo de ejecución.

El entrypoint auditado es `bootstrap-final.mjs`. Los fragmentos bajo `payload/` forman un paquete comprimido cuya concatenación tiene SHA-256 `db5f3186b6491f25b252847d03e5dfa94c3b8510e30b45e510494e87cad47f0b`.
