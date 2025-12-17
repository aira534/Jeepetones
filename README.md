# Jeepetones
## Ainara Yu Elio San Martín y Rui Bartolomé Segura
Robot siguelineas con comunicación IoT a través de MQTT.

## 1.Organización del código
### Arduino
El arduino se encarga de la parte de seguir linea, comprobar distancia a objetos y genera los mensajes que luego se neviarán por comunicación en serie a la placa ESP32 para que esta los envié al servidor mqtt. <br>
Para esto tenemos tres ºº112tareas controladas por RTOS de arduino, estas son:<br>
- Tarea para el ultrasonido, esta tarea se encarga de medir la distancia al objeto y de estar más cerca de la distancia estipulada 
- Segundo elemento
- Tercer elemento
