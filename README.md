*Este proyecto ha sido creado como parte del currículo de 42 por [jaatif-a]*

# Codexion

> Domina la carrera por los recursos antes de que la fecha límite te domine a ti.

---

## Descripción

**Codexion** es una simulación con hilos y mutex de un problema clásico de **exclusión mutua y reparto de recursos**: la variante "cenando filósofos" de toda la vida, disfrazada de oficina de coders.

N personas programan en un hub circular. Hay N dongles. Cada persona necesita **sus dos dongles vecinos a la vez** (izquierdo y derecho) para compilar. Si una persona no empieza a compilar antes de `time_to_burnout` ms desde su última compilación (o desde el inicio de la simulación), se quema (*burnout*). El objetivo del proyecto es implementar la simulación de forma que esto **nunca ocurra**, bajo un scheduler configurable (`fifo` o `edf`), evitando al mismo tiempo interbloqueos, inanición, carreras de datos, fugas de memoria y comportamiento indefinido.

Traducción directa del enunciado a la metáfora de "los filósofos": `coder` = filósofo, `dongle` = tenedor, `compilar` = comer, `depurar` + `refactorizar` = pensar, `quemarse` = morir.

### Mapa de archivos

| Archivo | Responsabilidad |
|---|---|
| `codexion.h` | Tipos, structs, prototipos. Sin variables globales. |
| `parse_args.c` | Parseo y validación estricta de los 8 argumentos. |
| `init_codexion.c` / `init_dongles.c` / `init_coders.c` / `queue_init.c` | Construcción de toda la estructura, con limpieza segura ante fallo parcial. |
| `destroy.c` | Liberación de todo lo (de verdad) inicializado. |
| `dongle_helpers.c` | Cálculo de prioridad, cooldown, cabeza de cola, timeout. |
| `dongle_acquire.c` | El bucle de "intenta tomar los dos dongles". |
| `dongle_take.c` / `dongle_release.c` | API pública: `take_dongles` / `release_dongles`. |
| `heap_compare.c` / `heap_insert.c` / `heap_remove.c` | Cola de prioridad (binary heap) implementada a mano. |
| `coder_actions.c` / `coder_routine.c` | Ciclo de vida de un coder (hilo). |
| `monitor.c` / `stop_check.c` | Hilo vigilante de burnout + condición de parada compartida. |
| `print_logs.c` | Logging serializado. |
| `threads.c` / `main.c` | Arranque y cierre ordenado de todos los hilos. |

### La topología: un anillo de dongles

Con `n` coders hay `n` dongles. El dongle `i` es el **derecho** del coder `i-1` y el **izquierdo** del coder `i` (módulo `n`):

```
            dongle0
      coder0 ──────── coder1
        │                │
    dongle3            dongle1
        │                │
      coder3 ──────── coder2
            dongle2
```

```c
coder->left  = &codex->dongles[i];
coder->right = &codex->dongles[(i + 1) % n];
```

Cada dongle tiene **como máximo dos pretendientes posibles**: sus dos coders vecinos. Esto es clave para razonar sobre corrección más adelante.

Caso especial `n == 1`: el único coder tiene el mismo dongle a izquierda y derecha, pero **nunca puede formar un par de dos dongles distintos**, así que jamás podrá compilar. `coder_routine` detecta este caso *antes* de llamar a nada de la lógica de dongles y simplemente deja que el monitor lo declare quemado en su momento — es el comportamiento esperado.

### La cola de prioridad (heap)

El enunciado pide implementar una cola de prioridad a mano (no hay ninguna en la librería estándar de C). Es un **binary min-heap** clásico, guardado en un array:

```c
typedef struct s_qnode { int coder_id; t_ms priority; } t_qnode;
typedef struct s_queue { t_qnode *nodes; int size; int capacity; pthread_mutex_t heap_lock; } t_queue;
```

- `get_prio(a, b)` compara solo `priority`; no necesita saber si el scheduler es FIFO o EDF, porque `calc_priority` ya tomó esa decisión antes de insertar. Esto desacopla el heap (estructura de datos genérica) de la política de planificación.
- `heapify_up` / `heapify_down`: las dos operaciones estándar de reordenamiento, separadas en `heap_insert.c` y `heap_remove.c`.
- `remove_from_queue` (cuando un coder se rinde porque la simulación paró) llama a **`heapify_up` y `heapify_down`**, porque el elemento movido al hueco dejado por el eliminado puede romper el heap *en cualquiera de las dos direcciones*.

¿Por qué un heap y no, por ejemplo, recorrer un array buscando el mínimo? Por exigencia explícita del enunciado, pero también porque `insert`/`pop` son O(log n) en vez de O(n). En la práctica aquí `n` es como máximo 2 (cada dongle solo tiene dos pretendientes posibles), así que el rendimiento puro es casi irrelevante — pero la estructura es correcta y general, no un atajo que solo funciona porque "aquí siempre hay 2 elementos".

---

## Instrucciones

### Compilación

```sh
make
```

Reglas disponibles: `all`, `clean`, `fclean`, `re`. Compilado con `-Wall -Wextra -Werror -pthread`, sin *relink*.

### Ejecución

```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
            time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

Ejemplo:

```sh
./codexion 5 800 200 100 100 5 50 edf
```

| Argumento | Significado |
|---|---|
| `number_of_coders` | Número de coders (= número de dongles). |
| `time_to_burnout` | ms desde la última compilación (o el inicio) tras los que alguien se quema. |
| `time_to_compile` | ms que dura compilar (con los dos dongles en mano). |
| `time_to_debug` | ms que dura depurar. |
| `time_to_refactor` | ms que dura refactorizar. |
| `number_of_compiles_required` | Si todos llegan a este número de compilaciones, la simulación para con éxito. |
| `dongle_cooldown` | ms de enfriamiento de un dongle tras soltarse. |
| `scheduler` | `fifo` o `edf`. |

Los argumentos se validan estrictamente (enteros positivos, sin overflow, `scheduler` restringido a `fifo`/`edf`); cualquier valor inválido se rechaza antes de arrancar ningún hilo.

### Verificación realizada

- **ThreadSanitizer**: FIFO, EDF, caso `n=1`, *stress* de 12 coders → 0 carreras detectadas.
- **Valgrind** `--leak-check=full`: terminación normal, quemado con 1 coder, quemado con varios coders, argumentos inválidos, y fallo real de `pthread_create` forzado con `ulimit -u 25` bajo un usuario sin privilegios → 0 *leaks*, 0 errores en todos los casos.
- **Precisión del log de quemado**: el timestamp logueado coincide con `time_to_burnout` con un margen real de 0–1ms, muy por debajo del límite de 10ms exigido.
- **Paralelismo bajo EDF**: confirmado en logs que hasta `n/2` coders compilan en el mismo milisegundo al arrancar, en vez de una cascada secuencial.

---

## Blocking cases handled

### Interbloqueo (deadlock) / condiciones de Coffman

Se rompen **dos** de las cuatro condiciones de Coffman, de forma independiente y redundante (defensa en profundidad):

1. **Se elimina "hold and wait" a nivel de recurso.** Cada coder registra su petición en las dos colas a la vez y solo "gana" cuando puede tomar **ambos dongles en una única operación atómica** (bajo los dos mutex bloqueados simultáneamente). Nunca existe un estado intermedio en el que un hilo tenga un solo dongle en la mano esperando indefinidamente el otro: o tiene los dos, o no tiene ninguno.

   ```c
   int take_dongles(t_coder *coder)
   {
       sort_dongles(coder, &first, &second);
       register_request(first, second, coder, calc_priority(coder));
       ok = acquire_loop(first, second, coder);
       ...
   }
   ```

2. **Orden de candados global y consistente ("espera circular" imposible).** Además, para los dos mutex de sistema que hay que bloquear a la vez, todo el código bloquea siempre el dongle de menor `id` primero, sin excepción, mediante una única función usada tanto al tomar como al soltar:

   ```c
   void sort_dongles(t_coder *coder, t_dongle **first, t_dongle **second)
   {
       if (coder->left->id < coder->right->id)
       { *first = coder->left; *second = coder->right; }
       else
       { *first = coder->right; *second = coder->left; }
   }
   ```

   Como el orden de bloqueo es una función total sobre los dongles (por `id`), nunca puede haber dos hilos esperándose en círculo. `take_dongles`, `release_dongles` y `cancel_request` pasan todos por `sort_dongles`.

### Inanición (starvation)

La prioridad de cada petición se calcula así:

```c
t_ms calc_priority(t_coder *coder)
{
    t_ms base_prio;
    if (coder->codex->args.scheduler == FIFO)
        base_prio = get_time_ms();
    else
        base_prio = coder->last_compile + coder->codex->args.time_to_burnout;
    return (base_prio - coder->id);
}
```

- En **FIFO**, la prioridad es el instante de la petición: el heap (min-heap) siempre concede el recurso a la solicitud más antigua.
- En **EDF**, la prioridad es el *deadline* real (`last_compile + time_to_burnout`): quien esté más cerca de quemarse recibe el dongle antes, implementando Earliest Deadline First.
- El identificador del coder se usa como **desempate** cuando dos peticiones comparten exactamente la misma prioridad (frecuente al arrancar la simulación), garantizando un orden totalmente determinista y evitando que dos empates recurrentes favorezcan siempre al mismo coder.

Probado con `n = 2, 7, 8, 15` bajo EDF: ningún coder se quema en docenas de ejecuciones.

### Gestión del cooldown

Tras soltar un dongle, nadie puede volver a tomarlo hasta que pase `dongle_cooldown` ms. `pthread_cond_timedwait` necesita un timeout explícito porque nadie avisa cuándo termina un cooldown (el aviso solo se dispara al *soltar* el dongle):

```c
t_timespec calc_timeout(t_dongle *dongle, t_codexion *codex)
{
    if (dongle->last_use_time != 0)
        wait_until_ms = dongle->last_use_time + codex->args.dongle_cooldown;
    else
    {
        fallback = codex->args.time_to_compile + codex->args.dongle_cooldown;
        wait_until_ms = get_time_ms() + fallback + 50;
    }
    ...
}
```

Si el dongle ya se soltó alguna vez, el timeout es exactamente "cuándo termina su cooldown". Si nunca se ha soltado (lo tiene otro coder ahora mismo por primera vez), no hay cooldown corriendo — el motivo real de espera es "está ocupado", y de eso sí avisa `pthread_cond_broadcast` en cuanto se libera. Un timeout de respaldo generoso (`time_to_compile + cooldown`) evita convertir la espera en un bucle de lock/unlock a toda velocidad (*busy-waiting* disfrazado) mientras confía en que el `broadcast` despierte al hilo mucho antes en la práctica.

### Detección precisa del agotamiento (burnout)

Un hilo monitor separado sondea a todos los coders cada 1ms (`monitor.c`) y compara el tiempo transcurrido desde la última compilación con `time_to_burnout`, protegido por el mutex de estado de cada coder. Medido en la práctica: desviación de 0–1ms respecto al momento real de quemado, muy por debajo del límite de 10ms exigido por el enunciado.

### Serialización del log

Todos los mensajes de log pasan por un único `write_mutex` global (`print_logs.c`), garantizando que dos hilos nunca entrelacen su salida y que cada línea de log se imprime de forma atómica y ordenada.

### Fugas de memoria y estados parcialmente inicializados

Contadores `n_dongles_init` / `n_coders_init` solo avanzan cuando un elemento quedó *totalmente* inicializado; si algo falla a mitad, cada función de init se limpia a sí misma antes de devolver error. `destroy_codexion` nunca llama a `pthread_mutex_destroy` sobre un mutex que no llegó a inicializarse. `launch_threads` hace `join` de los hilos ya creados *antes* de destruir nada si `pthread_create` falla a mitad de la creación.

---

## Thread synchronization mechanisms

| Primitiva | Instancias | Qué protege |
|---|---|---|
| `pthread_mutex_t dongle->mutex` | Una por dongle | `available`, `last_use_time`; punto de sincronización de `pthread_cond_*` para ese dongle. |
| `pthread_cond_t dongle->cond` | Una por dongle | Despierta a los hilos que esperan ese dongle concreto (al soltarlo, `pthread_cond_broadcast`). |
| `pthread_mutex_t` (estado del coder) | Uno por coder | Contadores de compilación, `last_compile`, y cualquier dato leído por el monitor. |
| `pthread_mutex_t write_mutex` | Global (uno) | Serializa todas las escrituras de log (`print_logs.c`). |
| `pthread_mutex_t stop_lock` | Global (uno) | Protege la bandera compartida de parada de la simulación. |
| `pthread_mutex_t heap_lock` | Uno por cola (dos por dongle: izquierda/derecha) | Protege el array interno del heap (`nodes`, `size`). |

### Coordinación de acceso a los dongles

`take_dongles` bloquea **los dos mutex de dongle** (en el orden fijo dado por `sort_dongles`, ver "Blocking cases handled") y solo entonces comprueba-y-toma de forma atómica:

```c
pthread_mutex_lock(&first->mutex);
pthread_mutex_lock(&second->mutex);
if (try_take_both(first, second, coder))
{
    pthread_mutex_unlock(&second->mutex);
    pthread_mutex_unlock(&first->mutex);
    return (1);
}
```

Como la comprobación (`is_queue_head` + `available` + `cooldown_passed`) y la toma ocurren bajo los mismos dos mutex, ningún otro hilo puede "colarse" entre que se comprueba que un dongle está libre y que se marca como ocupado — así se elimina la carrera de datos clásica *check-then-act*.

Si no se pueden tomar ambos, el hilo **no hace busy-waiting puro**: identifica cuál de los dos dongles es el cuello de botella y se duerme en su variable de condición con un timeout, soltando los mutex mientras espera:

```c
if (dongle_ready(first, coder->id, cooldown))
    wait_turn(first, second, coder);   // first ya está listo, el problema es second
else
    wait_turn(second, first, coder);   // first es el bloqueante
```

Al soltar (`release_dongles`), cada dongle se libera en su **propia** sección crítica independiente (nunca reteniendo los dos mutex a la vez), y se hace `pthread_cond_broadcast` sobre su condición para despertar a cualquier coder que esperara justo ese dongle.

### Comunicación thread-safe entre coders y el monitor

El monitor no comparte estado mutable directo con los coders salvo a través de mutex: lee `last_compile` y los contadores de cada coder bajo el mutex de estado de ese coder, nunca accede a memoria sin bloquear primero. Cuando detecta un burnout, o cuando un coder llega a `number_of_compiles_required`, ambos caminos llaman a `set_stopped_true`, que:

1. Bloquea `stop_lock` y marca la bandera compartida de parada.
2. Hace `pthread_cond_broadcast` sobre **todos** los dongles, para despertar inmediatamente a cualquier coder que estuviera dormido esperando un timeout largo, en vez de dejarlo esperar hasta que expire por sí solo.

Esto evita tanto una carrera de datos sobre la bandera de parada (protegida por `stop_lock`) como una terminación lenta o colgada (gracias al `broadcast` explícito).

### Doble candado en el heap: `heap_lock` vs `dongle->mutex`

`heap_lock` protege solo la estructura interna del heap; `dongle->mutex` es el candado "de alto nivel" que decide quién puede tocar ese dongle ahora. En el flujo actual, toda llamada a `insert`/`pop`/`remove_from_queue` ocurre ya dentro de una sección protegida por `dongle->mutex`, así que `heap_lock` es hoy redundante en la práctica — pero se deja como defensa en profundidad: si en el futuro se añade una ruta de código que toque la cola sin pasar por `dongle->mutex`, el heap seguiría siendo seguro por sí mismo en vez de corromperse silenciosamente.

---

## Recursos

### Referencias clásicas

- E. W. Dijkstra, *Hierarchical ordering of sequential processes* (1971) — origen del problema de los filósofos comensales.
- A. Silberschatz, P. Galvin, G. Gagne, *Operating System Concepts* — capítulos de sincronización de procesos, exclusión mutua y condiciones de Coffman.
- Documentación oficial de POSIX Threads: `man pthread_mutex_lock`, `man pthread_cond_timedwait`, `man pthread_create`.
- Documentación de [ThreadSanitizer](https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual) y de [Valgrind Memcheck](https://valgrind.org/docs/manual/mc-manual.html), usadas para la verificación descrita en "Instrucciones".
- Material clásico sobre *Earliest Deadline First scheduling* en sistemas de tiempo real, aplicado aquí como política de asignación de recursos en vez de planificación de CPU.

### Uso de IA

Se utilizó un asistente de IA (Claude, de Anthropic) como apoyo puntual durante el desarrollo, concretamente para:

- Revisar la redacción y estructura de este README.md conforme a los requisitos del subject.
- Discutir y verificar el razonamiento sobre la ausencia de deadlock (condiciones de Coffman) y de inanición, comparando el diseño propuesto con la teoría clásica.
- Sugerir escenarios de prueba adicionales para ThreadSanitizer y Valgrind (por ejemplo, forzar el fallo de `pthread_create` con `ulimit -u`).

Todo el código fuente (`.c`/`.h`) fue diseñado, escrito y depurado por el autor; la IA no generó lógica de sincronización ni código de producción del proyecto.
