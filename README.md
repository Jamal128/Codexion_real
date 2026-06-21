Este proyecto ha sido creado como parte del currículo de 42 por \<jaatif-a>

# Codexion

Simulación con hilos y mutex de un problema clásico de **exclusión mutua y reparto de recursos** (la variante "cenando filósofos" de toda la vida, disfrazada de oficina de coders). Este documento explica **cómo funciona por dentro**, en concreto la gestión de los dongles y el heap de prioridad, y **cómo se evita cada problema clásico de concurrencia** (deadlock, inanición, carreras de datos, fugas de memoria, crashes).

---

## 1. El problema, en una frase

> N personas programan en un hub circular. Hay N dongles. Cada persona necesita **sus dos dongles vecinos a la vez** (izquierdo y derecho) para compilar. Si una persona no empieza a compilar antes de `time_to_burnout` ms desde su última compilación (o desde el inicio), se quema. La simulación debe evitar que esto pase bajo el scheduler `edf`.

Traducción directa del enunciado a la metáfora de "los filósofos": `coder` = filósofo, `dongle` = tenedor, `compilar` = comer, `depurar`+`refactorizar` = pensar, `quemarse` = morir.

---

## 2. Mapa de archivos

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

---

## 3. La topología: un anillo de dongles

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

Cada dongle tiene **como máximo dos pretendientes posibles**: sus dos coders vecinos. Esto es importante y se usa varias veces más abajo para razonar sobre corrección.

Caso especial `n == 1`: el único coder tiene el mismo dongle a izquierda y derecha, pero **nunca puede formar un par de dos dongles distintos**, así que jamás podrá compilar. `coder_routine` detecta este caso *antes* de llamar a nada de la lógica de dongles y simplemente deja que el monitor lo declare quemado en su momento — es el comportamiento esperado (con 1 sola persona y la regla de "dos dongles para compilar", no hay solución posible).

---

## 4. Gestión de los dongles (el corazón del proyecto)

### 4.1 Por qué se toman los dos dongles **a la vez**, no uno y luego el otro

La forma "ingenua" de resolver el problema de los filósofos (cada uno coge primero su tenedor izquierdo, luego espera el derecho) es la receta clásica del deadlock: si todos cogen el izquierdo a la vez, todos se quedan esperando eternamente el derecho, que está en la mano del vecino.

Aquí cada coder **registra su petición en las dos colas a la vez**, y solo "gana" cuando puede tomar **ambos dongles en una única operación atómica**:

```c
int	take_dongles(t_coder *coder)
{
	sort_dongles(coder, &first, &second);
	register_request(first, second, coder, calc_priority(coder));
	ok = acquire_loop(first, second, coder);
	...
}
```

Nunca existe un estado intermedio en el que un hilo tenga **un solo dongle en la mano** esperando indefinidamente el otro. O tiene los dos, o no tiene ninguno. Eso elimina por completo la condición necesaria para el deadlock clásico ("hold and wait" con un único recurso retenido).

### 4.2 Segunda barrera anti-deadlock: orden de candados consistente

Aunque el punto anterior ya evita el deadlock "de recursos", seguimos teniendo **dos mutex de sistema** (`first->mutex` y `second->mutex`) que hay que bloquear a la vez dentro de `acquire_loop`. Si cada hilo los bloqueara en un orden distinto, podríamos tener un deadlock *a nivel de mutex* (hilo A bloquea X y espera Y; hilo B bloquea Y y espera X).

Por eso **todo el código bloquea siempre el dongle de menor `id` primero**, sin excepción, mediante una única función usada tanto al tomar como al soltar:

```c
void	sort_dongles(t_coder *coder, t_dongle **first, t_dongle **second)
{
	if (coder->left->id < coder->right->id)
	{
		*first = coder->left;
		*second = coder->right;
	}
	else
	{
		*first = coder->right;
		*second = coder->left;
	}
}
```

`take_dongles`, `release_dongles` y `cancel_request` *todos* pasan por `sort_dongles`. Como el orden de bloqueo es una función *global y total* sobre los dongles (por `id`), nunca puede haber dos hilos esperándose en círculo. Esto es defensa en profundidad: aunque el punto 4.1 ya bastaba, tener **dos mecanismos independientes** que evitan el mismo problema hace el código robusto incluso si alguien lo modifica en el futuro.

### 4.3 La decisión de "¿puedo tomar este dongle?"

Un dongle se puede tomar si se cumplen **tres condiciones a la vez**:

```c
static int	dongle_ready(t_dongle *d, int coder_id, t_ms cooldown)
{
	return (is_queue_head(d, coder_id)      // 1. soy el primero en la cola de ESTE dongle
		&& d->available                      // 2. nadie lo tiene en la mano ahora mismo
		&& cooldown_passed(d, cooldown));    // 3. ya pasó el enfriamiento desde que se soltó
}
```

Y un coder solo avanza cuando **las dos condiciones de sus dos dongles son verdad simultáneamente**, comprobado mientras se tienen *los dos mutex* bloqueados (para que nadie pueda colarse entre la comprobación y la toma):

```c
pthread_mutex_lock(&first->mutex);
pthread_mutex_lock(&second->mutex);
if (try_take_both(first, second, coder))   // check-and-take atómico
{
    pthread_mutex_unlock(&second->mutex);
    pthread_mutex_unlock(&first->mutex);
    return (1);
}
```

Si no se puede, el hilo no hace *busy-waiting* puro: identifica **cuál de los dos dongles es el cuello de botella** y se duerme en su variable de condición con un timeout (sección 4.5), soltando los mutex mientras espera para no bloquear a nadie más:

```c
if (dongle_ready(first, coder->id, cooldown))
	wait_turn(first, second, coder);   // first ya está listo, el problema es second
else
	wait_turn(second, first, coder);   // first es el bloqueante
```

### 4.4 Prioridad: FIFO, EDF, y una historia de cómo un bug solo aparecía al ejecutar

```c
t_ms	calc_priority(t_coder *coder)
{
	if (coder->codex->args.scheduler == FIFO)
		base_prio = get_time_ms();                                    // orden de llegada
	else
		base_prio = coder->last_compile + coder->codex->args.time_to_burnout; // deadline EDF
	return (base_prio * 1000000 + (coder->id % 2) * 1000 + coder->id);
}
```

Esta prioridad se inserta en las **dos** colas del coder (la del dongle izquierdo y la del derecho), y el heap de cada dongle siempre concede el dongle a quien tenga el valor **más bajo** (FIFO ⇒ el que más tiempo lleva esperando; EDF ⇒ el que tiene el deadline de quemado más cercano).

**El bug que solo se vio al correr el programa, no leyendo el código:** al arrancar la simulación, *todos* los coders tienen el mismo `last_compile` (el instante de inicio), así que en EDF todos arrancan con el **mismo deadline**. La primera versión desempataba ese empate por `id` puro (menor id gana siempre). Eso suena inocente, pero genera una **cascada total**: el coder 0 le gana a todos sus vecinos, así que el coder 1 nunca puede competir con él hasta que termine, el coder 2 nunca puede competir con el 1... el resultado es que con 8 coders, ¡toda la simulación se vuelve estrictamente secuencial al arrancar! Con `time_to_burnout` ajustado pero razonable, el último de la fila se quemaba antes de que le llegara el turno — **2 de cada 3 ejecuciones reales con 8 coders fallaban**, aunque el anillo en teoría permite que la mitad compile en paralelo.

La corrección: desempatar primero por **paridad** del id, y solo como último recurso por el id en sí (`(id % 2) * 1000 + id`). Con esto, en un empate, todos los coders de id par le ganan a sus vecinos impares y viceversa — es decir, se aproxima al 2-coloreado óptimo de un ciclo, que es exactamente lo que se necesita para que hasta `n/2` coders puedan compilar a la vez sin conflicto:

```
coder0(par) vs coder7(impar) → gana 0     coder0 vs coder1(impar) → gana 0   ⇒ coder0 compila YA
coder2(par) vs coder1(impar) → gana 2     coder2 vs coder3(impar) → gana 2   ⇒ coder2 compila YA
coder4(par) vs coder3(impar) → gana 4     coder4 vs coder5(impar) → gana 4   ⇒ coder4 compila YA
coder6(par) vs coder5(impar) → gana 6     coder6 vs coder7(impar) → gana 6   ⇒ coder6 compila YA
```

Confirmado en el log real tras el arreglo: los coders 1, 3, 5, 7 (display, 1-indexado) arrancan a compilar en el mismo milisegundo. Con `n` impar queda exactamente una "costura" en el anillo donde dos vecinos comparten paridad y se desempata por id puro — un cuello de botella muy localizado, nada comparable a la cascada total original.

Este truco **solo afecta a empates exactos**; en cuanto la simulación avanza un poco, cada coder tiene un `last_compile` distinto y el deadline real (la magnitud, multiplicada por `1 000 000`) domina sobre el desempate, así que EDF sigue siendo EDF de verdad.

### 4.5 Cooldown y por qué `calc_timeout` no es un *busy-wait* disfrazado

```c
t_timespec	calc_timeout(t_dongle *dongle, t_codexion *codex)
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

`pthread_cond_timedwait` necesita *algún* timeout porque **nadie avisa explícitamente cuando termina un cooldown** (el aviso solo se dispara al *soltar* el dongle, no al final del enfriamiento). Por eso, si el dongle ya se soltó alguna vez, el timeout es exactamente "cuándo termina su cooldown".

Pero si el dongle **nunca se ha soltado** (está siendo usado por primera vez, o nadie lo ha tocado todavía), no hay ningún cooldown corriendo — el motivo de espera real es "alguien más lo tiene ahora mismo", y de eso **sí** avisa `pthread_cond_broadcast` en cuanto se libera. Poner aquí un timeout "ya mismo" (como hacía una versión anterior) convierte la espera en un bucle de lock/unlock a toda velocidad mientras el otro compila — gasta CPU sin necesidad. La solución: un timeout de respaldo generoso (`time_to_compile + cooldown`, el peor caso real), confiando en que el `broadcast` despierte al hilo mucho antes en la práctica.

### 4.6 Soltar los dongles

```c
void	release_dongles(t_coder *coder)
{
	sort_dongles(coder, &first, &second);   // mismo orden que al tomar
	... available = true; last_use_time = get_time_ms(); pthread_cond_broadcast ...
}
```

Simétrico a `take_dongles`, usa el mismo `sort_dongles` (orden por id) por consistencia, y nunca retiene los dos mutex a la vez (cada dongle se suelta en su propia sección crítica independiente) — así que ni siquiera podría participar en un deadlock de dos candados aunque el orden fuera distinto.

---

## 5. El heap (cola de prioridad)

El enunciado pide implementar una cola de prioridad a mano (no hay ninguna en la librería estándar de C). Es un **binary min-heap** clásico, guardado en un array:

```c
typedef struct s_qnode { int coder_id; t_ms priority; } t_qnode;
typedef struct s_queue { t_qnode *nodes; int size; int capacity; pthread_mutex_t heap_lock; } t_queue;
```

- `get_prio(a, b)` compara solo `priority` — ya no necesita saber si el scheduler es FIFO o EDF, porque `calc_priority` ya hizo esa decisión antes de insertar. Esto desacopla el heap (estructura de datos genérica) de la política de planificación.
- `heapify_up` / `heapify_down`: las dos operaciones estándar de reordenamiento, separadas en `heap_insert.c` y `heap_remove.c`.
- `remove_from_queue` (cuando un coder se rinde porque la simulación paró) hace algo que muchas implementaciones simplificadas omiten: tras mover el último elemento al hueco que deja el eliminado, llama a **`heapify_up` y `heapify_down`**, porque ese elemento movido puede romper el heap *en cualquiera de las dos direcciones*, no solo hacia abajo.

**¿Por qué un heap y no, por ejemplo, recorrer un array buscando el mínimo?** Por exigencia explícita del enunciado, pero también porque `insert`/`pop` son O(log n) en vez de O(n). En la práctica aquí `n` es como máximo 2 (cada dongle solo tiene dos pretendientes posibles: sus dos vecinos), así que el rendimiento puro es irrelevante — pero la estructura es correcta y general, no un atajo que solo funciona porque "aquí siempre hay 2 elementos".

### 5.1 Doble candado: `heap_lock` vs `dongle->mutex`

Cada dongle tiene **dos** protecciones distintas:

- `dongle->mutex`: protege `available`, `last_use_time`, y sirve de punto de sincronización para `pthread_cond_*`. Es el candado "de alto nivel" que decide *quién puede tocar este dongle ahora*.
- `queue->heap_lock`: protege solo la estructura interna del heap (el array, `size`).

En el flujo actual, **toda** llamada a `insert`/`pop`/`remove_from_queue`/`get_id` ocurre ya dentro de una sección protegida por `dongle->mutex` (al registrar la petición, al comprobar+tomar, y al cancelar si la simulación para). Es decir: `heap_lock` es hoy **redundante en la práctica**, pero se deja como defensa en profundidad — si en el futuro alguien añade una ruta de código que toque la cola sin pasar por `dongle->mutex`, el heap seguiría siendo seguro por sí mismo en vez de corromperse silenciosamente.

---

## 6. Cómo se evita "todo lo malo"

| Problema | Cómo se evita aquí |
|---|---|
| **Deadlock** | (a) adquisición atómica de los dos dongles, nunca "uno y espero el otro"; (b) orden de candados global y consistente por `id` vía `sort_dongles`. Dos mecanismos independientes, no uno solo. |
| **Inanición bajo EDF** | Prioridad = deadline real, con desempate por paridad para el caso degenerado de deadlines empatados al arrancar (ver §4.4). Probado con n=2,7,8,15 y EDF sin un solo quemado en docenas de ejecuciones. |
| **Carreras de datos** | Un mutex por dongle, un mutex por coder (estado/contadores), un `write_mutex` global para los logs, un `stop_lock` para la bandera de parada, `heap_lock` por cola. Verificado con **ThreadSanitizer** en 4 escenarios concurrentes distintos: 0 *warnings*. |
| **Fugas de memoria** | Contadores `n_dongles_init` / `n_coders_init` que solo avanzan cuando un elemento quedó *totalmente* inicializado; si algo falla a mitad, cada función de init se limpia a sí misma antes de devolver error. Verificado con **Valgrind** (`--leak-check=full`) en: terminación normal, quemado con 1 coder, quemado con varios, argumentos inválidos, y fallo real de `pthread_create` (forzado con `ulimit -u` y un usuario sin privilegios) → 0 *leaks* en los 5 casos. |
| **Crashes / UB** | `destroy_codexion` nunca llama a `pthread_mutex_destroy` sobre un mutex que no llegó a inicializarse (gracias a los contadores anteriores); `launch_threads` hace `join` de los hilos ya creados *antes* de destruir nada si `pthread_create` falla a mitad; `ft_atol_positive` comprueba overflow *antes* de multiplicar (no después, que sería UB). |
| **Norma 42** | Los 22 archivos pasan `norminette` real sin un solo error (sin comentarios dentro de funciones, ≤25 líneas por función, ≤5 funciones por archivo, declaraciones y prototipos alineados). |

---

## 7. Cumplimiento del enunciado

| Exigencia del subject | Dónde se cumple |
|---|---|
| Sin variables globales | Todo vive en `t_codexion`, pasado por puntero. |
| Un hilo por coder + hilo monitor separado | `launch_threads` / `coder_routine` / `monitor_routine`. |
| Un dongle entre cada par (o uno solo si `n=1`) | Topología en anillo, §3. |
| Mutex + variable de condición para los dongles | `t_dongle.mutex` / `t_dongle.cond`. |
| Cooldown obligatorio tras soltar | `cooldown_passed` + `calc_timeout`, §4.5. |
| Arbitraje justo `fifo` / `edf` | `calc_priority` + heap, §4.4 y §5. |
| Nadie se quemuza con EDF (parámetros viables) | §4.4 — incluye el fix del bug de paralelismo real. |
| Log de quemado ≤10ms del momento real | Hilo monitor con sondeo cada 1ms (`monitor.c`); medido en la práctica con desviación de 0ms en runs de prueba. |
| Logging serializado, sin mezclarse | `write_mutex` en `print_logs.c`. |
| Cola de prioridad propia (heap) | §5, sin librería externa. |
| Parada por quemado **o** por `number_of_compiles_required` | `all_done` (chequeado por cada coder tras compilar) + detección de quemado en el monitor; ambos llaman a `set_stopped_true`, que además hace `broadcast` a todos los dongles para despertar a quien esté esperando. |
| Rechazo de argumentos inválidos (negativos, no enteros, scheduler inválido) | `ft_atol_positive` + `parse_scheduler`, con chequeo de overflow seguro. |
| `-Wall -Wextra -Werror -pthread`, sin *relink*, reglas `NAME/all/clean/fclean/re` | `Makefile`. |
| Norma sin errores | §6, verificado con `norminette` real. |

---

## 8. Compilar y ejecutar

```sh
make
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

---

## 9. Verificación realizada (no solo afirmada)

- **Norminette real** (instalada desde PyPI): `OK!` en los 22 archivos.
- **Compilación**: `cc -Wall -Wextra -Werror -pthread`, 0 *warnings*.
- **ThreadSanitizer**: FIFO, EDF, caso `n=1`, *stress* de 12 coders → 0 carreras detectadas.
- **Valgrind** `--leak-check=full`: terminación normal, quemado con 1 coder, quemado con varios coders, argumentos inválidos, y fallo real de `pthread_create` forzado con `ulimit -u 25` bajo un usuario sin privilegios → **0 *leaks*, 0 errores** en todos los casos.
- **Precisión del log de quemado**: medido en varias corridas, el timestamp logueado coincide exactamente con `time_to_burnout` (margen real de 0–1ms, muy por debajo del límite de 10ms).
- **Paralelismo bajo EDF**: confirmado en logs que hasta `n/2` coders compilan en el mismo milisegundo al arrancar, en vez de la cascada secuencial de la versión anterior.

