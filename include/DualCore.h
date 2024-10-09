#ifndef DualCore_h
#define DualCore_h

// Librerías usadas
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "Objetos.h"
#include "DualCore.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <SD.h>
// #include <Audio.h>
#include <FS.h>

// Claves de los núcleos
#define NUCLEO_PRIMARIO 0X01
#define NUCLEO_SECUNDARIO 0X00

/* --- DEFINICION DE PINES --- */
// Conexiones del Joystick
#define VRX_PIN 27 // ESP32 pin GPIO36 (ADC0) connected to VRX pin
#define VRY_PIN 13 // ESP32 pin GPIO39 (ADC0) connected to VRY pin
// Conexiones de los botones
#define BTN_EXIT 34
#define BTN_ENTER 35
// Conexión del módulo SD
#define CS_PIN 5
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23

// I2S Connections
#define I2S_DOUT 15
#define I2S_BCLK 26
#define I2S_LRC 25

File root; // Instancia de la clase para SD

// Audio audio; // Instancia de la clase para Audio

// Pin del buzzer
#define BUZZER_PIN 4

// Valores maximos del joystick
#define MAX_VERT 4095
#define MAX_HORI 4095

// Variables para leer los valores del Joystick
int valueX = 0; // to store the X-axis value
int valueY = 0; // to store the Y-axis value

// Variables globales para almacenar el puntaje y nivel del juego actual (Funciona para almacenar valores cuando se pausa el juego).
int checkPointPuntaje = 0;
int checkPointNivel = 0;

/* --- CARACTERES PERSONALIZADOS --- */
// Personaje
byte characterPersonaje[] = {B01110, B01010, B01110, B11111, B00100, B00100, B01010, B10001};

// Diamante
byte characterDiamante[] = {B00000, B00000, B01110, B11111, B11111, B01110, B00100, B00000};

/*~ Instancia de la clase para el manejo de la pantalla ( Dirección I2C, cantidad de columnas, cantidad de filas ) ~*/
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Creación de objetos del Personaje y Diamante
Personaje personaje(0, 0);
Diamante objetivo(random(13), random(2));

// Banderas globales (No usadas aún)
bool isPauseActivated = false;
bool isGameInProgress = false;
bool isTheLevelFinishedWithSuccess = false;
bool isAudioStopped = false;

// Variable para registrar el contador de tiempo
unsigned long inicioMilis = 0;

/* -- INSTANCIAS FREE-RTOS para TASKs -- */
TaskHandle_t MusicTask_t;
TaskHandle_t GameLogicTask_t;
TaskHandle_t GamePauseTask_t;

/* -- INSTANCIAS FREE-RTOS para QUEUES para comunicación entre núcleos */
QueueHandle_t musicQueue;
QueueHandle_t gameQueue;

// Enumeración para los estados de la música
enum MusicState
{
    MUSIC_INTRO,
    MUSIC_MENU,
    MUSIC_GAME,
    MUSIC_ELEVATOR,
    MUSIC_PAUSE
};

// Enumeración para los estados del juego
enum GameState
{
    STATE_INTRO,
    STATE_MENU,
    STATE_GAME,
    STATE_SCORES,
    STATE_PAUSE
};

// Variables globales para el estado inicial de la música y del juego
volatile MusicState currentMusicState = MUSIC_INTRO;
volatile GameState currentGameState = STATE_INTRO;

// Variables globales para elegir nombre
int posChar = 0;                     // posicion del caracter
int posLetra = 0;                    // posicion de la letra
char nom[4] = {'A', 'A', 'A', '\0'}; // cadena con el nombre del jugador
int PuntajeTop = 0;

// Encabezados de funciones
void ChangeMusic(MusicState newState);                            // Cambiar música
void ChangeGameState(GameState newState);                         // Cambiar de estados del juego
void ReadMaxScores(void);                                         // Leer scores máximos
void IntroGame(void);                                             // Ejecutar el intro
void PrintDirectory(File dir, int numTabs);                       // Imprimir directorio
void ActivarBuzzer(unsigned int frecuency, unsigned long millis); // Activar PinBuzzer
void MostrarMenuPausa(void);                                      // Menú de pausa
void MostrarMenuPrincipal(void);                                  // Menú principal
bool nivel(int contador, int puntosRequeridos, int puntajeEntrante);
void JuegoCompleto(void); // Lógica completa del juego
void EvaluarNivelFinal(void);
char *ElegirNombre(void);
void GuardarScore(int Puntaje, char *Nombre);

/*--- CLASE MAESTRA --- */

class DualCoreESP32
{
public:
    DualCoreESP32();
    void ConfigCores(void);

private:
    static void MusicTask(void *pvParameters);
    static void PauseTask(void *pvParameters);
    static void GameLogicTask(void *pvParameters);
};

/* --- Funciones de la Clase DualCoreESP32 --- */

// Inicializar colas en el constructor
DualCoreESP32::DualCoreESP32()
{
    musicQueue = xQueueCreate(1, sizeof(MusicState));
    gameQueue = xQueueCreate(1, sizeof(GameState));
}

// Creación de Tareas(3) Para el DualCore
void DualCoreESP32 ::ConfigCores(void)
{
    // ---> Empieza el setup de los pines, micro SD y Audio

    // Estados de pines
    pinMode(VRX_PIN, INPUT);          // Entrada para el eje X
    pinMode(VRY_PIN, INPUT);          // Entrada para el eje Y
    pinMode(BTN_EXIT, INPUT_PULLUP);  // Boton
    pinMode(BTN_ENTER, INPUT_PULLUP); // Boton

    // Set microSD Card CS as OUTPUT and set HIGH
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);

    // --- INICIALIZACION DE MÓDULO SD ---
    Serial.print(F("Initializing SD card... "));

    if (!SD.begin(CS_PIN))
    {
        Serial.println(F("Card initialization failed!"));
        while (true)
            ;
    }

    Serial.println(F("Initialization done."));

    // Initialize SPI bus for microSD Card
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    // Initialize SPI bus for microSD Card
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    // Setup I2S
    // audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

    // Set Volume
    // audio.setVolume(21);

    Serial.println(F("Files in the card:"));
    root = SD.open("/");
    PrintDirectory(root, 0);
    Serial.println("");

    /*~ Inicializar la pantalla LCD ~*/
    lcd.init();
    lcd.backlight();

    // Creación de caracteres principales
    lcd.createChar(0, characterPersonaje);
    lcd.createChar(1, characterDiamante);

    // Registrar tiempo
    inicioMilis = millis();

    // Tarea para la música
    xTaskCreatePinnedToCore(
        this->MusicTask,
        "Musica",
        30000,
        NULL,
        1,
        &MusicTask_t,
        NUCLEO_SECUNDARIO);

    // Tarea para la lógica del juego
    xTaskCreatePinnedToCore(
        this->GameLogicTask,
        "LogicaJuego",
        35000,
        NULL,
        1,
        &GameLogicTask_t,
        NUCLEO_PRIMARIO);

    // Tarea para manejar únicamente la pausa del juego
    xTaskCreatePinnedToCore(
        this->PauseTask,
        "PausaDelJuego",
        1000,
        NULL,
        1,
        &GamePauseTask_t,
        NUCLEO_SECUNDARIO);
}

// --- TASKS DE LOS CORES --

// Tarea para manejar la pausa del juego.
void DualCoreESP32 ::PauseTask(void *pvParameters)
{
    while (true)
    {
        if (!digitalRead(BTN_EXIT) && isGameInProgress == true)
            isPauseActivated = true;
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

// Tarea para cambiar entre música
void DualCoreESP32 ::MusicTask(void *pvParameters)
{
    MusicState newState;

    while (true)
    {
        if (xQueueReceive(musicQueue, &newState, 0) == pdTRUE)
        {
            currentMusicState = newState;

            // Detener la música actual
            // audio.stopSong();
            // vTaskDelay(10 / portTICK_PERIOD_MS);

            switch (currentMusicState)
            {
            case MUSIC_INTRO:
                // if (!SD.exists("/intro.mp3"))
                // Serial.println("Archivo intro.mp3 no encontrado");
                // audio.connecttoFS(SD,"/intro.mp3");
                break;
            case MUSIC_MENU:
                // if (!SD.exists("/menu.mp3"))
                // Serial.println("Archivo menu.mp3 no encontrado");
                // audio.connecttoFS(SD,"/menu.mp3");
                break;
            case MUSIC_GAME:
                // if (!SD.exists("/game.mp3"))
                // Serial.println("Archivo game.mp3 no encontrado");
                // audio.connecttoFS(SD,"/game.mp3");
                break;
            case MUSIC_PAUSE:
                // audio.stopSong();
                // isAudioStopped = true;
                break;
            case MUSIC_ELEVATOR:
                // if (!SD.exists("/elevator.mp3"))
                // Serial.println("Archivo elevator.mp3 no encontrado");
                // audio.connecttoFS(SD,"/elevator.mp3");
                break;
            default:
                break;
            }
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

// Tarea para correr toda la lógica del juego
void DualCoreESP32 ::GameLogicTask(void *pvParameters)
{
    GameState newState;
    while (1)
    {
        if (xQueueReceive(gameQueue, &newState, 0) == pdTRUE)
        {
            currentGameState = newState;
            switch (currentGameState)
            {
            // INTRODUCCIÓN
            case STATE_INTRO:
                ChangeMusic(MUSIC_INTRO);
                IntroGame();
                break;
            // MENU
            case STATE_MENU:
                ChangeMusic(MUSIC_MENU);
                MostrarMenuPrincipal();
                break;
            // JUEGO
            case STATE_GAME:
                ChangeMusic(MUSIC_GAME);
                JuegoCompleto();
                break;
            // PUNTAJES
            case STATE_SCORES:
                ChangeMusic(MUSIC_ELEVATOR);
                ReadMaxScores();
                break;
            // PAUSA
            case STATE_PAUSE:
                ChangeMusic(MUSIC_PAUSE);
                MostrarMenuPausa();
            default:
                break;
            }
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

/**** --- INICIO DE FUNCIONES GLOBALES --- ****/

//-- Función para cambiar el estado de la música (cambiar música)
void ChangeMusic(MusicState newState)
{
    xQueueSend(musicQueue, &newState, 0);
}

//-- Función para cambiar el estado del juego; cambiar entre funciones
void ChangeGameState(GameState newState)
{
    xQueueSend(gameQueue, &newState, 0);
}

//-- Estado STATE_SCORES;
// Lee la el archivo JSON e imprime los 4 mejores Scores guardados
void ReadMaxScores(void)
{
    // Borrar pantalla
    lcd.clear();

    File JsonFile = SD.open("/GameData.json");
    if (!JsonFile)
    {
        Serial.println("Error opening GameData.json");
        return;
    }

    // JSON Doc
    JsonDocument doc;
    deserializeJson(doc, JsonFile);
    JsonArray bestScores = doc["bestScores"].as<JsonArray>();
    JsonFile.close(); // Cierra el archivo antes de entrar en el bucle

    int scoreCount = bestScores.size();
    int state = digitalRead(BTN_EXIT);
    int contador = 0;
    while (digitalRead(BTN_EXIT))
    {
        JsonObject score = bestScores[contador];
        const char *name = score["name"];
        int scoreValue = score["score"];

        if (name && strlen(name) > 0)
        {
            lcd.setCursor(0, 0);
            lcd.print(contador + 1); // Posición
            lcd.print(" Pos| Name: ");
            lcd.print(name);
            lcd.setCursor(7, 1);
            lcd.print("Score: ");
            lcd.print(scoreValue);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            lcd.clear();
        }

        // Cambio al menú principal
        if (!digitalRead(BTN_EXIT))
            ChangeGameState(STATE_MENU);
        contador++;
        if (contador > 4)
            contador = 0;
    }
}

//-- Función para reproducir el Intro del juego
void IntroGame()
{
    // Variables locales para la intro
    const byte characterFull = 0xFF;
    const byte characterEmpty = 0x20;
    const char *autores[] = {"=== Autores ===", "Alonso Flores", "Joel Garcia", "Victor Martinez", "Eric Puente"};
    const char *title = "Catch the";
    const char *nameGame = "Diamonds";

    byte diamondTopLeft[] = {B00000, B00000, B00111, B01000, B10100, B10010, B10001, B01000};
    byte diamondTop[] = {B00000, B00000, B11111, B10001, B01010, B00100, B01010, B10001};
    byte diamondTopRight[] = {B00000, B00000, B11100, B00010, B00101, B01001, B10001, B00010};
    byte diamondBottomLeft[] = {B00101, B00010, B00001, B00000, B00000, B00000, B00000, B00000};
    byte diamondBottom[] = {B10001, B01010, B00100, B10001, B01110, B00000, B00000, B00000};
    byte diamondBottomRight[] = {B10100, B01000, B10000, B00000, B00000, B00000, B00000, B00000};

    // Crea los caracteres del diamante del 3 al 6
    lcd.createChar(2, diamondTopLeft);
    lcd.createChar(3, diamondTop);
    lcd.createChar(4, diamondTopRight);
    lcd.createChar(5, diamondBottomLeft);
    lcd.createChar(6, diamondBottom);
    lcd.createChar(7, diamondBottomRight);

    // Iniciar eliminando todo en pantalla
    lcd.clear();

    // "Feria" de caracteres
    lcd.write(characterFull);
    for (int i = 0; i < 17; i++)
    {
        lcd.setCursor(i + 1, 0);
        lcd.write(characterFull);
        lcd.setCursor(i, 1);
        lcd.write(characterFull);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    for (int i = 15; i > 0; i--)
    {
        lcd.setCursor(i - 1, 0);
        lcd.write(characterEmpty);
        lcd.setCursor(i, 1);
        lcd.write(characterEmpty);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
    lcd.clear();

    // Dibuja el diamante
    lcd.setCursor(6, 0);
    for (int i = 2; i <= 4; i++)
        lcd.write(byte(i));

    lcd.setCursor(6, 1);
    for (int i = 5; i <= 7; i++)
        lcd.write(byte(i));

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    lcd.clear();

    // Titulo del juego desplazándose
    lcd.print(title);
    for (int i = 0; i < 7; i++)
    {
        // Mostrar el título desplazándose
        lcd.scrollDisplayRight();
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }

    // Nombre del juego
    lcd.setCursor(8, 1);
    lcd.print(nameGame);
    for (int i = 0; i < 7; i++)
    {
        lcd.scrollDisplayLeft();
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);

    lcd.clear();

    // Imprimir nombre de los autores
    const int numAutores = sizeof(autores) / sizeof(autores[0]);
    bool finalizadoCorrectamente = false;

    for (int i = 0; i < numAutores; i += 2)
    {
        lcd.clear();

        // Primer autor
        lcd.setCursor(0, 0);
        lcd.print(autores[i]);

        // Segundo autor (si existe)
        if (i + 1 < numAutores)
        {
            lcd.setCursor(0, 1);
            lcd.print(autores[i + 1]);
        }
        if (i + 2 >= numAutores)
        {
            finalizadoCorrectamente = true;
        }
        vTaskDelay(1200 / portTICK_PERIOD_MS);
    }

    if (finalizadoCorrectamente)
    {
        Serial.println("Finalizó la ronda FOR de autores");
        ChangeGameState(STATE_MENU);
    }
    else
    {
        Serial.println("Error: el ciclo no finalizó correctamente");
    }
}

//-- Función para imprimir el directorio que tiene la MicroSD --
void PrintDirectory(File dir, int numTabs)
{
    while (true)
    {
        File entry = dir.openNextFile();
        if (!entry)
        {
            // no more files
            break;
        }
        for (uint8_t i = 0; i < numTabs; i++)
        {
            Serial.print('\t');
        }
        Serial.print(entry.name());
        if (entry.isDirectory())
        {
            Serial.println("/");
            PrintDirectory(entry, numTabs + 1);
        }
        else
        {
            // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}

void ActivarBuzzer(unsigned int frecuency, unsigned long millis)
{
    tone(BUZZER_PIN, frecuency, millis);
}

void MostrarMenuPrincipal(void)
{
    int optionToSelect = 0;
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Comenzar");
    lcd.setCursor(1, 1);
    lcd.print("Scores");

    while (digitalRead(BTN_ENTER))
    {
        valueY = analogRead(VRY_PIN); // Leer eje Y del joystick
        // Cambiar selección del menú
        // Ajustar para mover hacia abajo
        if (valueY < 1000)
        {
            lcd.setCursor(0, 0);
            lcd.write(0x20);
            optionToSelect = 1;
            ActivarBuzzer(2000, 50);
        }
        if (valueY == MAX_VERT)
        {
            lcd.setCursor(0, 1);
            lcd.write(0x20);
            optionToSelect = 0;
            ActivarBuzzer(2000, 50);
        }
        lcd.setCursor(0, optionToSelect);
        lcd.write(0x7E); // Flecha (→)
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    // Serial.println("Sale del programa");
    switch (optionToSelect)
    {
    case 0: // Se envía a iniciar juego
        ChangeGameState(STATE_GAME);
    case 1: // Se envía a scores
        ChangeGameState(STATE_SCORES);
    }
}

void MostrarMenuPausa(void)
{
    int optionToSelect = 0;
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Reanudar");
    lcd.setCursor(1, 1);
    lcd.print("Menu principal");

    while (digitalRead(BTN_ENTER))
    {
        valueY = analogRead(VRY_PIN); // Leer eje Y del joystick
        // Cambiar selección del menú
        // Ajustar para mover hacia abajo
        if (valueY < 1000)
        {
            lcd.setCursor(0, 0);
            lcd.write(0x20);
            optionToSelect = 1;
            ActivarBuzzer(2000, 50);
        }
        if (valueY == MAX_VERT)
        {
            lcd.setCursor(0, 1);
            lcd.write(0x20);
            optionToSelect = 0;
            ActivarBuzzer(2000, 50);
        }
        lcd.setCursor(0, optionToSelect);
        lcd.write(0x7E); // Flecha (→)
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    // Serial.println("Sale del programa");
    switch (optionToSelect)
    {
    case 0: // Se reinicia el juego
        ChangeGameState(STATE_GAME);
    case 1: // Se envía menú principal
        isGameInProgress = false;
        ChangeGameState(STATE_MENU);
    }
}

void mostrarMensaje(const char *linea1, const char *linea2)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(linea1);
    lcd.setCursor(0, 1);
    lcd.print(linea2);
}

bool nivel(int duracionEnSegundos, int puntosRequeridos, int puntajeEntrante)
{
    static unsigned long lastUpdateTime = 0;
    const unsigned long UPDATE_INTERVAL = 100; // Actualizar cada 100ms

    unsigned long currentMillis = millis();

    // Actualizar solo cada UPDATE_INTERVAL milisegundos
    if (currentMillis - lastUpdateTime >= UPDATE_INTERVAL)
    {
        lastUpdateTime = currentMillis;

        lcd.clear();

        // Calcular el tiempo restante
        int tiempoRestante = duracionEnSegundos - (currentMillis - inicioMilis) / 1000;

        if (tiempoRestante >= 0)
        {
            // Mover personaje con joystick
            valueX = analogRead(VRX_PIN);
            valueY = analogRead(VRY_PIN);

            if (valueX == MAX_HORI)
            {
                personaje.Left();
            }
            if (valueX < 100)
            {
                personaje.Right();
            }
            if (valueY == MAX_VERT)
            {
                personaje.Up();
            }
            if (valueY < 100)
            {
                personaje.Down();
            }

            // Dibujar en la pantalla LCD
            lcd.setCursor(personaje.GetX(), personaje.GetY());
            lcd.write(0);
            lcd.setCursor(objetivo.GetX(), objetivo.GetY());
            lcd.write(1);
            // Verificar colisión
            if (objetivo.Colision(personaje.GetX(), personaje.GetY(), objetivo.GetX(), objetivo.GetY()))
            {
                ActivarBuzzer(1000, 10);
                personaje.IncrementarPuntaje();
                objetivo.RehubicarObjeto();
            }
            lcd.setCursor(14, 0);
            lcd.print(tiempoRestante);
            lcd.setCursor(14, 1);
            lcd.print(personaje.ImprimirPuntaje());

            return false; // El nivel sigue activo
        }

        // El tiempo se acabó, verificar resultado
        if (personaje.ImprimirPuntaje() - puntajeEntrante >= puntosRequeridos)
        {
            mostrarMensaje("Nivel completado!", " =============> ");
            vTaskDelay(2000 / portTICK_PERIOD_MS); // Dar tiempo para leer el mensaje
            return true;
        }
        else
        {
            mostrarMensaje("Tiempo agotado", "Intenta de nuevo");
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            return true;
        }
    }
    return false; // No ha pasado suficiente tiempo para actualizar
}

void EvaluarNivelFinal(int puntajeFinal)
{
    File JsonFile = SD.open("/GameData.json");
    if (!JsonFile)
    {
        Serial.println("Error opening Scores");
        return;
    }
    // JSON Doc con tamaño predefinido
    JsonDocument doc;
    deserializeJson(doc, JsonFile);
    JsonArray bestScores = doc["bestScores"].as<JsonArray>();
    JsonFile.close(); // Cierra el archivo antes de entrar en el bucle
    JsonObject score = bestScores[0];
    PuntajeTop = bestScores[0]["score"]; // Puntaje Top del Json

    char *nick = nullptr;

    if (personaje.ImprimirPuntaje() >= puntajeFinal)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Ganaste el juego!");
        Serial.println("Juego completado con éxito");

        if (personaje.ImprimirPuntaje() >= PuntajeTop)
        {
            // mostrarMensaje("!Nuevo Puntaje!","Con "+ personaje.ImprimirPuntaje());
            nick = ElegirNombre();
            GuardarScore(personaje.ImprimirPuntaje(), nick);
            Serial.println("Nuevo Score");
        }
    }
    else
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Lo siento...");
        lcd.setCursor(0, 1);
        lcd.print("Fin del juego");
        Serial.println("Fin del juego");
    }

    // Cambiamos estado del juego a terminado
    isGameInProgress = false;
}

void JuegoCompleto()
{
    const int NIVELES = 3;
    const int tiempos[NIVELES] = {10, 10, 10};
    const int puntosRequeridos[NIVELES] = {1, 1, 1};

    // Reiniciamos valores cada vez que se inicie el juego
    if (!isPauseActivated)
    {
        Serial.println("Entro en juego completo");
        personaje.ReiniciarValores();
        checkPointNivel = 0;
        checkPointPuntaje = 0;
    }

    for (int i = checkPointNivel; i < NIVELES; i++)
    {
        lcd.clear();

        // Si no se inicializa desde una pausa, mostrar
        if (!isPauseActivated)
        {
            lcd.print("Nivel ");
            lcd.print(i + 1);
            lcd.setCursor(0, 1);
            lcd.print("Alcanza: ");
            lcd.print(puntosRequeridos[i] + personaje.ImprimirPuntaje());
            lcd.print(" pts.");

            // Guardamos puntaje del personaje
            checkPointPuntaje = personaje.ImprimirPuntaje();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        else
        {
            personaje.puntaje = checkPointPuntaje;
            isPauseActivated = false;
        }

        inicioMilis = millis(); // Reiniciar el tiempo al inicio de cada nivel

        bool nivelCompletado = false;
        isGameInProgress = true;

        while (!nivelCompletado)
        {
            if (!isPauseActivated)
            {
                // Serial.println(isPauseActivated);
                nivelCompletado = nivel(tiempos[i], puntosRequeridos[i], checkPointPuntaje);
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            else
            {
                break;
            }
        }

        // Si no alcanzó los puntos requeridos, terminar el juego
        if (personaje.ImprimirPuntaje() - checkPointPuntaje < puntosRequeridos[i])
        {
            break;
        }

        if (!isPauseActivated)
            checkPointNivel++;
    }

    if (isPauseActivated)
    {
        ChangeGameState(STATE_PAUSE);
    }
    else
    {
        EvaluarNivelFinal(puntosRequeridos[NIVELES - 1]);
        vTaskDelay(2000 / portTICK_PERIOD_MS); // Dar tiempo para leer el mensaje final
        ChangeGameState(STATE_MENU);
    }
}

char *ElegirNombre(void)
{
    while (digitalRead(BTN_ENTER))
    {
        valueX = analogRead(VRX_PIN);
        valueY = analogRead(VRY_PIN);
        Serial.println("Entro");
        char abc[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

        // Mover posicion:
        if (valueX < 100)
        {
            posChar = (posChar < 2) ? posChar + 1 : 2;
        } // Derecha
        if (valueX == MAX_HORI)
        {
            posChar = (posChar > 0) ? posChar - 1 : 0;
        } // Izquierda

        // mover letras:
        if (valueY < 100)
        {
            posLetra = (posLetra < 25) ? posLetra + 1 : 0;
        } // Abajo
        if (valueY == MAX_VERT)
        {
            posLetra = (posLetra > 0) ? posLetra - 1 : 25;
        } // Arriba
        // Asignar la letra seleccionada a la posición correspondiente
        nom[posChar] = abc[posLetra];

        lcd.setCursor(0, 1);
        lcd.print("Nickname: ");
        // Serial.println(PuntajeTop);
        //  Mover cursor a la posición actual y activar parpadeo
        lcd.setCursor(10 + (posChar * 2), 1);
        lcd.print(nom[posChar]);
        lcd.blink();

        // Mostrar las tres letras del nombre
        for (int j = 0; j < 3; j++)
        {
            lcd.setCursor(10 + (j * 2), 1);
            lcd.print(nom[j]);
        }
    }
    char *nombre = nom;
    return nombre;
}

void GuardarScore(int Puntaje, char *Nombre)
{
    bool Iterador = true;
    int Contador = 3;
    File JsonFile = SD.open("/GameData.json");
    if (!JsonFile)
    {
        Serial.println("Error opening Scores");
        return;
    }
    // JSON Doc con tamaño predefinido
    JsonDocument doc;
    deserializeJson(doc, JsonFile);
    JsonArray bestScores = doc["bestScores"].as<JsonArray>();
    JsonFile.close(); // Cierra el archivo antes de entrar en el bucle
    while (Iterador)
    {
        bestScores[Contador]["score"] = bestScores[Contador - 1]["score"];
        bestScores[Contador]["name"] = bestScores[Contador - 1]["name"];
        Contador--;
        if (Contador < 1)
            Iterador = false;
    }

    bestScores[0]["score"] = Puntaje;
    bestScores[0]["name"] = Nombre;

    // Guardar los cambios en el archivo JSON
    JsonFile = SD.open("/GameData.json", FILE_WRITE);
    if (!JsonFile)
    {
        Serial.println("Error opening Scores for writing");
        return;
    }
    // Sobreescribir el archivo con los nuevos datos
    serializeJson(doc, JsonFile);
    JsonFile.close();
}

#endif