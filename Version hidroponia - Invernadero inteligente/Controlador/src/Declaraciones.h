#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>	  // display OLED
#include <Adafruit_SSD1306.h> // display OLED
#include <AHT10.h>
#include <ArduinoJson.h>
#include <CTBot.h>
#include <EEPROM.h>
#include <SD.h>
#include <SPI.h>
#include "time.h" // tiempo unix
#include <Wire.h> // I2C
#include <WiFi.h>
#include <WiFiMulti.h>

#define DEBUG_SERIAL // Comentar para eliminar los Serial.print
#ifdef DEBUG_SERIAL
	#define imprimir(x) Serial.print(x)
	#define imprimirln(x) Serial.println(x)
#else
	#define imprimir(x)
	#define imprimirln(x)
#endif

// Constantes de funcionamiento generales
#define DELAY_ACTIVIDAD_INVERNADERO 0UL // (ms) tiempo de espera para el loop del invernadero
#define W_SSID_SIZE		33 // 32 caracteres + null terminator
#define W_PASS_SIZE		64 // 63 caracteres + null terminator
#define F_EMAIL_SIZE	86 // 63 de domain + 21 de @cet2bariloche.edu.ar (o menos de @gmail y @hotmail.com) + null terminator
#define F_PASS_SIZE		31 // 30 caracteres + null terminator
// Pines
#define SD_CS			5
// ledes y botón
#define PIN_BTN			4
#ifndef LED_BUILTIN
	#define LED_BUILTIN	2
#endif
#define LED_VENTILACION		15 // Active low
#define LED_WIFI			12 // Active low
// de los relés
#define PIN_BOMBA_1			17
#define PIN_BOMBA_2			16
#define PIN_BOMBA_3			26
#define PIN_VENTILACION		27
// del multiplexor
#define MUX_A 32
#define MUX_B 33
#define MUX_C 25
// de los sensores AHT10 (desde el MUX)
enum PinesAHT10MUX : uint8_t
{
	AHT_INT_HIGH_MUX_PIN,
	AHT_INT_MID_MUX_PIN,
	AHT_INT_LOW_MUX_PIN,
	AHT_AGUA_1_MUX_PIN,
	AHT_AGUA_2_MUX_PIN,
	AHT_AGUA_3_MUX_PIN,
	AHT_EXT_MUX_PIN,
	AHT_GEOTERMICO_MUX_PIN,
};
// de los sensores humedad suelo
#define SOIL_1_PIN A0
#define SOIL_2_PIN A3
#define SOIL_3_PIN A6
#define SOIL_4_PIN A7

uint8_t cant_redes_wifi = 0;

// Variables de tiempo generales
unsigned long ultima_vez_invernadero_funciono = 0;
unsigned long ultima_vez_display_cambio = 0;

// Flags de estado generales
/* -------------------------IDEA:-------------------------
Posiblemente crear class Salida con flags: activada, desactivada, forzada, ultima_vez_activada/desactivada.
Y crear child classes con los métodos propios: abrir(), chequear(), prender(), apagar(), abrir un ángulo(), esperando, temp_max, etc;
declarando Riego, Calefa, Ventilación y quizás Alarma. Ver cómo incorporar los valores de la EEPROM (fácil, pero declararlos todos
en el mismo lugar que los otros valores de la EEPROM [humedad suelo es el único no relacionado con una salida]).

-------------------------2DA IDEA:-------------------------
Cambiar esto por un struct FlagsSalidas. Las funciones de cada salida pueden quedar sueltas. No es muy buen encapsulado pero meh.
*/
bool ventilacion_forzada	= false; // si el estado de ventilación está siendo forzado por telegram
bool ventilando				= false;
bool esperando_riego		= false; // para chequearRiego()

// Tiempo.h
char SERVIDOR_NTP[] = "pool.ntp.org";
bool tiempo_unix_configurado = false;
inline void inicializarTiempoUnix();
unsigned long obtenerTiempoUnix();
String mensajeSegundosATiempo(unsigned long segundos);


// Sensores.h
#define MUESTRAS_HUMEDAD_SUELO 16		// 16 máximo
void establecerSalidaMUX(uint8_t salida);
void inicializarSensores();
void leerSensores();
void leerAHT10Interiores();
void leerSoilInteriores();
void leerAHT10Exteriores();
void leerSoilExteriores();
// variables
// AHTs interiores
float temp_interior_promedio;
float humedad_aire_interior_promedio;
// AHTs exteriores
float temp_exterior;
float humedad_aire_exterior;
float temp_exterior_geotermica;
// soil moisture sensors
int humedad_suelo_interior;
int humedad_suelo_exterior;
class AHT10Mux
{
	public:
		AHT10Mux(uint8_t Asalida_del_mux);
		bool     begin();
		float    readTemperature(bool readI2C = AHT10_FORCE_READ_DATA);
		float    readHumidity(bool readI2C = AHT10_FORCE_READ_DATA);
	private:
		uint8_t salida_del_mux;
};


// Control.h
unsigned long ultima_vez_bomba_encendio = 0;
void chequearVentilacion();
void chequearRiego();
void activarVentilacion();
void desactivarVentilacion();
//void abrirVentana();
//void cerrarVentana();
//void chequear_iluminacion();
#define ANGULO_APERTURA	90		// posición de apertura de la ventana
#define ANGULO_CERRADO	0		// posición de cerrado de la ventana
// parte del botón (no utilizado, bueno tenerlo)
#define TIEMPO_MIN_BTN_MANTENIDO 800UL
enum class EstadoBoton : uint8_t
{
	Suelto,
	Clickeado,
	Mantenido,
	DobleClickeado
};
EstadoBoton leerBoton(unsigned long timeout_lectura);


// Display.h
unsigned long ultima_vez_display_actualizo = 0;
void inicializarDisplay();
void cambiarDatoDisplay();
void displayConectandoWiFi();
void displayErrorWiFi();
void displayConetadoA(String ssid_conectada);
void actualizarDisplay();
void displayHumedadAire();
void displayHumedadSuelo();
void displayTemperatura();
void displayError();
void displayNoSD();
void displayErrorSD();
// TODO:
void displayLogo(/*SE VALE PONER DELAY (1-2 seg)*/); // Invernadero inteligente que esté centrado, nada más


#define DELAY_CAMBIO_DISPLAY		10000UL
#define DELAY_ACTUALIZACION_DISPLAY	500UL
#define SCREEN_WIDTH				128		// ancho del display OLED display, en píxeles
#define SCREEN_HEIGHT				64		// alto del display OLED display, en píxeles
enum class DisplayDato : uint8_t
{
	Temperatura,
	HumedadAire,
	HumedadSuelo
};
DisplayDato DatoDelDisplay = DisplayDato::Temperatura;


// Graficos.h
unsigned long ultima_vez_thingspeak_actualizo = 0;
void actualizarGraficos();
inline bool inicializarThingSpeak();
#define FIELD_TEMP_INT		1
#define FIELD_TEMP_EXT		2
#define FIELD_HUM_AIRE_INT	3
#define FIELD_HUM_AIRE_EXT	4
#define FIELD_HUM_SUELO_INT	5
#define FIELD_HUM_SUELO_EXT	6


// Conectividad.h
#define CANT_REDES_WIFI 3
char w_ssid[CANT_REDES_WIFI][W_SSID_SIZE];
char w_pass[CANT_REDES_WIFI][W_PASS_SIZE];
void inicializarWiFi();
bool guardarRedWiFi(const char* ssid, const char* password);
bool guardarRedWiFi(const char* ssid);
bool conectarWiFi();


// Firebase.h



// SD_manejo.h
//char STRINGS_PATH[]			= "controlador/strings/";
char CONFIG_FOLDER_PATH[]		= "controlador/config/";
char WIFI_FOLDER_PATH[]			= "wifi/";
char FIREBASE_FOLDER_PATH[]		= "firebase/";
//char PARAMETROS_FOLDER_PATH[]	= "parametros/";
char NOMBRE_ARCHIVOS_WSSID[]	= "ssid";
char NOMBRE_ARCHIVOS_WPASS[]	= "ssid";
char TXT[]						= ".txt";
void inicializarSD();
void configWiFi();
void configFirebase();
void escribirDatos(String dato);
enum class ResultadoLecturaSD : uint8_t
{
	NO_ARCHIVO,
	NO_CONTENIDO,
	EXITOSO
};
ResultadoLecturaSD leerArchivoSDA(char *buffer, const uint8_t caracteres, const char *path);


// Telegram.h
// variables
#define TELEGRAM_TIEMPO_MAX_CONFIGURACION	15000UL
unsigned long ultima_vez_alarma_funciono = 0;
unsigned long ultima_vez_comprobacion_wifi = 0;
String		chat_rpta; // necesariamente global para cambiarla en evaluarMensajeFloat() y evaluarMensajeInt()
uint64_t 	chat_id = 0; // comienza en 0 para comprobaciones en chequearAlarma()
uint16_t 	chat_numero_entrada_int;	// cuando preguntamos por un número entero de entrada
float		chat_numero_entrada_float;	// cuando preguntamos por un número con decimal de entrada
bool		chat_primer_mensaje = true; // para chequearMensajesRecibidosTelegram()
// WiFi
void conectarWiFi(bool parar_programa);
bool conectarWiFiCon(const String& Assid, const String& Apassword);
void chequearConexion();
// funciones varias
void chequearMensajesRecibidosTelegram();
void chequearAlarma();
void enviarMensaje(const uint64_t Aid, const String& Amensaje);
bool evaluarMensajeInt(uint16_t Avalor_min, uint16_t Avalor_max, String Aunidad);
bool evaluarMensajeFloat(float Avalor_min, float Avalor_max, String Aunidad);
// comandos
void comandoStart();
void comandoLecturas();
void comandoInfo();
void comandoProg();
void comandoVentilar();
void comandoTiempoAl();
void comandoTiempoRiego();
void comandoTiempoEspera();
void comandoTmax();
void comandoTmin();
void comandoTvent();
void comandoAlarma();
void comandoHum();
void comandoLed();
void comandoReprog();


// EEMPROM_manejo.h
// funciones
void chequearEEPROMProgramada();
void setDireccionesEEPROM();
void leerEEPROMProgramada();
void cargarValoresPorDefecto();
void imprimirEEPROMValsDirsReads();
template <typename T>
void escribirEEPROM(int Adireccion, T Adato); // template <typename T> T leerEEPROM(int Adireccion, T Adato);
// variables de la EEPROM con sus valores por defecto
#define TEMP_MAXIMA_ALARMA_DEFECTO		45.0F
#define TEMP_MINIMA_ALARMA_DEFECTO		-5.0F
#define TEMP_MAXIMA_VENTILACION_DEFECTO	35.0F
#define HUMEDAD_SUELO_MINIMA_DEFECTO	60
#define LAPSO_ALARMA_MINUTOS_DEFECTO	60
#define ALARMA_ACTIVADA_DEFECTO			true
#define TIEMPO_BOMBEO_SEGUNDOS_DEFECTO	10
#define TIEMPO_ESPERA_MINUTOS_DEFECTO	15
bool		eeprom_programada;			// 0.	para verificar si está programada o no la EEPROM
float		temp_maxima_alarma;			// 1.
float		temp_minima_alarma;			// 2.
float		temp_maxima_ventilacion;	// 3.
uint8_t		humedad_suelo_minima;		// 4.	70 % es vaso de agua, 29 % es el aire
uint16_t	lapso_alarma_minutos;		// 5.	60 minutos (máx 65535 min o 1092 horas o 45 días)
bool		alarma_activada;			// 6.
uint16_t	tiempo_bombeo_segundos;		// 7.	4 segundos (máx 65535 seg o 18,2 horas)
uint16_t	tiempo_espera_minutos;		// 8.	15 minutos (máx 65535 min)
// manejo de las direcciones de la EEPROM
enum EEPROMDireccionesVariables : uint8_t
{
	DIR_EEPROM_PROGRAMADA,
	DIR_TEMP_MAXIMA_ALARMA,
	DIR_TEMP_MINIMA_ALARMA,
	DIR_TEMP_MAXIMA_VENTILACION,
	DIR_HUMEDAD_SUELO_MINIMA,
	DIR_LAPSO_ALARMA_MINUTOS,
	DIR_ALARMA_ACTIVADA,
	DIR_TIEMPO_BOMBEO_SEGUNDOS,
	DIR_TIEMPO_ESPERA_MINUTOS,
	CANT_VARIABLES_EEPROM
};
										// bool, float, float, float, int8, int, bool, int, int
const int LONGITUD_DATO_EEPROM[CANT_VARIABLES_EEPROM] = {1, 4, 4, 4, 1, 2, 1, 2, 2};
int direccion[CANT_VARIABLES_EEPROM];
int espacios_EEPROM;


// Clases
CTBot Bot;
File DatalogSD;
WiFiMulti WiFiMultiO;
Adafruit_SSD1306 Display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
AHT10 AhtSeleccionado(AHT10_ADDRESS_0X38);
AHT10Mux AhtInteriorHigh(AHT_INT_HIGH_MUX_PIN);
AHT10Mux AhtInteriorMid(AHT_INT_MID_MUX_PIN);
AHT10Mux AhtInteriorLow(AHT_INT_LOW_MUX_PIN);
AHT10Mux AhtAgua1(AHT_AGUA_1_MUX_PIN);
AHT10Mux AhtAgua2(AHT_AGUA_2_MUX_PIN);
AHT10Mux AhtAgua3(AHT_AGUA_3_MUX_PIN);
AHT10Mux AhtExterior(AHT_EXT_MUX_PIN);
AHT10Mux AhtExteriorGeotermico(AHT_GEOTERMICO_MUX_PIN);