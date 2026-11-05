# Contribuir a POSIM

Puedes mejorar tanto el código como la documentación. Empieza por un cambio concreto y reproducible, y mantén coherentes los modelos, los controladores y las guías.

## Trabajar en una rama

{{CODE:contributing:0}}

Para cambios de simulación, utiliza el [entorno de instalación](requirements.md) adecuado. No incluyas directorios de compilación o instalación generados ni credenciales locales en el commit.

## Comprobar el comportamiento modificado

Elige un ejemplo mínimo relevante. Verifica la resolución de recursos, la aparición del modelo, el avance del tiempo, los datos recibidos, el controlador si corresponde y el cierre normal. Registra revisión, renderizador, arquitectura y comando.

Los cambios de sensores o dinámica requieren una comparación numérica apropiada. Una captura muestra el aspecto, pero no demuestra la precisión física. Conserva la atribución original al modificar código heredado.

## Actualizar las ocho versiones de la documentación

El sitio admite inglés, coreano, español, francés, alemán, italiano, japonés y chino. El inglés usa `/`; los demás idiomas, `/ko/`, `/es/`, `/fr/`, `/de/`, `/it/`, `/ja/` y `/zh/`.

Actualiza los archivos correspondientes en `website/content/<idioma>`. Mantén iguales comandos, unidades, requisitos y límites, pero redacta con naturalidad y terminología habitual de robótica. Los marcadores compartidos `CODE` y `MEDIA` permiten reutilizar ejemplos ejecutables e imágenes. Cada idioma necesita todas las páginas; no se sustituye un texto ausente por inglés.

Al cambiar una función pública:

- Actualiza el [catálogo de funciones](features.md) y la guía de uso junto con el código.
- Explica instalación y activación, valores predeterminados, unidades, entradas y salidas, alcance de la configuración, un ejemplo observable y límites del modelo. Incluye efectos opcionales y adaptadores.
- Registra las páginas nuevas en `website/scripts/pages.mjs` y sus títulos en `website/i18n/<idioma>.json`. Mantén también los README de cada idioma.

El orden de páginas es común; los catálogos de mundos y objetos se generan desde el código. Documenta el producto actual. Guarda notas internas, planes e informes de validación fuera del repositorio; usa issues y pull requests para discutir cambios y resultados.

## Revisar el sitio

Desde `website`, con Node.js 24 y pnpm 10.11.0:

{{CODE:contributing:1}}

Abre `http://127.0.0.1:4174`. Comprueba enlaces, copia de comandos, búsqueda, cambio de idioma y navegación móvil. La publicación y el mantenimiento se explican en [website/README.md](https://github.com/IOES-Lab/POSIM/blob/main/website/README.md).

## Abrir una pull request

Explica el problema, el comportamiento resultante y las comprobaciones realizadas. Incluye comandos y registros pertinentes para cambios de simulación, y vistas de escritorio y móvil para cambios de diseño. Indica qué verificaste y qué queda sin comprobar.

Usa [GitHub issues](https://github.com/IOES-Lab/POSIM/issues) para errores reproducibles y [pull requests](https://github.com/IOES-Lab/POSIM/pulls) para revisar cambios. IOES-Lab mantiene el proyecto en [Korea Maritime & Ocean University](https://lab.wschoi.com).
