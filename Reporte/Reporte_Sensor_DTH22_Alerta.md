# Reporte técnico — Sistema de alerta por temperatura (DHT22 + Make + Telegram)

**Materia:** Desarrollo Sustentable con Automatización
**Referencia:** basado en la guía "Escenario Leer DTH11" (Miguel Ángel Barrón
Hernández, Instituto Tecnológico de Mazatlán), adaptada aquí a una
implementación propia con sensor **DHT22**.

## 1. Introducción

Este proyecto implementa un sistema que detecta cuando la temperatura de
un espacio sube por encima de un umbral definido y notifica
automáticamente por Telegram, sugiriendo encender el aire acondicionado.
Un Arduino UNO R4 WiFi con un sensor DHT22 conectado envía el dato de
temperatura a un escenario de Make.com mediante un webhook, y Make
ejecuta el envío del mensaje de alerta.

## 2. Objetivos

- Leer temperatura y humedad con el sensor DHT22 desde el Arduino UNO R4 WiFi.
- Configurar un webhook personalizado en Make.com como disparador del escenario.
- Enviar los datos del Arduino a Make en formato JSON mediante una petición HTTPS (con respaldo HTTP).
- Configurar el módulo de Telegram Bot para enviar un mensaje de alerta con el valor real de temperatura recibido.
- Servir una página web local desde el propio Arduino para monitorear la temperatura, humedad y estado del LED en tiempo real.

## 3. Material utilizado

- Arduino UNO R4 WiFi
- Sensor DHT22
- Protoboard
- LED + resistencia
- Cables Dupont
- Cuenta de Make.com (zona us2.make.com)
- Bot de Telegram (creado con @BotFather)

## 4. Diagrama del circuito

Sensor DHT22 → pin digital 2 (con alimentación 5V/GND), LED → pin 13 en
serie con una resistencia. Ver
[`Diagrama/diagrama_circuito_dht22.png`](../Diagrama/diagrama_circuito_dht22.png).

## 5. Escenario de Make.com

El escenario **"Leer DTH22"** está compuesto por dos módulos conectados
en secuencia (ver
[`Codigo/make_scenario_leer_dth22.json`](../Codigo/make_scenario_leer_dth22.json)):

| Módulo | Tipo | Función |
|---|---|---|
| 1. Webhooks — Custom webhook | Disparador | Recibe el POST con `{"temperatura": <valor>}` que envía el Arduino |
| 2. Telegram Bot — Send a Text Message | Acción | Envía el mensaje `⚠️ El cuarto está muy caliente ({{1.temperatura}}). Enciende el aire acondicionado.` al chat configurado |

## 6. Código del Arduino

Ver [`Codigo/sensor_dht22_alerta.ino`](../Codigo/sensor_dht22_alerta.ino)
(las credenciales de WiFi y la URL del webhook se definen aparte en un
archivo local `secrets.h`, no incluido en el repo).

Además de la lógica de lectura y envío de alertas, el código incluye dos
funciones que no están en la guía original:

- **Servidor web local** (`enviarPagina`) — el Arduino expone su propia
  IP en la red WiFi y sirve una página HTML que muestra la temperatura,
  humedad y estado del LED en vivo, refrescándose cada 5 segundos.
- **Envío con respaldo HTTPS → HTTP** (`enviarAlertaMake`) — intenta
  primero conectar por HTTPS (puerto 443); si falla, reintenta por HTTP
  (puerto 80) antes de darse por vencido.

### Lógica de disparo por cambio de estado

El Arduino solo envía la alerta la **primera vez** que la temperatura
cruza el umbral (`UMBRAL_TEMP`), evitando mensajes duplicados mientras
se mantiene alta. Solo se vuelve a enviar si la temperatura primero baja
del umbral y después vuelve a subir.

## 7. Resultados

- Mientras la temperatura se mantiene por debajo del umbral, el LED
  permanece apagado y la página local muestra el estado "Normal"; no se
  envía ninguna petición a Make.
- En cuanto la temperatura alcanza o supera el umbral, el LED se
  enciende, la página cambia a "CALIENTE (LED encendido)", y el Arduino
  envía una sola petición POST al webhook.
- Make recibe el JSON, ejecuta el módulo de Telegram y el destinatario
  recibe el mensaje de alerta con la temperatura real detectada.

Evidencia del circuito armado en
[`Resultados/circuito_armado_1.jpeg`](../Resultados/circuito_armado_1.jpeg)
y
[`Resultados/circuito_armado_2.jpeg`](../Resultados/circuito_armado_2.jpeg).

## 8. Observaciones

- La red WiFi debe ser de 2.4 GHz; el módulo WiFi del UNO R4 no se
  conecta a redes de 5 GHz.
- El bot de Telegram necesita recibir un mensaje "Start" de parte del
  destinatario antes de la primera prueba, o no podrá enviarle mensajes.
- El nombre del campo JSON (`temperatura`) debe coincidir exactamente
  con el que Make detectó al capturar la muestra de datos, o el módulo
  de Telegram no podrá mapearlo.

## 9. Conclusiones

La práctica permitió integrar un sensor físico (DHT22), una placa con
conectividad WiFi (Arduino UNO R4) y una plataforma de automatización en
la nube (Make.com) para construir un sistema de alerta funcional de
punta a punta: desde la lectura del sensor hasta la notificación al
usuario por Telegram, sin intervención manual.
