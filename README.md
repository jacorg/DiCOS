# DiC_OS
Sistema Operativo desarrollado para la materia "Implementacion de sistemas operativos I"
Este sistema operativo tiene fines solamente didacticos, no es recomendable para utilizarlo en produccion de dispositivos y o productos.
Se tomo como referencia el sistema operativo desarrollado por Mg. Ing. Gonzalo Sanchez (https://github.com/gonzaloesanchez/MSE_OS)


## Examen ISO I

### Recomendaciones
Resuelva el siguiente ejercicio en la PC con la EDU-CIAA utilizando el sistema operativo que
implementó durante la cursada. Entregue el repositorio con el código de la aplicación (resolución
del presente enunciado) junto con su sistema operativo y envíe el URL al docente. Incluya
cualquier instructivo que considere necesario para que el docente pueda compilar y probar su
sistema operativo y su resolución. Se requiere generar un ​ tag en el commit con la versión de
código que deba corregirse, usando el nombre “examen”.

### Enunciado
Implemente un sistema que mida la diferencia de tiempos entre flancos positivos y negativos
generados por dos pulsos, cuyas ocurrencias se solapen temporalmente, según se muestra en la
figura. Cada caso de solapamiento tendrá un led específico asociado, el cual se encenderá
inmediatamente luego de que los dos botones dejen de ser presionados. El tiempo en que el led
correspondiente estará encendido será la suma de los tiempos entre flancos ascendentes y
descendentes respectivamente.

### Implementación
La implementación se realizo mediante la creacion de tres tareas. Cada tarea asociada a una operación 
del manejo de teclas TEC1 y TEC2.Las teclas se manejan por interrupciones.
 La tercer tarea se encarga de calculos de tiempos, encendido de LEDs y envio de mensajes a la UART. 
Se utilizaron recursos del OS que facilitaron la implementación, como delays y semaforos binarios para sincronización.
Se dejo de lado el uso de colas por a consideración del desarroliista.



## Comentarios y problemas sobre el desarrollo del OS durante la materia ISO I.

-.Se implemento full stacking de todos los registros y tambien de registros de FPU.
Implementación tareas BLOCKED y RUNNING.
Implementación 8 tareas que tiene el OS.
Implementación de la función createTask se asigna prioridad a la tareas entre 0 (max) y 3(mín).

-.Se necesita implementar el sistema de prioridades para atención de tareas.

-.El diagrama de scheduling se hizo en base a prioridades dentro de un array bidimensional
donde las filas son las prioridades de las tareas y las columnas cada tarea asignada a un determinado nivel.
El uso de memoria es mayor pero una vez ordenado, no se modifican las tareas ni existe reordenamiento.

-.Se implemento diagrama de scheduling basado en asignacion de prioridades, se implemento
función delay dentro de la API. Falta documentar las funciones. Se comienza con la implementación
de los semaforos y colas

-.Se implementaron las funciones de API para menejo de colas y semaforos.
-.Se implemento las funciones de llamado a interrupciones.
-.Falta validad con diferentes tareas el correcto funcionamiento de las colas.
-.Se corrigió un error en el scheduling que provocaba un error en el cambio de contexto
  cuando se utilizaban varios delays en tareas diferentes (todas las tareas del usuario se bloqueaban y 
  no se producía el cambio de contecto a la tarea Idle).

-.Subo parte de la resolución del parcial. Falta solamente el envio de los datos por medio de la UART
 todos los otros requerimientos estan funcionando correctamente.


