/*
 * test_Driver_Led.cpp
 *
 *	Test unitario para el m�dulo Driver_Led
 */



//------------------------------------------------------------------------------------
//-- TEST HEADERS --------------------------------------------------------------------
//------------------------------------------------------------------------------------

#include "mbed.h"
#include "AppConfig.h"
#include "unity.h"
#include "Heap.h"
#include "Led.h"
#include "xLed.h"
#include "mbed_api_userial.h"

#if ESP_PLATFORM == 1 || (__MBED__ == 1 && defined(ENABLE_TEST_DEBUGGING) && defined(ENABLE_TEST_Driver_Led))

/** Requerido para test unitarios ESP-MDF */
#if ESP_PLATFORM == 1
void (*syslog_print)(const char*level, const char* tag, const char* format, ...) = NULL;

#define PinName32_LED_RED_REMOTE		(PinName32)8 	//PA_8
#define PinName32_LED_GREEN_REMOTE	(PinName32)9 	//PA_9
#define PinName32_LED_WHITE_REMOTE	(PinName32)10 //PA_10
const PinName32 LedArray[] =	{PinName32_LED_RED_REMOTE, PinName32_LED_GREEN_REMOTE, PinName32_LED_WHITE_REMOTE};

/** Requerido para test unitarios STM32 */
#elif __MBED__ == 1 && defined(ENABLE_TEST_DEBUGGING) && defined(ENABLE_TEST_Driver_Led)
#include "unity_test_runner.h"
/// Configuraci�n MBED_API_uSerial

// Configuraci�n leds
#define PinName32_LED_RED_LOCAL		PA_8
#define PinName32_LED_GREEN_LOCAL		PA_9
#define PinName32_LED_WHITE_LOCAL		PA_10
const PinName32 LedArray[] = {PinName32_LED_RED_LOCAL, PinName32_LED_GREEN_LOCAL, PinName32_LED_WHITE_LOCAL};
#endif

#define LED_COUNT				3


//------------------------------------------------------------------------------------
//-- SPECIFIC COMPONENTS FOR TESTING -------------------------------------------------
//------------------------------------------------------------------------------------

static const char* _MODULE_ = "[TEST_LED].......";
#define _EXPR_	(true)


//------------------------------------------------------------------------------------
//-- REQUIRED HEADERS & COMPONENTS FOR TESTING ---------------------------------------
//------------------------------------------------------------------------------------

/** Leds locales  a verificar */
static Led* led[LED_COUNT];
bool are_locals = false;

/** Leds remotos a verificar */
static xLed* xled[LED_COUNT];
bool are_remotes = false;

/** Librer�a userial */
static MBED_API_uSerial mbed_api_userial;



//------------------------------------------------------------------------------------
//-- TEST FUNCTIONS ------------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
static void test_led_init_userial(){
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Inicializando MBED_API_uSerial");

	#if ESP_PLATFORM==1
	int result = mbed_api_userial.init(uSerial_CPU::CPU_ONBOARD_ESP32);
	#elif __MBED__ == 1
	int result = mbed_api_userial.init(uSerial_CPU::CPU_ONBOARD_STM32);
	#endif
	TEST_ASSERT_EQUAL(result, 0);

	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Esperando a que MBED_API_uSerial est� operativa");
	while(!mbed_api_userial.ready()){
		Thread::wait(100);
	}
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "MBED_API_uSerial iniciada!!!");
	Thread::wait(100);
	mbed_api_userial.setLoggingLevel(ESP_LOG_WARN);
	are_locals = false;
	are_remotes = false;
}


//------------------------------------------------------------------------------------
static void test_led_new_locals(){
	for(int i=0;i<LED_COUNT;i++){
		led[i] = new Led(LedArray[i], Led::LedOnOffType, Led::OnIsHighLevel, 0);
		TEST_ASSERT_NOT_NULL(led[i]);
		led[i]->off();
	}
	are_locals = true;
}



//------------------------------------------------------------------------------------
static void test_led_new_remotes(){
	uSerial_CPU remote_cpu = (MBED_API_uSerial::getCPU()==uSerial_CPU::CPU_ONBOARD_ESP32)? uSerial_CPU::CPU_ONBOARD_STM32 : uSerial_CPU::CPU_ONBOARD_ESP32;
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "CPU local=%d, CPU_remota=%d", (uint32_t)MBED_API_uSerial::getCPU(), (uint32_t)remote_cpu);
	for(int i=0;i<LED_COUNT;i++){
		xled[i] = new xLed(LedArray[i], Led::LedOnOffType, Led::OnIsHighLevel, 0, remote_cpu);
		TEST_ASSERT_NOT_NULL(xled[i]);
	}
	are_remotes = true;
}


//------------------------------------------------------------------------------------
static void test_led_on(){
	for(int i=0;i<LED_COUNT;i++){
		if(are_locals){
			led[i]->on();
		}
		if(are_remotes){
			xled[i]->on();
		}
	}
}


//------------------------------------------------------------------------------------
static void test_led_off(){
	for(int i=0;i<LED_COUNT;i++){
		if(are_locals)
			led[i]->off();
		if(are_remotes){
			xled[i]->off();
		}
	}
}


//------------------------------------------------------------------------------------
static void test_led_blink(){
	const uint32_t blink_sequence[] = {250, 250};
	for(int i=0;i<LED_COUNT;i++){
		if(are_locals){
			TEST_ASSERT_EQUAL(led[i]->setBlinkMode(blink_sequence, 2), 0);
		}
		if(are_remotes){
			TEST_ASSERT_EQUAL(xled[i]->setBlinkMode(blink_sequence, 2), 0);
		}
	}
}


//------------------------------------------------------------------------------------
static void test_led_destroy(){
	for(int i=0;i<LED_COUNT;i++){
		if(are_locals){
			delete(led[i]);
		}
		if(are_remotes){
			delete(xled[i]);
		}
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
TEST_CASE("Crea leds y sus pines en CPU local", "[Driver_Led]") {
	test_led_new_locals();
}


//------------------------------------------------------------------------------------
TEST_CASE("Crea leds en CPU remota", "[Driver_Led]") {
	test_led_new_remotes();
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




//------------------------------------------------------------------------------------
//-- TEST ENRY POINT -----------------------------------------------------------------
//------------------------------------------------------------------------------------


#if __MBED__ == 1 && defined(ENABLE_TEST_DEBUGGING) && defined(ENABLE_TEST_Driver_Led)
void firmwareStart(){
	esp_log_level_set(_MODULE_, ESP_LOG_DEBUG);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Inicio del programa");
	Heap::setDebugLevel(ESP_LOG_DEBUG);
	unity_run_menu();
}
#endif

#endif
