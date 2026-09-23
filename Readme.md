# Nombre del proyecto

Sensor DHT22 + Make + Telegram — Alerta de temperatura

## Descripción

Sistema que detecta cuando la temperatura de un espacio sube por encima
de un umbral definido y notifica automáticamente por Telegram,
sugiriendo encender el aire acondicionado. Un Arduino UNO R4 WiFi con un
sensor DHT22 envía el dato de temperatura a un escenario de Make.com
mediante un webhook, que ejecuta el envío del mensaje de alerta.

## Objetivos de aprendizaje

Integrar un sensor físico (DHT22), una placa con conectividad WiFi
(Arduino UNO R4) y una plataforma de automatización en la nube
(Make.com) para construir un sistema de alerta funcional de punta a
punta, incluyendo un servidor web local de monitoreo servido desde el
propio Arduino.

## Material utilizado

- Arduino UNO R4 WiFi
- Sensor DHT22
- Protoboard
- LED + resistencia
- Cables Dupont
- Cuenta de Make.com (zona us2.make.com)
- Bot de Telegram

## Diagrama del circuito

[Diagrama/diagrama_circuito_dht22.png](Diagrama/diagrama_circuito_dht22.png)

## Código

- [Codigo/sensor_dht22_alerta.ino](Codigo/sensor_dht22_alerta.ino) — sketch del Arduino (usa `secrets.h` local, no incluido en el repo; ver [Codigo/secrets.h.example](Codigo/secrets.h.example))
- [Codigo/make_scenario_leer_dth22.json](Codigo/make_scenario_leer_dth22.json) — blueprint del escenario de Make.com

## Video del funcionamiento

[Ver video en YouTube](https://youtu.be/r1upw_lRkkc) — "Práctica Sensor DHT22 / Arduino" (2:28)

## Evidencias de armado

- [Resultados/circuito_armado_1.jpeg](Resultados/circuito_armado_1.jpeg)
- [Resultados/circuito_armado_2.jpeg](Resultados/circuito_armado_2.jpeg)

## Terminal

[Terminal/monitor_serie_arduino.png](Terminal/monitor_serie_arduino.png) —
Monitor Serie del Arduino IDE mostrando las lecturas de temperatura y la
respuesta exitosa de Make (`HTTP/1.1 200 OK`).

## Reporte

[Reporte/Reporte_Sensor_DTH22_Alerta.md](Reporte/Reporte_Sensor_DTH22_Alerta.md)

Incluye:

- Objetivos y material
- Detalle del escenario de Make y del código del Arduino
- Resultados y observaciones
- Conclusiones

## Conclusiones

La práctica permitió integrar un sensor físico, una placa con
conectividad WiFi y una plataforma de automatización en la nube para
construir un sistema de alerta funcional de punta a punta: desde la
lectura del sensor hasta la notificación al usuario por Telegram, sin
intervención manual.
