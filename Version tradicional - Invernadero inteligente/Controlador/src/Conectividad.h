#pragma once

#include "Declaraciones.h"

// Devuelve verdadero si se obtuvieron datos de la aplicación, falso si no.
bool configInicial(bool ignorar_config_inicial = false)
{
	if (tiene_config_inicial  &&  !ignorar_config_inicial)
		return true;

	int8_t intentos_bluetooth = 0;
	BTSerial.begin(BLUETOOTH_NOMBRE);
	//Serial.println("Bluetooth encendido");

reintentar:
	++intentos_bluetooth;
    if (intentos_bluetooth == 10)
	{
		// TODO: mensaje display
		BTSerial.end();
		return false;
	}

	unsigned long tiempo_0_bluetooth = millis();
	byte byte_recibido = BLUETOOTH_TEST_BYTE;
	do
	{
		displayEsperando(intentos_bluetooth);
		if (millis() - tiempo_0_bluetooth >= BLUETOOTH_TIEMPO_MAX_CONFIG)
		{
			//Serial.println("Pasó demasiado tiempo");
			// TODO: mensaje display
			BTSerial.end();
			tiene_wifi = false;
			return false;
		}
		if (BTSerial.available() > 0)
			byte_recibido = BTSerial.read();
	} while (byte_recibido == BLUETOOTH_TEST_BYTE);

	//Serial.println("Mensaje distinto del byte de prueba");
	if(!decodificarMensaje(byte_recibido))
	{
		limpiarBufferBluetooth();
		//Serial.println("no es ninguno, volviendo a llamar");
		// TODO IMPORTANTE: MOSTRAR EN EL DISPLAY QUE HAY QUE REPETIR LA CONFIGURACIÓN
		goto reintentar;
	}

	BTSerial.end();
	return true; // El mensaje exitoso del display lo ponen las funciones de decodificación
}

//==================================================================================================================//

// Devuelve verdadero si se decodificó qué tipo de mensaje envió la aplicación. Falso si no.
bool decodificarMensaje(byte primer_byte)
{
	//Serial.println(primer_byte);
	BTSerial.read(); // deshacernos del \n

	if (primer_byte == BLUETOOTH_PRIMER_BYTE_SIN_WIFI)
		decodificarSinWiFi();
	else if (primer_byte == BLUETOOTH_PRIMER_BYTE_CON_WIFI)
		decodificarConWiFi();
	else
		return false; // No se identificó el tipo de dato

	tiene_config_inicial = true;
	escribirEEPROM(direccion[DIR_TIENE_CONFIG_INICIAL], tiene_config_inicial);
	return true;
}

//==================================================================================================================//

void decodificarSinWiFi()
{
	//Serial.println("config sin wifi");
	tiene_wifi = false;
	escribirEEPROM(direccion[DIR_TIENE_WIFI], tiene_wifi);
	// TODO: mensaje display
}

//==================================================================================================================//

void decodificarConWiFi()
{
	char ssid[32];
	char password_wifi[63];

	// BTSerial.readBytesUntil no funcionaba :S
	leerBTSerialHasta('\n', ssid, sizeof(ssid));
	leerBTSerialHasta('\n', password_wifi, sizeof(password_wifi));

	//Serial.print("SSID: ");
	//Serial.println(ssid);
	//Serial.print("PASS: ");
	//Serial.println(password_wifi);
	// TODO: mensaje display de las credenciales añadidas
	tiene_wifi = true;
	escribirEEPROM(direccion[DIR_TIENE_WIFI], tiene_wifi);
	guardarRedWiFi(ssid, password_wifi);
}

//==================================================================================================================//

void leerBTSerialHasta(char terminador, char* array, size_t longitud)
{
	char incom_char;
	for (size_t i = 0; i < longitud; ++i)
	{
		incom_char = BTSerial.read();
		if (incom_char != terminador)
			array[i] = incom_char;
		else
		{
			array[i] = '\0';
			return;
		}
	}

}

//==================================================================================================================//

void limpiarBufferBluetooth()
{
	while(BTSerial.available() > 0)
		BTSerial.read();
}

//==================================================================================================================//

void displayEsperando(int8_t Aintentos_bluetooth)
{
	bool conectado = BTSerial.hasClient();
	if (Aintentos_bluetooth > 1)
		displayReintentarBT(conectado);
	else
		displayConfigInicialBT(conectado);
}



//============================================PARTE DE CONEXIONES A WiFi============================================//

// Enviar "NULL" si es WiFi público
void guardarRedWiFi(const char* ssid, const char* password_wifi)
{
	if (password_wifi == "NULL")
		WiFiMultiO.addAP(ssid);
	else
		WiFiMultiO.addAP(ssid, password_wifi);
	// TODO: y acá hacemos lo de JSON
}

//==================================================================================================================//

// Si no tiene WiFi, pregunta si quiere agregar. Si tiene y no se conecta, pregunta si quiere cambiar/agregar credenciales
void inicializarWiFi()
{
	if (!tiene_wifi)
	{
		if ( !quiereAgregarCredenciales() )
			return;

		if ( !configInicial(true) )
			return;
	}

reintentar:
	displayConectandoWiFi();
	WiFi.mode(WIFI_STA);
	if(WiFiMultiO.run() != WL_CONNECTED)
	{
		if ( !quiereCambiarCredenciales() )
			return;

		if ( !configInicial(true) )
			return;
		goto reintentar;
	}

	displayConetadoA( WiFi.SSID() );
}

//==================================================================================================================//

bool quiereAgregarCredenciales()
{
	// ¿Desea agregar una red WiFi?
	if (leerBoton(TIEMPO_MAX_ESPERA_BTN) == EstadoBoton::Mantenido)
		return true;
	return false;
}

bool quiereCambiarCredenciales()
{
	// TODO: mostrar en el display No me pude conectar a WiFi. ¿Cambiar credenciales?
	if (leerBoton(TIEMPO_MAX_ESPERA_BTN) == EstadoBoton::Mantenido)
		return true;
	return false;
}


/*
Función conectarWiFi():
Queremos que inicialice bluetooth por un tiempo definido (2 minutos) y que espere a que le lleguen serial strings desde la app.
Necesitamos SSID, PASS, username y pass de firebase (usar wifiMulti.addAP()). Usaremos WiFiMulti Tutorial de la App:

https://youtu.be/l10NGNCGUBc (en vez de botones poner write boxes y enviar lo que se escriba)

Tutorial WiFiMulti:

https://randomnerdtutorials.com/esp32-wifimulti/

Y una vez lleguen los datos guardarlos en la memoria flash del controlador. Tutorial:

https://youtu.be/VnfX9YJbaU8?t=2380 (sí, que el código también esté en este archivo)
Si no sirve bien este tuto (mirarlo después de lograr la comunicación por bluetooth, buscar "save data in spiffs esp32" en yt)


También queremos definir chequearConexion(). Lo bueno es que con WiFiMulti es una sóla línea "if(wifiMulti.run() == WL_CONNECTED)"

*/