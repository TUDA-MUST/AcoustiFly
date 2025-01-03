/*
How to program:
0. Connect the AcoustiFly via USB.
1. Press and hold the boot-button.
2. While holding the boot-button, also press the reset-button.
3. Release reset and afterwards release boot.
4. Now you will be able to program the board.
5. After Flashing, shortly press the reset-button for your program to start.
*/
#include <Arduino.h>
#include <stdio.h>
#include "driver/ledc.h"
#include <math.h>
#include <EEPROM.h>
#include <scpiparser.h>
#include <WiFi.h>

// PWM SETTINGS
#define PWM_FREQ_WO_CAL 40000
#define LEDC_CHANNEL 0
#define LEDC_RESOLUTION_BIT 8                                      // ! Affects ledcwrite
const int DUTY_CYCLE = (int)(pow(2, LEDC_RESOLUTION_BIT) / 2 - 1); // calculate Duty Cycle to always be at 50%

// ADC SETTINGS
#define ADC_RESOLUTION 12 // set the ADC resolution to 12 bit (standard)

// PIN DECLARATION
#define PIN_HBRIDGE_A_NORMAL 6
#define PIN_HBRIDGE_A_INVERTED 5
#define PIN_HBRIDGE_B_NORMAL 7
#define PIN_HBRIDGE_B_INVERTED 8
#define PIN_HBRIDGE_EN 4
#define PIN_HBRIDGE_CURRENTSENSE 9
#define PIN_LED_GREEN 1
#define PIN_LED_RED 2
#define PIN_BAT_STATE 10
#define PIN_MISO 40
#define PIN_SCK 41
#define PIN_MOSI 42

// Auto-Cal Settings
#define EEPROMAdresse 0
#define Mess_Wiederholungen 100

// Define Functions
long get_battery_voltage_mv();
void set_frequency(long g_freq);
long g_current_frequency = 0;

// Stuff for checking battery state
#define TIME_MS_BATTERY_CHECK 1000 // I check for battery state in 1000ms intervalls
long g_ms_time_difference_battery = 0;

//SCPI Stuff:
struct scpi_parser_context obj_ctx;

scpi_error_t scpi_identify(struct scpi_parser_context *context, struct scpi_token *command);
scpi_error_t scpi_help(struct scpi_parser_context *context, struct scpi_token *command);
scpi_error_t scpi_diag(struct scpi_parser_context *context, struct scpi_token *command);
scpi_error_t scpi_reset(struct scpi_parser_context *context, struct scpi_token *command);
scpi_error_t scpi_get_frequency(struct scpi_parser_context *context, struct scpi_token *command);
scpi_error_t scpi_get_current(struct scpi_parser_context *context, struct scpi_token *command);
scpi_error_t scpi_set_frequency(struct scpi_parser_context *context, struct scpi_token *command);
scpi_error_t scpi_set_calvalue(struct scpi_parser_context *context, struct scpi_token *command);
scpi_error_t scpi_do_pause(struct scpi_parser_context *context, struct scpi_token *command);
scpi_error_t scpi_do_movedown(struct scpi_parser_context *context, struct scpi_token *command);

void setup()
{
  // Serial Setup
  Serial.begin(115200);

  // EEPROM Setup
  EEPROM.begin(24);

  // LED SETUP
  pinMode(PIN_LED_GREEN, OUTPUT); // set LED pins to output
  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_LED_RED, HIGH);
  GPIO.func_out_sel_cfg[PIN_LED_GREEN].inv_sel = 1; // invert the output since they are used as a current sink!
  GPIO.func_out_sel_cfg[PIN_LED_RED].inv_sel = 1;   // this way digitalWrite(HIGH) actually turns on the LED

  //SCPI Setup
  struct scpi_command* com_set;
  struct scpi_command* com_get;
  struct scpi_command* com_do;
  scpi_init(&obj_ctx);

  com_set = scpi_register_command(obj_ctx.command_tree, SCPI_CL_CHILD, "SET", 3, "S", 1, NULL);
  com_get = scpi_register_command(obj_ctx.command_tree, SCPI_CL_CHILD, "GET", 3, "G", 1, NULL);
  com_do = scpi_register_command(obj_ctx.command_tree, SCPI_CL_CHILD, "DO", 2, "D", 1, NULL);

  //SCPI Register Commands
  //Allgemein
  scpi_register_command(obj_ctx.command_tree, SCPI_CL_SAMELEVEL, "*IDN?", 5, "IDN?", 4, scpi_identify);
  scpi_register_command(obj_ctx.command_tree, SCPI_CL_SAMELEVEL, "*RST", 4, "RST", 3, scpi_reset);
  scpi_register_command(obj_ctx.command_tree, SCPI_CL_SAMELEVEL, "HELP?", 5, "HELP", 4, scpi_help);
  scpi_register_command(obj_ctx.command_tree, SCPI_CL_SAMELEVEL, "DIAG", 4, "DIA", 3, scpi_diag);
  //Set
  scpi_register_command(com_set, SCPI_CL_CHILD, "FREQUENCY", 9, "FREQ", 4, scpi_set_frequency);
  scpi_register_command(com_set, SCPI_CL_CHILD, "CALVALUE", 8, "CAL", 3, scpi_set_calvalue);
  //Get
  scpi_register_command(com_get, SCPI_CL_CHILD, "FREQUENCY", 9, "FREQ", 4, scpi_get_frequency);
  scpi_register_command(com_get, SCPI_CL_CHILD, "CURRENT", 7, "CURR", 4, scpi_get_current);
  //Do
  scpi_register_command(com_do, SCPI_CL_CHILD, "PAUSE", 5, "PA", 2, scpi_do_pause);
  scpi_register_command(com_do, SCPI_CL_CHILD, "MOVEDOWN", 8, "DOWN", 4, scpi_do_movedown);

  // PWM SETUP
  ledcSetup(LEDC_CHANNEL, PWM_FREQ_WO_CAL, LEDC_RESOLUTION_BIT); // Setup PWM Channel
  ledcAttachPin(PIN_HBRIDGE_A_NORMAL, LEDC_CHANNEL);             // attach Pins to PWM Channel
  ledcAttachPin(PIN_HBRIDGE_A_INVERTED, LEDC_CHANNEL);
  ledcAttachPin(PIN_HBRIDGE_B_NORMAL, LEDC_CHANNEL);
  ledcAttachPin(PIN_HBRIDGE_B_INVERTED, LEDC_CHANNEL);
  GPIO.func_out_sel_cfg[PIN_HBRIDGE_A_INVERTED].inv_sel = 1; // invert the output of two of the pins
  GPIO.func_out_sel_cfg[PIN_HBRIDGE_B_INVERTED].inv_sel = 1;
  pinMode(PIN_HBRIDGE_EN, OUTPUT); // enable HBRIDGE
  digitalWrite(PIN_HBRIDGE_EN, HIGH);
  ledcWrite(LEDC_CHANNEL, DUTY_CYCLE); // start PWM

  // Check if Transducer is present
  set_frequency(PWM_FREQ_WO_CAL * 0.9); // Setup Frequency to 40khz * 0.9 for baseline reading
  delay(500);
  long l_current_no_resonance = analogReadMilliVolts(PIN_HBRIDGE_CURRENTSENSE);
  set_frequency(PWM_FREQ_WO_CAL); // Setup Frequency to 40khz for baseline reading at resonace
  delay(500);
  long l_current_at_resonance = analogReadMilliVolts(PIN_HBRIDGE_CURRENTSENSE);
  Serial.print(String(F("Current at Resonance: ")) + String(l_current_at_resonance) + String(F(" - Current not at resonance: ")) + String(l_current_no_resonance));

  if (l_current_no_resonance < l_current_at_resonance * 0.9)
  { // Transducer is Present
    Serial.println(F(", i.e. Transducer should be present."));
  }
  else
  { // Transducer isnt present
    Serial.println();
    Serial.println(F("ERROR: NO TRANSDUCER DETECTED - DELETING STORED CAL-VALUES"));
    EEPROM.writeLong(EEPROMAdresse, 0);
    EEPROM.commit();
    while (analogReadMilliVolts(PIN_HBRIDGE_CURRENTSENSE) < l_current_at_resonance * 1.2)
    { // If measured Current is below te previously measured there is no Transducer installed
      Serial.println(F("ERROR: NO TRANSDUCER"));
      delay(250);
      digitalWrite(PIN_LED_RED, digitalRead(PIN_LED_RED));
    }
  }

  // Calibration-Routine, if necessary:
  long l_Actual_Measured_Current = 0;
  long l_Maximal_Measured_Current = 0;
  long l_Beste_Frequenz = 0;
  if (EEPROM.readLong(EEPROMAdresse) == 0 || EEPROM.read(EEPROMAdresse) == 255)
  { // Checks if there Calibration is neccesary
    digitalWrite(PIN_LED_RED, LOW);
    Serial.println(F("No Cal. value found - I will start calibrating now!!"));
    for (int i = 0; i < 10; i++)
    { // Blink for signalizing, that CAL will start.
      digitalWrite(PIN_LED_GREEN, digitalRead(PIN_LED_GREEN));
      delay(100);
    }
    for (int l_freq = 40500; l_freq >= 39500; l_freq = l_freq - 10)
    { // Hier kalibriere ich den Bumms in 10 Hz Schritten. This will take a while! 
      digitalWrite(PIN_LED_GREEN, digitalRead(PIN_LED_GREEN));
      set_frequency(l_freq);
      delay(2); // es wird 2ms gewartet bevor gesampelt wird
      l_Actual_Measured_Current = 0;
      for (int mess = 0; mess < Mess_Wiederholungen; mess++)
      {
        l_Actual_Measured_Current = l_Actual_Measured_Current + analogReadMilliVolts(PIN_HBRIDGE_CURRENTSENSE); // Addition aller l_Actual_Measured_Currente
        delay(10);
      }
      l_Actual_Measured_Current = l_Actual_Measured_Current / Mess_Wiederholungen; // Mittelwert wird gebildet
      Serial.println(String(F("Freq: ")) + String(l_freq) + String(F(" Current: ")) + String(l_Actual_Measured_Current));
      if (l_Actual_Measured_Current > l_Maximal_Measured_Current)
      {
        l_Maximal_Measured_Current = l_Actual_Measured_Current;
        l_Beste_Frequenz = l_freq;
      }
    }
    Serial.println(String(F("Best Freq: ")) + String(l_Beste_Frequenz) + String(F(" with current: ")) + String(l_Maximal_Measured_Current));
    EEPROM.writeLong(EEPROMAdresse, l_Beste_Frequenz);
    EEPROM.commit();
    delay(500);
  }

  // Finally turn levitator on. I avoid bifurcation by sweeping the frequency down.
  long l_cal_freq = EEPROM.readLong(EEPROMAdresse);
  Serial.println(String(F("Stored Cal. value is: ")) + String(l_cal_freq));
  delay(100);
  digitalWrite(PIN_LED_GREEN, LOW);
  for (int l_freq = 42000; l_freq >= l_cal_freq; l_freq = l_freq - 25)
  {
    set_frequency(l_freq); // Setup PWM Channel
    delay(1);
  }
  set_frequency(EEPROM.readLong(EEPROMAdresse));
  digitalWrite(PIN_LED_GREEN, HIGH);
  digitalWrite(PIN_LED_RED, LOW);
  Serial.println(F("Welcome to Acoustifly!"));
  Serial.println(F("Print HELP? to see all available commands."));
}

void loop()
{
  //Determine if Serial Data for detecting SCPI Data is available:
  if (Serial.available() > 0) //Hier schaue ich ob Serielle Daten Kommen & wenn ja dann wird der Befehl nach SCPI durchsucht
  {
    char line_buffer[128];
    unsigned char read_length;
    /* Read in a line and execute it. */
    read_length = Serial.readBytesUntil('\n', line_buffer, 128);
    if (read_length > 0)
    {
      scpi_execute_command(&obj_ctx, line_buffer, read_length);
    }
  }

  //This is for determining Battery-State: 
  if (g_ms_time_difference_battery > millis())
  {
    g_ms_time_difference_battery = millis();
  }
  if (millis() - g_ms_time_difference_battery > TIME_MS_BATTERY_CHECK)
  {
    g_ms_time_difference_battery = millis();
    long l_batteryvoltage_mv = get_battery_voltage_mv();
    //Serial.println(String(l_batteryvoltage_mv) + " ; " + String(analogReadMilliVolts(PIN_HBRIDGE_CURRENTSENSE)));
    if (l_batteryvoltage_mv < 3200) //Battery is fucking low -> Turn of Levitation
    {
      digitalWrite(PIN_HBRIDGE_EN, LOW);
      digitalWrite(PIN_LED_GREEN, LOW);
      bool quit = false;
      while (quit == false)
      {
        digitalWrite(PIN_LED_RED, digitalRead(PIN_LED_RED));
        delay(100);
        if (get_battery_voltage_mv() > 3800)
        {
          quit = true;
        }
      }
      digitalWrite(PIN_HBRIDGE_EN, HIGH);
      digitalWrite(PIN_LED_GREEN, HIGH);
    }
    else if (l_batteryvoltage_mv < 3500) //Battery will soon run out, indicate that
    {
      digitalWrite(PIN_LED_RED, digitalRead(PIN_LED_RED));
      digitalWrite(PIN_LED_GREEN, digitalRead(PIN_LED_GREEN));
    }
    else //Battery State is ok. 
    {
      digitalWrite(PIN_LED_GREEN, HIGH);
      digitalWrite(PIN_LED_RED, LOW);
    }
  }
}

// Functions:
long get_battery_voltage_mv()
{
  return analogReadMilliVolts(PIN_BAT_STATE) * 11 + 200;
}

void set_frequency(long g_freq)
{
  ledcSetup(LEDC_CHANNEL, g_freq, LEDC_RESOLUTION_BIT);
  g_current_frequency = g_freq;
}

void printSerialError(){
  Serial.println("ERROR");
}

//Antwortet auf *IDN?
scpi_error_t scpi_identify(struct scpi_parser_context *context, struct scpi_token *command)
{
  scpi_free_tokens(command);

  Serial.println(F("This is Acoustifly!"));
  Serial.println(F("Made by Measurement and Sensor Technology Group at TU Darmstadt"));
  Serial.println(F("https://www.etit.tu-darmstadt.de/must/home_must/index.de.jsp"));
  Serial.println(F("This is a project of Sven Suppelt"));
  Serial.println(F("https://de.linkedin.com/in/sven-suppelt-81b604190"));
  Serial.println(F("Software Date: 11.09.2024"));
  return SCPI_SUCCESS;
}

//Reset the device
scpi_error_t scpi_reset(struct scpi_parser_context *context, struct scpi_token *command)
{
  scpi_free_tokens(command);
  Serial.println(F("Resetting the device as well as the cal. value."));
  Serial.println(F("Goodbye!"));
  EEPROM.writeLong(EEPROMAdresse, 0);
  EEPROM.commit();
  ESP.restart();
  return SCPI_SUCCESS;
}

//Antwortet auf help?
scpi_error_t scpi_help(struct scpi_parser_context *context, struct scpi_token *command)
{
  scpi_free_tokens(command);
  Serial.println(F("Available Commands:"));
  Serial.println(F("*IDN?  -  Returns Identification."));
  Serial.println(F("*RST   -  Restarts the device and deletes its cal. value."));
  Serial.println(F("HELP?  -  Returns this text."));
  Serial.println(F("DIAG   -  Returns Diagnostic Information."));
  Serial.println(F(":SET:FREQUENCY XXXX   -  Sets the H-Bridge Frequency to this Value (XXXX). Unit is Hz. Allowed 30000 - 50000.")); 
  Serial.println(F(":SET:CALVALUE XXXX   -  Sets the CalValue to this Value (XXXX). Unit is Hz. Allowed 30000 - 50000. Warning: Not Recommended!"));
  Serial.println(F(":GET:FREQUENCY  -   Returns the current Frequency."));
  Serial.println(F(":GET:CURRENT   -   Measures the current and returns it."));
  Serial.println(F(":DO:PAUSE XXXX  -  Pauses the Output for the given time XXXX in ms. Allowed 0 - 1000."));
  Serial.println(F(":DO:MOVEDOWN   -   Tries to do a MoveDown with values calculated by the free fall equations."));
  Serial.println(F("------------------"));
  Serial.println(F("ATTENTION: PLEASE WRITE EVERYTHING IN CAPS."));
  // Serial.println(F("Commands are in SCPI-Notation, meaning, you can write :DO:MOVEDOWN or :D:DOWN ."));
  // Serial.println(F("I.e. everything that is not caps is optional!"));
  return SCPI_SUCCESS;
}

//Print Diag. Info
scpi_error_t scpi_diag(struct scpi_parser_context *context, struct scpi_token *command)
{
  scpi_free_tokens(command);

  Serial.println(String(F("Current frequency: ")) + String(g_current_frequency));
  Serial.println(String(F("Battery state (mV): ")) + String(get_battery_voltage_mv()));
  Serial.println(String(F("Current: ")) + String(analogReadMilliVolts(PIN_HBRIDGE_CURRENTSENSE)));
  Serial.println(String(F("Stored Cal. value: ")) + String(EEPROM.readLong(EEPROMAdresse)));
  Serial.println(String(F("Uptime (ms): ")) + String(millis()));
  Serial.println(String(F("ESP Board MAC Address: ")) + String(WiFi.macAddress()));
  return SCPI_SUCCESS;
}

//Set Frequency
scpi_error_t scpi_set_frequency(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_token* args;
  struct scpi_numeric output_numeric;
  unsigned char output_value;

  args = command;

  while (args != NULL && args->type == 0)
  {
    args = args->next;
  }

  output_numeric = scpi_parse_numeric(args->value, args->length, 1e3, 0, 25e6);
  if (output_numeric.length == 0 ||
      (output_numeric.length == 2 && output_numeric.unit[0] == 'H' && output_numeric.unit[1] == 'z'))
  {
    double input = output_numeric.value;
    if (input > 30000 && input < 50000) {
      set_frequency(input);
      Serial.println("OK");
    }
    else {
      printSerialError();
    }
  }
  else
  {
    printSerialError();
    scpi_error error;
    error.id = -200;
    error.description = "Invalid unit";
    error.length = 26;

    scpi_queue_error(&obj_ctx, error);
    scpi_free_tokens(command);
    return SCPI_SUCCESS;
  }

  scpi_free_tokens(command);

  return SCPI_SUCCESS;
}

//Set Cal Value
scpi_error_t scpi_set_calvalue(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_token* args;
  struct scpi_numeric output_numeric;
  unsigned char output_value;

  args = command;

  while (args != NULL && args->type == 0)
  {
    args = args->next;
  }

  output_numeric = scpi_parse_numeric(args->value, args->length, 1e3, 0, 25e6);
  if (output_numeric.length == 0 ||
      (output_numeric.length == 2 && output_numeric.unit[0] == 'H' && output_numeric.unit[1] == 'z'))
  {
    double input = output_numeric.value;
    if (input > 30000 && input < 50000) {
      EEPROM.writeLong(EEPROMAdresse, input);
      EEPROM.commit();
      Serial.println("OK");
    }
    else {
      printSerialError();
    }
  }
  else
  {
    printSerialError();
    scpi_error error;
    error.id = -200;
    error.description = "Invalid unit";
    error.length = 26;

    scpi_queue_error(&obj_ctx, error);
    scpi_free_tokens(command);
    return SCPI_SUCCESS;
  }

  scpi_free_tokens(command);

  return SCPI_SUCCESS;
}

//Gibt die Aktuell eingestellte Frequenz zurück
scpi_error_t scpi_get_frequency(struct scpi_parser_context* context, struct scpi_token* command)
{
  Serial.println(g_current_frequency);
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

//Gibt die Aktuellen Current zurück
scpi_error_t scpi_get_current(struct scpi_parser_context* context, struct scpi_token* command)
{
  Serial.println(analogReadMilliVolts(PIN_HBRIDGE_CURRENTSENSE));
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

//Do Pause
scpi_error_t scpi_do_pause(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_token* args;
  struct scpi_numeric output_numeric;
  unsigned char output_value;

  args = command;

  while (args != NULL && args->type == 0)
  {
    args = args->next;
  }

  output_numeric = scpi_parse_numeric(args->value, args->length, 1e3, 0, 25e6);
  if (output_numeric.length == 0 ||
      (output_numeric.length == 2 && output_numeric.unit[0] == 'H' && output_numeric.unit[1] == 'z'))
  {
    double input = output_numeric.value;
    if (input > 0 && input < 1000) {
      digitalWrite(PIN_LED_GREEN, LOW);
      digitalWrite(PIN_LED_RED, HIGH);
      digitalWrite(PIN_HBRIDGE_EN, LOW);
      delay(input);
      digitalWrite(PIN_HBRIDGE_EN, HIGH);
      digitalWrite(PIN_LED_GREEN, HIGH);
      digitalWrite(PIN_LED_RED, LOW);
      Serial.println("OK");
    }
    else {
      printSerialError();
    }
  }
  else
  {
    printSerialError();
    scpi_error error;
    error.id = -200;
    error.description = "Invalid unit";
    error.length = 26;

    scpi_queue_error(&obj_ctx, error);
    scpi_free_tokens(command);
    return SCPI_SUCCESS;
  }

  scpi_free_tokens(command);

  return SCPI_SUCCESS;
}

//Do MoveDown
scpi_error_t scpi_do_movedown(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);
  //Values from calculating the free-fall distance
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, HIGH);
  digitalWrite(PIN_HBRIDGE_EN, LOW);
  delayMicroseconds(29579);
  digitalWrite(PIN_HBRIDGE_EN, HIGH);
  digitalWrite(PIN_LED_GREEN, HIGH);
  digitalWrite(PIN_LED_RED, LOW);
  Serial.println("OK");
  return SCPI_SUCCESS;
}
