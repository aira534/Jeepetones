# Jeepetones
## Ainara Yu Elio San Martín y Rui Bartolomé Segura
Robot siguelineas con comunicación IoT a través de MQTT.

## 1.Organización del código
### Arduino
El arduino se encarga de seguir la linea, comprobar la distancia a objetos y de generar los mensajes que luego se enviarán por comunicación en serie a la placa ESP32 para que esta los envíe al servidor mqtt. <br>
Para esto disponemos de tres tareas controladas por RTOS de arduino, estas son:

#### Tarea Ultrasonido
Tarea con nivel 2 de prioridad.<br>
Se encarga de medir la distancia hasta un objeto y detectar cuando el robot se acerca más de lo permitido a él. La distancia incluye un pequeño margen para compensar posibles retrasos en la lectura, la prioridad de otras tareas, el tiempo de frenado del robot y la inercia causada por su velocidad antes de que se ejecute la orden de detener los motores.<br><br>
Además del frenado básico, se incluye un margen de seguridad adicional. Este margen permite que, cuando el robot se acerque al obstáculo pero aún no haya alcanzado el límite establecido, reduzca la velocidad. De esta manera, el frenado completo se realiza de manera más eficiente y segura.
```c
// Variables de la distancia
#define DIST_STOP 10 // With a bit of gap because the intertia of movement
#define DIST_SLOW 15 // Distance where the robot is going to star slowing the speed

// Dentro de la tarea
// Filter wrong measures (0) and check if obstacle is close enough to slow the vel
if (dist <= (DIST_SLOW + DIST_STOP) && dist > 0.1 ) {
  MAXIMUM_SPEED = 100;
  BASE_SPEED = 75;
}
```

Tarea Infrarojos y PD- Tarea dedicada a la detección de los infrarrojos y a la regulación de la velocidad por un PD.

####
- Tarea que envía los PINGs requeridos cada 4 segundos.
