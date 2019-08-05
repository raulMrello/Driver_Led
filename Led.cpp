/*
 * Led.cpp
 *
 *  Created on: 20/04/2015
 *      Author: raulMrello
 */

#include "Led.h"


//------------------------------------------------------------------------------------
//--- PRIVATE TYPES ------------------------------------------------------------------
//------------------------------------------------------------------------------------


#define NULL_CALLBACK               (void(*)())0
 
//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
Led::Led(PinName led, LedType type, LedLogicLevel level, uint32_t period_ms, uSerial_CPU cpu){
	// Crea objeto
    _id = (uint32_t)led;
    _debug = false;
    _type = type;
    _period_ms = period_ms;
    _max_intensity = 1.0f;
    _min_intensity = 0;
    
    // desactiva el modo de parpadeo
    _num_blinks = 0;
    for(uint8_t i=0;i<MaxBlinkCount;i++){
    	_blinks[i] = 0;
    }
    if(_type == LedOnOffType){
        _out_01 = new xDigitalOut(led, cpu);
    }
    else{
        _out = new xPwmOut(led, cpu);
        _out->period_ms(_period_ms);
    }    
    _level = level;      
    
    // Deja apagado por defecto
    off(0, 0, 0);
}


//------------------------------------------------------------------------------------
Led::~Led(){
	off();
	_tick_blink.detach();
	_tick_ramp.detach();
	_tick_duration.detach();
	if(_type == LedOnOffType){
		delete(_out_01);
	}
	else{
		delete(_out);
	}
}


//------------------------------------------------------------------------------------
void Led::on(uint32_t ms_duration, uint8_t intensity, uint32_t ms_ramp){
	if(_stat == LedIsBlinking){
		_tick_blink.detach();
	}
	if(_type == LedDimmableType){
		_tick_ramp.detach();
	}
	if(_istemp){
		_tick_duration.detach();
	}
    _istemp = false;
    // si es temporal...
    if(ms_duration > 0){
        // hace un backup del estado actual y lanza el timer
        _istemp = true;
        _bkp_stat = _stat;
        _tick_duration.attach_us(callback(this, &Led::temporalCb), (ms_duration * 1000));
    }
    _stat = LedIsOn;
    // Si no hay rampa...
    if(ms_ramp == 0){
        _action = LedGoOnEnd;
        if(_type == LedOnOffType){
           	uint8_t value  = (intensity != 0)? 1 : 0;
			value = (_level == OnIsHighLevel)? value : (1 - value);
			_max_intensity = value;
			_intensity = value;
			_out_01->write(value);
        }
        else{
            _max_intensity = convertIntensity(intensity);
            _intensity = _max_intensity;
            _out->write(_intensity);        
        }
    }
    // si hay rampa, la inicia
    else{
        _action = LedGoingOn;
        _tick_ramp.attach_us(callback(this, &Led::rampOnCb), (ms_ramp * 1000));
    }    
}


//------------------------------------------------------------------------------------
void Led::off(uint32_t ms_duration, uint8_t intensity, uint32_t ms_ramp){
	if(_stat == LedIsBlinking){
		_tick_blink.detach();
	}
	if(_type == LedDimmableType){
		_tick_ramp.detach();
	}
	if(_istemp){
		_tick_duration.detach();
	}
    _istemp = false;
    // si es temporal...
    if(ms_duration > 0){
        // hace un backup del estado actual y lanza el timer
        _istemp = true;
        _bkp_stat = _stat;
        _tick_duration.attach_us(callback(this, &Led::temporalCb), (ms_duration * 1000));
    }
    _stat = LedIsOff;
    // Si no hay rampa...
    if(ms_ramp == 0){        
        _action = LedGoOffEnd;
        if(_type == LedOnOffType){
        	uint8_t value  = (intensity != 0)? 1 : 0;
        	value = (_level == OnIsHighLevel)? value : (1 - value);
        	_min_intensity = value;
        	_intensity = value;
            _out_01->write(value);
        }
        else{
            _min_intensity = convertIntensity(intensity);
            _intensity = _min_intensity;
            _out->write(_intensity);        
        }
    }
    // si hay rampa, la inicia
    else{
        _action = LedGoingOff;
        _tick_ramp.attach_us(callback(this, &Led::rampOffCb), (ms_ramp * 1000));
    }    
}


//------------------------------------------------------------------------------------
void Led::blink(uint32_t ms_blink_on, uint32_t ms_blink_off, uint32_t ms_duration, uint8_t intensity_on, uint8_t intensity_off){
    // si no hay temporizaciones de On y Off, no permite la ejecución
    if(ms_blink_on == 0 && ms_blink_off == 0){
        return;
    }
	if(_stat == LedIsBlinking){
		_tick_blink.detach();
	}
	if(_type == LedDimmableType){
		_tick_ramp.detach();
	}
	if(_istemp){
		_tick_duration.detach();
	}
    
    _istemp = false;
    _ms_blink_on = ms_blink_on;
    _ms_blink_off = ms_blink_off;
    // si es temporal...
    if(ms_duration > 0){
        // hace un backup del estado actual y lanza el timer
        _istemp = true;
        _bkp_stat = _stat;
        _tick_duration.attach_us(callback(this, &Led::temporalCb), (ms_duration * 1000));
    }
    _stat = LedIsBlinking;    
    _action = LedGoOnEnd;
    if(_type == LedOnOffType){
        _max_intensity = convertIntensity(100);
        _min_intensity = convertIntensity(0);
        _intensity = _max_intensity;
        _out_01->write((uint8_t)_intensity);
    }
    else{
        _max_intensity = convertIntensity(intensity_on);
        _min_intensity = convertIntensity(intensity_off);
        _intensity = _max_intensity;
        _out->write(_intensity);        
    }
    _tick_blink.attach_us(callback(this, &Led::blinkCb), (_ms_blink_on * 1000));
}


//------------------------------------------------------------------------------------
void Led::updateBlinker(uint32_t ms_blink_on, uint32_t ms_blink_off){
    _ms_blink_on = ms_blink_on;
    _ms_blink_off = ms_blink_off;
}    


//------------------------------------------------------------------------------------
int Led::setBlinkMode(const uint32_t blinks[], uint8_t count){
	if(count > MaxBlinkCount){
		return -1;
	}
	_num_blinks = count;
	for(int i=0;i<_num_blinks;i++){
		_blinks[i] = blinks[i];
	}
	_curr_blink = -1;
	_executeBlinkMode();
	return 0;
}



//------------------------------------------------------------------------------------
//-- PRIVATE METHODS IMPLEMENTATION --------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void Led::_executeBlinkMode(){
	if(_num_blinks > 0){
		// siguiente parpadeo
		_curr_blink = (_curr_blink >= (_num_blinks-1))? 0 : (_curr_blink+1);
		// si es par correponde un ON
		if(!(_curr_blink & 1)){
			on(_blinks[_curr_blink]);
			return;
		}
		off(_blinks[_curr_blink]);
	}
}


//------------------------------------------------------------------------------------
void Led::rampOffCb(){
    if(_level == OnIsHighLevel){
        _intensity -= (_max_intensity/10);
        if(_intensity <= _min_intensity){
            _tick_ramp.detach();
            _intensity = _min_intensity;
            _action = LedGoOffEnd;
            _out->write(_intensity);
            return;
        }
    }
    else{
        _intensity += (_max_intensity/10);
        if(_intensity >= _max_intensity){
            _tick_ramp.detach();
            _intensity = _max_intensity;
            _action = LedGoOffEnd;
            _out->write(_intensity);
            return;
        }
    }
    _out->write(_intensity);
}


//------------------------------------------------------------------------------------
void Led::rampOnCb(){
    if(_level == OnIsHighLevel){
        _intensity += (_max_intensity/10);
        if(_intensity >= _max_intensity){
            _tick_ramp.detach();
            _intensity = _max_intensity;
            _action = LedGoOnEnd;
            _out->write(_intensity);
            return;
        }
    }
    else{
        _intensity -= (_max_intensity/10);
        if(_intensity <= _min_intensity){
            _tick_ramp.detach();
            _intensity = _min_intensity;
            _action = LedGoOnEnd;
            _out->write(_intensity);
            return;
        }
    }
    _out->write(_intensity);
}


//------------------------------------------------------------------------------------
void Led::blinkCb(){
    if(_action == LedGoOnEnd){
        _intensity = (_level == OnIsHighLevel)? _min_intensity : _max_intensity;
        _action = LedGoOffEnd;
        if(_type == LedOnOffType){
            _out_01->write((uint8_t)_intensity);
        }
        else{
            _out->write(_intensity);        
        }   
        _tick_blink.attach_us(callback(this, &Led::blinkCb), (_ms_blink_off * 1000)); 
                
    }
    else{
        _intensity = (_level == OnIsHighLevel)? _max_intensity : _min_intensity;
        _action = LedGoOnEnd;
        if(_type == LedOnOffType){
            _out_01->write((uint8_t)_intensity);
        }
        else{
            _out->write(_intensity);        
        }
        _tick_blink.attach_us(callback(this, &Led::blinkCb), (_ms_blink_on * 1000));         
        
    }
}


//------------------------------------------------------------------------------------
void Led::temporalCb(){
    _tick_blink.detach();
    _tick_ramp.detach();
    _tick_duration.detach();    
    _stat = _bkp_stat;
    // si está activado el modo blink en cascada, lo procesa
    if(_num_blinks > 0){
    	_executeBlinkMode();
    }
    // en otro caso lo procesa de forma normal
    else{
		if(_stat == LedIsOff){
			off(0, (uint8_t)(_min_intensity*100), 0);
		}
		else if (_stat == LedIsOn){
			on(0, (uint8_t)(_max_intensity*100), 0);
		}
		else{
			blink(_ms_blink_on, _ms_blink_off, 0, (uint8_t)(_max_intensity*100), (uint8_t)(_min_intensity*100));
		}
    }
}


//------------------------------------------------------------------------------------
double Led::convertIntensity(uint8_t intensity){
    intensity = (intensity > 100)? 100 : intensity;
    if(_level == OnIsHighLevel){
    	double fint = (intensity != 0)? (((double)intensity)/100) : 0;
        return(fint);
    }
    else{
    	double fint = (intensity != 1)? (((double)intensity)/100) : 1.0f;
    	return(1.0f - fint);
    }
}
