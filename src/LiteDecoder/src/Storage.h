#ifndef _STORAGE_H
#define _STORAGE_H

#include <Arduino.h>

class EpromStream
{
  private:
    EpromStream(unsigned int index);

    friend struct Storage;

    unsigned int m_uIndex;

  public:
    void Get(unsigned char &byte);
    void Get(unsigned short &number);
	void Get(uint16_t &number);

	unsigned int Get(char *name, unsigned int nameSize);

    void Put(unsigned char byte);
    void Put(unsigned short number);

	template <typename T>
	unsigned int PutData(const T &data);

#if 0 //CHECK THIS
    void Put(const char *str);
#endif

};

struct Storage
{
  static bool LoadConfig();

  static void SaveConfig();

  static void Dump();
};

#endif
