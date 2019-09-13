#include "main.h"

/**
 * @brief hardware init pin, port for keypad
 * @param  none
 * @retval none
 **/ 
void key_hw_init()
{
  pinMode(KEY_MODE_PIN, INPUT_PULLUP);
  pinMode(KEY_NEXT_PIN, INPUT_PULLUP);
  pinMode(KEY_BACK_PIN, INPUT_PULLUP);
  pinMode(KEY_OK_PIN, INPUT_PULLUP);
}

/**
 * @brief check row has pressed and return it's code
 * @param row as row in pad
 * @retval code of key pressed.
 **/ 
key_pressed_t key_hw_read()
{
  if(KEY_MODE_IN){ return KEY_MODE;}
  if(KEY_NEXT_IN){ return KEY_NEXT;}
  if(KEY_BACK_IN){ return KEY_BACK;}
  if(KEY_OK_IN){ return KEY_OK;}
  
  return KEY_INVALID;
}

/**
 * @brief assert low to column and get code of row which has pressed.
 * @param none
 * @retval code of key
 **/ 
static key_pressed_t key_scan(void)
{
  key_pressed_t key;

  /* check input as low */
  key = key_hw_read();
  if(key != KEY_INVALID) return key;

  return KEY_INVALID;
}

/**
 * @brief get key code which pressed and store in queue.
 * @param tmo as time out for wait queue ready.
 * @retval key code
 **/ 
key_pressed_t key_getcode()
{
    key_pressed_t key;
    //check if key_is_pressed
    key = key_scan();
    return key;
  
  return KEY_INVALID;
}
