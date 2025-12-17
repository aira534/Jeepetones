# Jeepetones
## Ainara Yu Elio San Martín y Rui Bartolomé Segura
Robot siguelineas con comunicación IoT a través de MQTT.

## 1.Organización del código
### Arduino
El arduino se encarga de seguir la linea, comprobar la distancia a objetos y de generar los mensajes que luego se enviarán por comunicación en serie a la placa ESP32 para que ésta los envíe al servidor mqtt. <br>
Para esto disponemos de tres tareas controladas por RTOS de arduino, éstas son:<br>
- Tarea para el ultrasonido, la cual se encarga de medir la distancia al objeto y de detectar cuando el robot está más cerca al objeto de la distancia estipulada.
- Tarea dedicada a la detección de los infrarrojos y a la regulación de la velocidad por un PD.
- Tarea que envía los PINGs requeridos cada 4 segundos.
