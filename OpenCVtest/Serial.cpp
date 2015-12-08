
#include "Serial.h"


SerialEmul::SerialEmul() { m_bOpened = false;countTX = 0;countRX = 0;avail = false; }
SerialEmul::~SerialEmul() {
	Close();
}
bool SerialEmul::Open(int nPort, int nBaud)
{

	if (m_bOpened) return(true);

	wchar_t szPort[15];
	DCB dcb;
	COMMTIMEOUTS timeouts;

	wsprintfW(szPort, L"\\\\.\\COM%d", nPort);
	serial = CreateFile(szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (serial == INVALID_HANDLE_VALUE) {
		/* printf("Error: Could not connect to COM%d.",nPort);
		exit(-1l); */
		return false;
	}

	GetCommState(serial, &dcb); //Allows one to obtain an already valid configuration.

	dcb.BaudRate = nBaud;
	dcb.ByteSize = 8;
	dcb.fBinary = true;
	dcb.fParity = false;
	dcb.StopBits = ONESTOPBIT;

	SetCommState(serial, &dcb);

	timeouts.ReadIntervalTimeout = 0xFFFFFFFF;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 5000;
	SetCommTimeouts(serial, &timeouts);

	m_bOpened = true;
	return true;
}
bool SerialEmul::Close() {
	if (serial == NULL) return true;

	CloseHandle(serial);
	serial = NULL;
	return true;
}
bool SerialEmul::available() {
	if (avail)
		return true;
	ReadFile(serial, &rxBuffer[0], 1, &countRX, NULL);
	if (countRX)
		avail = true;
	else
		avail = false;
	return avail;
}
uint8_t SerialEmul::read() {
	if (!avail) {
		ReadFile(serial, &rxBuffer[0], 1, &countRX, NULL);
	}
	avail = false;
	countRX = 0;
	return rxBuffer[0];
}
void SerialEmul::flush() {
	FlushFileBuffers(serial);
}
void SerialEmul::write(uint8_t val) {
	WriteFile(serial, (LPSTR)&val, sizeof(val), &countTX, NULL);
}
void SerialEmul::write(uint8_t* val, DWORD x) {
	WriteFile(serial, (LPSTR)val, x, &countTX, NULL);
}
/*unsigned long SerialEmul::millis() {
	auto epoch = std::chrono::high_resolution_clock::from_time_t(0);
	auto now = std::chrono::high_resolution_clock::now();
	auto mseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - epoch).count();
	return mseconds;
}*/