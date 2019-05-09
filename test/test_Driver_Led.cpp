/*
 * test_Driver_Led.cpp
 *
 *	Test unitario para el módulo Driver_Led
 */



//------------------------------------------------------------------------------------
//-- TEST HEADERS --------------------------------------------------------------------
//------------------------------------------------------------------------------------

#include "unity.h"
#include "Led.h"


//------------------------------------------------------------------------------------
//-- SPECIFIC COMPONENTS FOR TESTING -------------------------------------------------
//------------------------------------------------------------------------------------

static const char* _MODULE_ = "[TEST_LED].......";
#define _EXPR_	(true)


//------------------------------------------------------------------------------------
//-- REQUIRED HEADERS & COMPONENTS FOR TESTING ---------------------------------------
//------------------------------------------------------------------------------------


/** interfaz para ejecutar requisitos iniciales */
static void executePrerequisites(esp_log_level_t level);
static Led* led = NULL;



//------------------------------------------------------------------------------------
//-- TEST CASES ----------------------------------------------------------------------
//------------------------------------------------------------------------------------


//---------------------------------------------------------------------------
TEST_CASE("Test ON...............................", "[Driver_Led]") {
	// ejecuta requisitos de test
	executePrerequisites(ESP_LOG_DEBUG);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Driver_Led... OK!");
	led->on();
}


//---------------------------------------------------------------------------
TEST_CASE("Test OFF...............................", "[Driver_Led]") {
	// ejecuta requisitos de test
	executePrerequisites(ESP_LOG_DEBUG);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Driver_Led... OK!");
	led->off();
}


//---------------------------------------------------------------------------
TEST_CASE("Test one-shot during 10 sec............", "[Driver_Led]") {
	// ejecuta requisitos de test
	executePrerequisites(ESP_LOG_DEBUG);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Starting one-shot");
	uint32_t one_shot[] = {250, 1000};
	TEST_ASSERT_EQUAL(led->setBlinkMode(one_shot, 2), 0);
	Thread::wait(10000);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Cancelling blink mode");
	led->cancelBlinkMode();
}


//---------------------------------------------------------------------------
TEST_CASE("Test two-shots during 10 sec............", "[Driver_Led]") {
	// ejecuta requisitos de test
	executePrerequisites(ESP_LOG_DEBUG);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Driver_Led... OK!");
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Starting two-shot");
	uint32_t two_shots[] = {250, 250, 250, 1000};
	TEST_ASSERT_EQUAL(led->setBlinkMode(two_shots, 4), 0);
	Thread::wait(10000);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Cancelling blink mode");
	led->cancelBlinkMode();
}


//---------------------------------------------------------------------------
TEST_CASE("Test three-shots during 10 sec............", "[Driver_Led]") {
	// ejecuta requisitos de test
	executePrerequisites(ESP_LOG_DEBUG);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Driver_Led... OK!");
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Starting three-shot");
	uint32_t three_shots[] = {250, 250, 250, 250, 250, 1000};
	TEST_ASSERT_EQUAL(led->setBlinkMode(three_shots, 6), 0);
	Thread::wait(10000);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Cancelling blink mode");
	led->cancelBlinkMode();
}


//------------------------------------------------------------------------------------
//-- PREREQUISITES -------------------------------------------------------------------
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
static void executePrerequisites(esp_log_level_t level){
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "  =================");
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "||  Starting test  ||");
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "  =================\r\n");

	// ajusta el nivel de depuración
	esp_log_level_set(_MODULE_, level);

	led = new Led(GPIO_NUM_18, Led::LedOnOffType, Led::OnIsLowLevel);
	TEST_ASSERT_NOT_NULL(led);

}
