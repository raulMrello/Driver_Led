/*
 * test_Driver_Led.cpp
 *
 *	Test unitario para el módulo Driver_Led
 */



//------------------------------------------------------------------------------------
//-- TEST HEADERS --------------------------------------------------------------------
//------------------------------------------------------------------------------------

#include "Appconfig.h"

#if ESP_PLATFORM == 1 || (__MBED__ == 1 && defined(ENABLE_TEST_DEBUGGING) && defined(ENABLE_TEST_Driver_Led))
#include "unity.h"
#include "mbed.h"
#include "Heap.h"

/** Requerido para test unitarios ESP-MDF */
#if ESP_PLATFORM == 1
void (*syslog_print)(const char*level, const char* tag, const char* format, ...) = NULL;
#define PinName_TX	GPIO_NUM_13
#define PinName_RX	GPIO_NUM_12
#define PinName_LED_RED_REMOTE		(PinName)8 	//PA_8
#define PinName_LED_GREEN_REMOTE	(PinName)9 	//PA_9
#define PinName_LED_WHITE_REMOTE	(PinName)10 //PA_10
const PinName LedArray[] =	{PinName_LED_RED_REMOTE, PinName_LED_GREEN_REMOTE, PinName_LED_WHITE_REMOTE};

/** Requerido para test unitarios STM32 */
#elif __MBED__ == 1 && defined(ENABLE_TEST_DEBUGGING) && defined(ENABLE_TEST_Driver_Led)
#include "unity_test_runner.h"
/// Configuración MBED_API_uSerial
#define PinName_TX	PC_10
#define PinName_RX	PC_11

// Configuración leds
#define PinName_LED_RED_LOCAL		PA_8
#define PinName_LED_GREEN_LOCAL		PA_9
#define PinName_LED_WHITE_LOCAL		PA_10
const PinName LedArray[] = {PinName_LED_RED_LOCAL, PinName_LED_GREEN_LOCAL, PinName_LED_WHITE_LOCAL};
#endif

#include "Led.h"
#include "mbed_api_userial.h"
#include "SerialMon.h"

#define LED_COUNT				3

#define MAX_USERIAL_MSG_SIZE	128
#define TX_BUFFER_SIZE			4 * MAX_USERIAL_MSG_SIZE
#define RX_BUFFER_SIZE			4 * MAX_USERIAL_MSG_SIZE
#define SERIAL_BAUDRATE			115200



//------------------------------------------------------------------------------------
//-- SPECIFIC COMPONENTS FOR TESTING -------------------------------------------------
//------------------------------------------------------------------------------------

static const char* _MODULE_ = "[TEST_LED].......";
#define _EXPR_	(true)


//------------------------------------------------------------------------------------
//-- REQUIRED HEADERS & COMPONENTS FOR TESTING ---------------------------------------
//------------------------------------------------------------------------------------

/** Leds a verificar */
static Led* led[LED_COUNT];

/** Puerto serie para pruebas con MBED_API_uSerial */
static SerialMon* g_serial = NULL;

/** Librería userial */
static MBED_API_uSerial mbed_api_userial;



//------------------------------------------------------------------------------------
//-- TEST FUNCTIONS ------------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
static void test_led_init_userial(){
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Inicializando Puerto serie");
	#if ESP_PLATFORM==1
	g_serial = new SerialMon(PinName_TX, PinName_RX, TX_BUFFER_SIZE, RX_BUFFER_SIZE, SERIAL_BAUDRATE, "uSerial", UART_NUM_1);
	#elif __MBED__ == 1
	g_serial = new SerialMon(PinName_TX, PinName_RX, TX_BUFFER_SIZE, RX_BUFFER_SIZE, SERIAL_BAUDRATE, "uSerial");
	#endif
	TEST_ASSERT_NOT_NULL(g_serial);
	g_serial->setLoggingLevel(ESP_LOG_WARN);
	g_serial->cfgStreamAnalyzer(MBED_API_uSerial::ProtocolHeaderValue,
								MBED_API_uSerial::ProtocolFooterValue,
								1,
								sizeof(uint16_t),
								true,
								RX_BUFFER_SIZE);
	g_serial->start(osPriorityNormal, OS_STACK_SIZE);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Esperando a que SerialMon esté operativo");
	while(!g_serial->isReady()){
		Thread::wait(100);
	}
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "SerialMon iniciada!!!");

	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Inicializando MBED_API_uSerial");

	#if ESP_PLATFORM==1
	int result = mbed_api_userial.init(static_cast<ISerial*>(g_serial), MAX_USERIAL_MSG_SIZE, CPU_ONBOARD_ESP32);
	#elif __MBED__ == 1
	int result = mbed_api_userial.init(static_cast<ISerial*>(g_serial), MAX_USERIAL_MSG_SIZE, CPU_ONBOARD_STM32);
	#endif
	TEST_ASSERT_EQUAL(result, 0);

	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Esperando a que MBED_API_uSerial esté operativa");
	while(!mbed_api_userial.ready()){
		Thread::wait(100);
	}
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "MBED_API_uSerial iniciada!!!");
	Thread::wait(100);
}


//------------------------------------------------------------------------------------
static void test_led_new_locals(){
	for(int i=0;i<LED_COUNT;i++){
		led[i] = new Led(LedArray[i], Led::LedOnOffType, Led::OnIsLowLevel, MBED_API_uSerial::getCPU());
		TEST_ASSERT_NOT_NULL(led[i]);
		led[i]->off();
	}
}


//------------------------------------------------------------------------------------
static void test_led_on(){
	for(int i=0;i<LED_COUNT;i++){
		led[i]->on();
	}
}


//------------------------------------------------------------------------------------
static void test_led_off(){
	for(int i=0;i<LED_COUNT;i++){
		led[i]->off();
	}
}


//------------------------------------------------------------------------------------
static void test_led_blink(){
	const uint32_t blink_sequence[] = {250, 250};
	for(int i=0;i<LED_COUNT;i++){
		TEST_ASSERT_EQUAL(led[i]->setBlinkMode(blink_sequence, 2), 0);
	}
}


//------------------------------------------------------------------------------------
static void test_led_destroy(){
	for(int i=0;i<LED_COUNT;i++){
		delete(led[i]);
	}
}

//------------------------------------------------------------------------------------
//-- TEST CASES ----------------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
TEST_CASE("Inicializacion MBED API uSerial", "[Driver_Led]") {
	test_led_init_userial();
}


//------------------------------------------------------------------------------------
TEST_CASE("Crea leds en CPU local", "[Driver_Led]") {
	test_led_new_locals();
}


//------------------------------------------------------------------------------------
TEST_CASE("Enciende los leds", "[Driver_Led]") {
	test_led_on();
}


//------------------------------------------------------------------------------------
TEST_CASE("Apaga los leds", "[Driver_Led]") {
	test_led_off();
}


//------------------------------------------------------------------------------------
TEST_CASE("Parpadea los leds", "[Driver_Led]") {
	test_led_blink();
}


//------------------------------------------------------------------------------------
TEST_CASE("Destruye los leds", "[Driver_Led]") {
	test_led_destroy();
}



//---------------------------------------------------------------------------
TEST_CASE("Test one-shot during 10 sec............", "[Driver_Led]") {
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Starting one-shot");
	uint32_t one_shot[] = {250, 1000};
	for(int i=0;i<LED_COUNT;i++){
		TEST_ASSERT_EQUAL(led[i]->setBlinkMode(one_shot, 2), 0);
	}
	Thread::wait(10000);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Cancelling blink mode");
	for(int i=0;i<LED_COUNT;i++){
		led[i]->cancelBlinkMode();
	}
}


//---------------------------------------------------------------------------
TEST_CASE("Test two-shots during 10 sec............", "[Driver_Led]") {
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Starting two-shot");
	uint32_t two_shots[] = {250, 250, 250, 1000};
	for(int i=0;i<LED_COUNT;i++){
		TEST_ASSERT_EQUAL(led[i]->setBlinkMode(two_shots, 4), 0);
	}
	Thread::wait(10000);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Cancelling blink mode");
	for(int i=0;i<LED_COUNT;i++){
		led[i]->cancelBlinkMode();
	}
}


//---------------------------------------------------------------------------
TEST_CASE("Test three-shots during 10 sec............", "[Driver_Led]") {
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Starting three-shot");
	uint32_t three_shots[] = {250, 250, 250, 250, 250, 1000};
	for(int i=0;i<LED_COUNT;i++){
		TEST_ASSERT_EQUAL(led[i]->setBlinkMode(three_shots, 6), 0);
	}
	Thread::wait(10000);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Cancelling blink mode");
	for(int i=0;i<LED_COUNT;i++){
		led[i]->cancelBlinkMode();
	}
}



//------------------------------------------------------------------------------------
//-- TEST ENRY POINT -----------------------------------------------------------------
//------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void firmwareStart(){
	esp_log_level_set(_MODULE_, ESP_LOG_DEBUG);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Inicio del programa");
	Heap::setDebugLevel(ESP_LOG_DEBUG);
	unity_run_menu();
}


#endif
