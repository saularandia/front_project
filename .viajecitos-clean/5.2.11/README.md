# Viajecitos 5.2.11 — Spotify y selección global de miembros

Versión incremental construida sobre `5.2.10-swipe-text-only`, conservando la interfaz móvil estable, los gestos con texto, el reparto avanzado, los iconos automáticos y Sandra 🦖.

## Cambios

- Spotify utiliza un grafismo más fiel: fondo negro, disco verde `#1ED760` y tres ondas oscuras, sin el círculo blanco invertido anterior.
- El formulario de creación y edición de viajes incorpora un control equivalente al de gastos:
  - `Seleccionar todos` incluye todos los perfiles disponibles.
  - `Deseleccionar todos` deja seleccionado únicamente al creador, que continúa siendo obligatorio.
  - El control muestra el recuento actual y el estado intermedio cuando solo hay parte de los miembros seleccionados.

## Garantías

- No modifica la estructura de las tarjetas, la navegación ni los formularios móviles existentes.
- No altera la detección de Instagram, YouTube, Google Maps ni el fallback web.
- No modifica los datos reales durante las pruebas.
- El build hereda y vuelve a ejecutar las pruebas de login, Sandra, repartos, gestos, enlaces y API aislada.
- Añade una prueba JSDOM específica para el nuevo grafismo de Spotify y para seleccionar/deseleccionar los seis perfiles manteniendo al creador.
- El resultado final continúa siendo autocontenido y sin dependencias de deployments antiguos en tiempo de ejecución.
