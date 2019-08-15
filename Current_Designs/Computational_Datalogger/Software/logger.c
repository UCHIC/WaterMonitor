/************************************************************\
 * Python Module 'Logger' for the Computational Datalogger.
 *
 * The functions in this module are designed for interacting
 * with the attached AVR based datalogger.
\************************************************************/

#include <Python.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <unistd.h>

#define ROM_BUSY    24
#define POWER_GOOD  25
#define BUFFER_MAX  21600
#define HEADER_SIZE 9

/** Initialize the Logger Module **/

static PyObject* init(PyObject* self, PyObject* args)
{
	wiringPiSetupGpio();			// Setup the wiringPi library to use Broadcom GPIO numbers.
	wiringPiSPISetup(0, 2000000);		// Setup the wiringPi SPI library to use CS 0 @ 2 MHz.

	pinMode(ROM_BUSY, OUTPUT);		// ROM_BUSY Pin output low
	digitalWrite(ROM_BUSY, LOW);

	pinMode(POWER_GOOD, OUTPUT);		// POWER_GOOD Pin output low
	digitalWrite(POWER_GOOD, LOW);

	sleep(1);				// Delay for one second

	return Py_None;
}

/** Return the maximum allowable buffer value **/

static PyObject* bufferMax(PyObject* self, PyObject* args)
{
	return Py_BuildValue("i", BUFFER_MAX);
}

/** Tell the AVR datalogger that the EEPROM chip is in use **/

static PyObject* setRomBusy(PyObject* self, PyObject* args)
{
	digitalWrite(ROM_BUSY, HIGH);	// Setting this pin high tells the datalogger the chip is in use

	return Py_None;
}

/** Tell the AVR datalogger that the Pi has successfully powered up **/

static PyObject* setPowerGood(PyObject* self, PyObject* args)
{
	digitalWrite(POWER_GOOD, HIGH);	// Setting this pin high tells the datalogger the Pi is on.

	return Py_None;
}

/** Read the contents of the EEPROM chip **/

static PyObject* loadData(PyObject* self, PyObject* args)
{
	unsigned int dataSize = BUFFER_MAX + HEADER_SIZE + 4;	// How many bytes to read from the EEPROM
	unsigned char data[dataSize];				// Array will hold data from the EEPROM
	data[0] = 0x03;						// Load the array with: Read instruction
	data[1] = 0x00;						//                      Address (High): 0
	data[2] = 0x00;						//                      Address (Mid):  0
	data[3] = 0x00;						//                      Address (Low):  0

	wiringPiSPIDataRW(0, data, dataSize);			// SPI Transaction: The contents of data are overwritten by the EEPROM response

	unsigned int recordNum = (data[4] << 16) + (data[5] << 8) + data[6];	// Calculate number of records stored

	unsigned int lastIndex = recordNum + 12;		// lastIndex points to the last record stored in data[]

	PyObject* dataTuple = PyTuple_New(recordNum + 7);	// The dataTuple is what will be used in the Python script Logger is used in.

	PyObject* PyData = Py_BuildValue("i", recordNum);	// To store data in dataTuple, we need a PyObject.

	PyTuple_SetItem(dataTuple, 0, PyData);			// The first value in dataTuple is the number of records it holds.

	unsigned int i;
	for(i = 7; i <= lastIndex; i++)				// This loop fills dataTuple with date/time and records.
	{
		PyData = Py_BuildValue("b", data[i]);
		PyTuple_SetItem(dataTuple, i - 6, PyData);
	}

	return dataTuple;					// dataTuple is returned for use in a Python script
}

/** Tell the AVR datalogger that the EEPROM chip is no longer in use **/

static PyObject* setRomFree(PyObject* self, PyObject* args)
{
	digitalWrite(ROM_BUSY, LOW);

	return Py_None;
}

/** Tell the AVR datalogger that the Pi is shutting down **/

static PyObject* setPowerOff(PyObject* self, PyObject* args)
{
	digitalWrite(POWER_GOOD, LOW);

	return Py_None;
}

static PyMethodDef methods[] = {
	{ "init", init, METH_NOARGS, "Initializes the Logger Python Module" },
	{ "bufferMax", bufferMax, METH_NOARGS, "Returns the maximum buffer size from the datalogger" },
        { "setRomBusy", setRomBusy, METH_NOARGS, "Sends a signal to the datalogger that the EEPROM chip is in use" },
        { "setPowerGood", setPowerGood, METH_NOARGS, "Sends a signal to the datalogger that the Pi booted succesfully" },
        { "loadData", loadData, METH_NOARGS, "Reads data from the EEPROM chip and returns a tuple" },
        { "setRomFree", setRomFree, METH_NOARGS, "Sends a signal to the datalogger that the EEPROM chip is not in use" },
        { "setPowerOff", setPowerOff, METH_NOARGS, "Sends a signal to the datalogger that the Pi is shutting down" },
        { NULL, NULL, 0, NULL }
};

PyMODINIT_FUNC initLogger(void)
{
        Py_InitModule3("Logger", methods, "Python Module written in C for interacting with specific external hardware");
}