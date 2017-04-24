/////////////////////////////////////////////////////////////////////////////
// Demo.h
/////////////////////////////////////////////////////////////////////////////

#ifndef VOLTMETER_H_
#define VOLTMETER_H_

/////////////////////////////////////////////////////////////////////////////
// Configuration
/////////////////////////////////////////////////////////////////////////////

// LCD refresh rate in Hz
#define DEMO_FRAME_RATE                50

// Font multipliers
#define FONT_SCALE_DEFAULT             1
#define FONT_SCALE_VOLTAGE             3

// Hysteresis in mV for Voltage display
#define HYSTERESIS_MV                  9

/////////////////////////////////////////////////////////////////////////////
// Defines
/////////////////////////////////////////////////////////////////////////////

// Beginning row number for y-axis centering
#define Y_VOLTAGE_POS                  (FONT_HEIGHT * 2)

// Center column number for x-axis centering
#define X_CENTER                       128/2

/////////////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////////////

void Voltmeter_main(void);


#endif /* VOLTMETER_H_ */
