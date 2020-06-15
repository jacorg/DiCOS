# DiC_OS
Sistema Operativo desarrollado para la materia "Implementacion de sistemas operativos I"
Este sistema operativo tiene fines solamente didacticos, no es recomendable para utilizarlo en produccion de dispositivos y o productos.

Se implemento full stacking de todos los registros y tambien de registros de FPU.
Implementación tareas BLOCKED y RUNNING.
Implementación 8 tareas que tiene el OS.
Implementación de la función createTask se asigna prioridad a la tareas entre 0 (max) y 3(mín).

Se necesita implementar el sistema de prioridades para atención de tareas.
-------------------------------------------------------------------------------------------------------
Se implemento diagrama de scheduling basado en asignacion de prioridades, se implemento
función delay dentro de la API. Falta documentar las funciones. Se comienza con la implementación
de los semaforos y colas
-------------------------------------------------------------------------------------------------------
Se implementaron las funciones de API para menejo de colas y semaforos.
Se implemento las funciones de llamado a interrupciones.
Falta validad con diferentes tareas el correcto funcionamiento de las colas.
Se corrigió un error en el scheduling que provocaba un error en el cambio de contexto
cuando se utilizaban varios delays en tareas diferentes (todas las tareas del usuario se bloqueaban y 
no se producía el cambio de contecto a la tarea Idle).
