# Solar-Grow (solarApp)

**Estacion ambiental inteligente para el cuidado de plantas, impulsada con energia solar e inteligencia artificial: backend FastAPI + app movil Flutter + dispositivo IoT basado en ESP8266.**

![Python](https://img.shields.io/badge/Python-3.13+-3776AB) ![FastAPI](https://img.shields.io/badge/FastAPI-0.115-009688) ![Flutter](https://img.shields.io/badge/Flutter-3.x-02569B) ![PostgreSQL](https://img.shields.io/badge/PostgreSQL-16-316192) ![ESP8266](https://img.shields.io/badge/ESP8266-NodeMCU-4B8F29)

---

## Que es Solar-Grow y para que sirve

Solar-Grow es un proyecto IoT de agricultura inteligente de bajo costo que construye una **maceta inteligente con energia solar**. El sistema monitorea en tiempo real las condiciones ambientales de la planta (temperatura, humedad del aire y humedad del suelo) y automatiza acciones como el **riego** cuando el suelo se seca.

La solucion se compone de tres partes que trabajan en conjunto:

1. **Dispositivo IoT (ESP8266 NodeMCU)**: una maceta inteligente con sensores conectada a un servidor local. Lee la humedad del suelo, la temperatura y la humedad del aire, controla una mini bomba de agua y recibe ordenes de riego manual.
2. **Backend (FastAPI)**: API central que almacena los datos en PostgreSQL, expone los servicios para la aplicacion movil, controla la maceta por puerto serial, se comunica por MQTT con la Raspberry Pi y usa **Google Gemini** para el diagnostico con imagenes y la generacion de recomendaciones de cuidado.
3. **Frontend (Flutter)**: aplicacion movil (Android e iOS) y web desde la cual el usuario registra sus plantas, ve los sensores en tiempo real, crea tareas de cuidado, revisa estadisticas y recibe recomendaciones generadas por IA.

Su objetivo es que cualquier persona pueda cuidar mejor sus plantas con tecnologia de bajo costo, aprendizaje automatizado y un asistente inteligente que analiza los datos y las imagenes de la planta.

## Componentes del sistema

### Maceta inteligente (ESP8266 NodeMCU)

Codigo Arduino en `esp8266_maceta_inteligente/`:

- Sensor de **humedad de suelo capacitivo v2.0** (entrada analogica A0) con conversion local a porcentaje.
- Sensor **DHT11** de temperatura y humedad del aire.
- **Optoacoplador CW401 + MOSFET IRFZ44N** para controlar la mini bomba de agua.
- **Riego automatico**: si la humedad del suelo es menor al 40 %, enciende la bomba de forma automatica.
- **Riego manual**: acepta el comando serial `WATER:<segundos>` enviado por el backend para regar durante un tiempo especifico.
- Envia los datos formateados por puerto serial cada 2 segundos sin bloquear el CPU.

### Backend (FastAPI)

APIs agrupadas en `backend/app/routers/`:

- **Auth** (`/api/auth`): registro e inicio de sesion con tokens JWT (python-jose) y contrasenas con bcrypt.
- **Plants** (`/api/plants`): CRUD de plantas del usuario, con datos botanicos, ubicacion (balcon, jardin, terraza, etc.) y dificultad de cuidado.
- **Sensors** (`/api/sensors`): registro y consulta de datos de sensores (temperatura, humedad de aire y de suelo, luz).
- **Tasks** (`/api/tasks`): tareas de cuidado de cada planta (advertencias: regar, abonar, podar, etc.).
- **Controls** (`/api/controls`): comandos de control del sistema (riego manual y automatico).
- **AI** (`/api/ai`): recomendaciones de cuidado generadas a partir de los sensores, imagenes de la camara y el historial de cuidados, ademas del diagnostico por vision.
- **Hardware** (`/api/hardware`): integracion con el ESP8266 por puerto serial (datos en tiempo real) y con la webcam de la PC para capturar la foto de diagnostico.

### Frontend (Flutter)

Aplicacion movil y web en `frontend/`:

- **Pantallas**: splash, login, registro, inicio (mis plantas y tareas), detalle de planta, registrar planta, informacion de la planta, recomendaciones, estadisticas, notificaciones, perfil y edicion de perfil.
- **Estado**: provider con autenticacion, plantas y tareas sincronizadas con la API.
- **Widgets**: planta animada, tarjeta de sensor, tarjeta de tarea, barra de clima.
- **Graficos**: estadisticas con `fl_chart`.
- **Plataformas**: Android, iOS y web (carpetas `android/`, `ios/`, `web/`).

## Stack tecnologico

- **Python 3.13+** y **FastAPI 0.115** (backend).
- **SQLAlchemy 2.0** con **PostgreSQL** (driver psycopg) como base de datos.
- **JWT** (python-jose + passlib/bcrypt) para autenticacion.
- **Hemqtt/MQTT** para la comunicacion con la Raspberry Pi.
- **Google Gemini API** para diagnostico por vision y recomendaciones de cuidado.
- **Flutter** con **provider**, **go_router** y **fl_chart** (app movil).
- **Arduino** para el firmware del ESP8266 NodeMCU.

## Estructura del proyecto

```
solarApp/
├── backend/                               # API FastAPI
│   ├── app/
│   │   ├── main.py                        # Punto de entrada de la API
│   │   ├── config.py                      # Configuracion (BD, JWT, MQTT, serial, Gemini)
│   │   ├── database.py                    # Motor de SQLAlchemy y sesiones
│   │   ├── models/                        # User, Plant, SensorData, Task, Recommendation, AIDiagnosis
│   │   ├── routers/                       # auth, plants, sensors, tasks, controls, ai, hardware
│   │   ├── schemas/                       # Esquemas Pydantic de entrada/salida
│   │   └── utils/
│   │       ├── auth.py                    # Dependencias de JWT
│   │       └── hardware_helper.py         # Serial ESP8266 y captura de webcam
│   ├── static/                            # Avatares, imagenes de plantas y diagnostico
│   ├── requirements.txt                   # Dependencias de Python
│   └── seed_data.py                       # Datos de ejemplo
├── frontend/                              # App Flutter (solar_grow)
│   ├── lib/
│   │   ├── main.dart
│   │   ├── config/                        # api_constants, theme
│   │   ├── models/                        # plant, sensor_data, task, user
│   │   ├── providers/                     # auth, plant, task
│   │   ├── screens/                       # auth, home, plant, recommendations, stats, notifications, profile, splash
│   │   ├── services/                      # api_service
│   │   └── widgets/                       # animated_plant, sensor_card, task_card, weather_bar
│   ├── android/  ios/  web/               # Plataformas generadas por Flutter
│   └── pubspec.yaml
└── esp8266_maceta_inteligente/
    └── esp8266_maceta_inteligente.ino     # Firmware de la maceta inteligente
```

## Configuracion

Toda la configuracion del backend se define en `backend/app/config.py` (se puede sobrescribir con variables de entorno o un archivo `.env`):

| Variable | Descripcion | Valor por defecto |
| --- | --- | --- |
| `DATABASE_URL` | Cadena de conexion a PostgreSQL | `postgresql+psycopg://postgres:123456@localhost:5432/solar_grow_db` |
| `SECRET_KEY` | Clave para firmar los tokens JWT | `solar-grow-secret-key-change-in-production-2024` |
| `ACCESS_TOKEN_EXPIRE_MINUTES` | Duracion del token (minutos) | `1440` (24 h) |
| `MQTT_BROKER` / `MQTT_PORT` | Broker MQTT para la Raspberry Pi | `localhost` : `1883` |
| `RASPBERRY_PI_URL` | URL de la Raspberry Pi | `http://192.168.1.100:5000` |
| `SERIAL_PORT` | Puerto serial del ESP8266 | `COM3` |
| `SERIAL_BAUDRATE` | Velocidad del puerto serial | `115200` |
| `GEMINI_API_KEY` | Clave de Google Gemini (gratis en AI Studio) | vacio |

## Instalacion y puesta en marcha

### Backend

```bash
cd backend
python -m venv venv
# Windows
venv\Scripts\activate
# Linux / macOS
source venv/bin/activate

pip install -r requirements.txt
```

Configura la base de datos PostgreSQL (o ajusta `DATABASE_URL`) y, opcionalmente, define `GEMINI_API_KEY`. Las tablas se crean automaticamente al iniciar la API:

```bash
uvicorn app.main:app --reload --port 8000
```

Documentacion interactiva en `http://localhost:8000/docs` (Swagger).

### Frontend (Flutter)

```bash
cd frontend
flutter pub get
flutter run            # en un emulador o dispositivo
```

Ajusta la URL del backend en `lib/config/api_constants.dart` segun donde corra la API.

### Maceta inteligente (ESP8266)

1. Abre `esp8266_maceta_inteligente/esp8266_maceta_inteligente.ino` en el IDE de Arduino.
2. Configura el modelo de placa (NodeMCU 1.0 ESP-12E), la conexion WIFI y las credenciales.
3. Sube el firmware a la placa y conectala por USB a la PC donde corre el backend.
4. Asegurate de que el puerto serial de `config.py` (`SERIAL_PORT`) coincida con el de la placa.

## API (resumen)

Todas las rutas bajo `/api`. Las rutas protegidas requieren el token JWT (`Authorization: Bearer <token>`).

| Area | Endpoints |
| --- | --- |
| **Autenticacion** | `POST /api/auth/register`, `POST /api/auth/login`, datos del usuario autenticado |
| **Plantas** | CRUD de `GET/POST/PUT/DELETE /api/plants` |
| **Sensores** | Registro y consulta de `GET/POST /api/sensors` |
| **Tareas** | CRUD de tareas de cuidado por planta |
| **Controles** | Comandos de riego (automatico y manual) |
| **IA** | `GET /api/ai/recommendations/{plant_id}` recomendaciones, diagnostico por imagen con Gemini |
| **Hardware** | `GET /api/hardware/clima-actual` datos en vivo del ESP8266 por serial, captura de foto y diagnostico |

## Notas de seguridad

- Las credenciales y claves del archivo `config.py` (`DATABASE_URL` con `postgres:123456`, `SECRET_KEY`, etc.) son valores de desarrollo. Cambialas y define un archivo `.env` antes de usar el sistema en produccion.
- El archivo `.env` nunca se sube al repositorio (esta en el `.gitignore`).

## Licencia

MIT