/*------------------------------------------------------------------------------------------------------------------*\
	Nombre:				Version hidroponía - Invernadero Inteligente
	Desarrollo: 		octubre 2022 - diciembre 2023
	Primer lanzamiento: -
	Creado por:			Quintana Santiago Esteban
\*------------------------------------------------------------------------------------------------------------------*/

#include "Conectividad.h"
#include "Control.h"
#include "Declaraciones.h" // contiene <Arduino.h> y todas las librerías
#include "Display.h"
#include "EEPROM_manejo.h"
#include "Firebase.h"
#include "SD_manejo.h"
#include "Sensores.h"
#include "Tiempo.h"

void setup()
{
	// inicializaciones varias
	#ifdef DEBUG_SERIAL
		Serial.begin(115200);
	#endif
	for (PinsOut pin : {PinsOut::BOMBA1, PinsOut::BOMBA2, PinsOut::BOMBA3,
						PinsOut::MARCHA, PinsOut::CONTRAMARCHA,
						PinsOut::MUX_A, PinsOut::MUX_B,	 PinsOut:: MUX_C})
	{
		digitalWrite(static_cast<uint8_t>(pin), LOW);
		pinMode(static_cast<uint8_t>(pin), OUTPUT);
  	}

	// inicializar display
	LCDP.inicializar();
	LCDP.displayLogo();

	// inicializar sensores y SD
	inicializarSensores();
	LCSD.inicializar();

	// leer los archivos de configuración de la tarjeta SD
	LCSD.leerConfigWiFi();
	LCSD.leerConfigFirebase();

	LCEE.inicializar();

	LCCT.configurarModosSalidas();

	delay(3500);

	// Conectarse a WiFi
	imprimirln("Conectando a WiFi...");
	LCWF.inicializarWiFi();
	imprimirln("Setup finalizado!");
	delay(2000);
}

//==============================================aquí se encuentra la función loop()==============================================//

void loop()
{
	if (millis() - ultima_vez_invernadero_funciono >= DELAY_ACTIVIDAD_INVERNADERO)
	{ // idealmente, en lugar de esperas pondríamos al uC en un estado de bajo consumo por un período fijo
		ultima_vez_invernadero_funciono = millis();

		// Leer sensores
		leerSensores();
		
		// Manejar conexiones y comunicaciones
		LCWF.correr();
		LCCT.controlarAlarma();
		LCFB.correr();

		// Datalog
		//LCFB.datalog(); ahorramos líneas de código llamandola desde LCSD.datalog y pasando los datos como args
		LCSD.datalog();

		// Tomar decisiones
		LCCT.controlarBomba1();
		LCCT.controlarBomba2();
		LCCT.controlarBomba3();
		LCCT.controlarVentilacion();

		// Actualizar datos mostrables
		LCDP.actualizar();
	}

	// cambiamos el contenido de la pantalla
	if (millis() - LCDP.ultima_vez_cambio >= DELAY_CAMBIO_DISPLAY)
	{
		LCDP.ultima_vez_cambio = millis();
		LCDP.cambiarDato();
	}
}