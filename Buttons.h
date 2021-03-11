#ifndef ESPBREW_BUTTONS_H
#define ESPBREW_BUTTONS_H

const int TOUCH_BUTTON_TIME_INTERVAL = 1000; // milliseconds
const int TOUCH_BUTTON_THRESHOLD = 30; // Resistive button 

/* We use a matrix to control up to 40 buttons simultaneously */
unsigned long ul_touchButtonTimeControl[40];

/* Summary: 
   Checks whether a resistive button has been pressed by the user. 
   This function can restrict the repetitive pressing of the button through 
   a time interval control.

   Params:
   i_ResistiveButtonPin: identifies the pin used to control the resistive button
   b_hasTimeControl: When the function has time control, the resistive button 
                     can only be activated once for each time interval
                     defined by TOUCH_BUTTON_TIME_INTERVAL

   Return: true if button has been pressed, otherwise false  
*/
bool CheckResistiveButton(int i_ResistiveButtonPin, bool b_hasTimeControl)
{
  bool b_returnValue = false;
  
  if (touchRead(i_ResistiveButtonPin) < TOUCH_BUTTON_THRESHOLD)
  {
    if (b_hasTimeControl)
    {
      if (millis() - ul_touchButtonTimeControl[i_ResistiveButtonPin] > TOUCH_BUTTON_TIME_INTERVAL)
      {
        b_returnValue = true;
        ul_touchButtonTimeControl[i_ResistiveButtonPin] = millis();    
      }
    }
    else
    {
      b_returnValue = true;
    }    
  }

#ifdef _DEBUG_
  if (b_returnValue) Serial.println("Button was pressed!");
#endif

  return b_returnValue;
}

#endif /* ESPBREW_BUTTONS_H */
