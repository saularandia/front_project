# Viajecitos 5.2.9 — Swipe polish

Corrección visual incremental sobre la versión estable `5.2.8-clean`.

## Cambio

Los fondos verde y rojo de las acciones de deslizamiento ya no quedan visibles alrededor de las esquinas de las tarjetas cuando están en reposo.

- La capa de acciones tiene opacidad `0` en reposo.
- Solo se revela después de detectar intención horizontal real.
- Tocar una tarjeta sin deslizar no activa el fondo.
- El contenedor del gesto y la tarjeta comparten un radio de 15 px.
- Se mantienen los gestos y diálogos de confirmación existentes.

## Regresión cubierta

El build se cancela si falla cualquiera de estas comprobaciones:

- seis perfiles y Sandra disponible al crear un viaje;
- cuatro modalidades de reparto avanzado;
- eliminación por gesto de actividades, gastos y enlaces;
- completar por gesto tareas y equipaje;
- encuestas sin gesto destructivo;
- iconos automáticos de plataformas;
- API probada contra un estado Blob aislado;
- frontend y API autocontenidos, sin dependencias de deployments antiguos;
- fondo de acciones oculto en reposo y activado únicamente tras movimiento horizontal.

## Build reproducible

El entrypoint es `bootstrap-final.mjs`. Reutiliza el payload auditado de `5.2.8` y aplica exclusivamente la corrección visual y de intención del gesto antes de ejecutar la puerta de calidad completa.
