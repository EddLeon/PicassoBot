#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <tchar.h>
#include <stdint.h>
#include <chrono>

class SerialEmul {
public:
	SerialEmul();
	~SerialEmul();

	bool Open(int nPort, int nBaud);
	bool Close(void);

	bool available();
	uint8_t read();
	void flush();
	void write(uint8_t val);
	void write(uint8_t *val, DWORD x);
	//unsigned long millis();

	int ReadData(void *, int);
	int SendData(const char *, int);
	int ReadDataWaiting(void);

	bool IsOpened(void) { return(m_bOpened); }
	DWORD countTX;
	DWORD countRX;
	bool avail;
protected:
	bool WriteCommByte(unsigned char);

	HANDLE serial;
	unsigned char txBuffer[100];
	unsigned char txChar;
	unsigned char rxBuffer[100];
	bool m_bOpened;

};
