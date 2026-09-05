/*
   ============================================================
   PROYECTO MACETA INTELIGENTE - ESP8266 NodeMCU
   ============================================================

   COMPONENTES:
   - ESP8266 NodeMCU
   - Sensor humedad suelo capacitivo v2.0 (A0)
   - Sensor DHT11 (D4)
   - Optoacoplador CW401 + MOSFET IRFZ44N (D1)
   - LED Indicador y Mini bomba de agua (Controlados por D1)

   FUNCIONAMIENTO:
   - Lee la humedad del suelo y convierte a porcentaje de manera local.
   - Lee temperatura y humedad ambiente del DHT11.
   - Si la humedad es < 40% (planta seca), enciende la bomba automáticamente.
   - Soporta comando serial "WATER:<segundos>" enviado por el servidor Python de solarApp
     para activar un riego manual de duración específica.
   - Envía los datos formateados por Serial cada 2 segundos sin bloquear el CPU.

   ============================================================
*/

#include <DHT.h>

// =======================
// CONFIGURACION DHT11
// =======================
#define DHTPIN D4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// =======================
// SENSOR HUMEDAD SUELO
// =======================
#define SOIL_PIN A0

// =======================
// LED / MOSFET (BOMBA)
// =======================
#define LED_PIN D1

// =======================
// CALIBRACION SENSOR
// =======================
// Sensor totalmente al aire (seco) -> valor alto
// Sensor sumergido en agua (húmedo) -> valor bajo
const int valorSeco = 780;
const int valorHumedo = 350;

// Variables de sensores
int humedadSueloRaw = 0;
int porcentajeHumedad = 0;
float temperatura = 0.0;
float humedadAire = 0.0;

// Variables para el riego manual remoto (serial)
bool riegoManualActivo = false;
unsigned long finRiegoManual = 0;

// Temporizadores no bloqueantes
unsigned long ultimoEnvioSensores = 0;
const unsigned long intervaloSensores = 2000; // Enviar datos cada 2000 ms (2 segundos)

void setup() {
  // Inicialización de la comunicación serial
  Serial.begin(115200);
  
  // Inicializar sensor DHT
  dht.begin();
  
  // Configurar pin de salida del LED/Bomba
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Apagado por defecto

  Serial.println("====================================");
  Serial.println("SISTEMA MACETA INTELIGENTE INICIADO");
  Serial.println("====================================");
}

void loop() {
  // 1. LEER Y PROCESAR COMANDOS SERIALES (RIEGO MANUAL)
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();
    
    // El backend envía el comando en formato WATER:<segundos>
    if (comando.startsWith("WATER:")) {
      int duracionSegundos = comando.substring(6).toInt();
      if (duracionSegundos > 0) {
        riegoManualActivo = true;
        finRiegoManual = millis() + ((unsigned long)duracionSegundos * 1000);
        Serial.print("COMANDO RECIBIDO -> REGAR POR ");
        Serial.print(duracionSegundos);
        Serial.println(" SEGUNDOS");
      }
    }
  }

  // 2. VERIFICAR SI TERMINÓ EL RIEGO MANUAL
  if (riegoManualActivo && millis() >= finRiegoManual) {
    riegoManualActivo = false;
    Serial.println("RIEGO MANUAL FINALIZADO");
  }

  // 3. DETERMINAR SI SE DEBE ACTIVAR LA BOMBA
  // Se activa por tierra seca (Humedad < 40%) O por riego manual remoto
  bool bombaDeberiaEncender = (porcentajeHumedad < 40) || riegoManualActivo;

  if (bombaDeberiaEncender) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  // 4. LECTURA DE SENSORES Y ENVÍO DE DATOS PERIÓDICO (CADA 2 SEGUNDOS)
  unsigned long tiempoActual = millis();
  if (tiempoActual - ultimoEnvioSensores >= intervaloSensores) {
    ultimoEnvioSensores = tiempoActual;

    // Lectura del sensor de humedad de suelo capacitivo
    humedadSueloRaw = analogRead(SOIL_PIN);
    
    // Mapear el valor analógico crudo al rango porcentual de 0 a 100
    porcentajeHumedad = map(humedadSueloRaw, valorSeco, valorHumedo, 0, 100);
    porcentajeHumedad = constrain(porcentajeHumedad, 0, 100);

    // Lectura de temperatura y humedad ambiente
    float tempRead = dht.readTemperature();
    float humRead = dht.readHumidity();

    // Comprobar lecturas válidas del sensor DHT11
    if (!isnan(tempRead)) {
      temperatura = tempRead;
    }
    if (!isnan(humRead)) {
      humedadAire = humRead;
    }

    // Imprimir los datos en consola en el formato exacto que espera el backend de Python
    Serial.println("================================");
    Serial.print("Valor analogico: ");
    Serial.println(humedadSueloRaw);

    Serial.print("Humedad suelo: ");
    Serial.print(porcentajeHumedad);
    Serial.println(" %");

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");

    Serial.print("Humedad ambiente: ");
    Serial.print(humedadAire);
    Serial.println(" %");

    // Imprimir el estado del LED/Bomba para que el backend actualice el estado en tiempo real
    if (bombaDeberiaEncender) {
      Serial.println("TIERRA SECA -> BOMBA ENCENDIDO");
      Serial.println("BOMBA ENCENDIDO");
    } else {
      Serial.println("TIERRA HUMEDA -> BOMBA APAGADO");
      Serial.println("BOMBA APAGADO");
    }
    Serial.println("================================");
  }
}
