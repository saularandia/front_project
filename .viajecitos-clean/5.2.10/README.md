# Viajecitos 5.2.10 — acciones de gesto solo con texto

Corrección incremental sobre `5.2.9-swipe-polish`.

## Cambio visual

Los fondos que aparecen durante el deslizamiento muestran únicamente:

- `Completar` al deslizar a la derecha en tareas y equipaje.
- `Eliminar` al deslizar a la izquierda en tareas, equipaje, actividades, gastos y enlaces.

Se han eliminado los símbolos `✓` y `🗑` de estas acciones. Los iconos normales de los botones de cada tarjeta no se modifican.

## Comportamiento conservado

- El fondo permanece oculto en reposo y solo aparece al confirmar intención horizontal.
- Tareas y equipaje: derecha para completar o dejar pendiente; izquierda para eliminar con confirmación.
- Actividades, gastos y enlaces: izquierda para eliminar con confirmación.
- Encuestas: sin gesto destructivo.
- Se conservan la interfaz móvil estable, Sandra 🦖, los repartos avanzados y los iconos automáticos de enlaces.

## Validación

El preview debe superar antes de quedar `READY`:

- `VJT_SWIPE_TEXT_ONLY_OK`
- `VJT_UI_TESTS_OK`
- `VJT_API_TESTS_OK`
- `VJT_CLEAN_RELEASE_OK`

Preview validado: `dpl_ckYkyw2bgSeWxWV7BQB8fUbbhWzX`.
