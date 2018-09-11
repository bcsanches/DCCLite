#ifndef _STORAGE_H
#define _STORAGE_H

#include <Arduino.h>

class EpromStream
{
  private:
    EpromStream(unsigned int index);

    friend struct Storage;
	friend class LumpWriter;

    uint32_t m_uIndex;

	//for lump writer
	void Skip(uint32_t bytes);
	void Seek(uint32_t pos);

  public:
  	void Get(char &ch);
    void Get(unsigned char &byte);	

#ifndef WIN32
    void Get(unsigned short &number);
#endif
	void Get(uint16_t &number);

	unsigned int Get(char *name, unsigned int nameSize);

	void Put(char ch);
    void Put(unsigned char byte);

#ifndef WIN32
    void Put(unsigned short number);
#endif

	void Put(uint16_t number);

	template <typename T>
	unsigned int PutData(const T &data);

#if 0 //CHECK THIS
    void Put(const char *str);
#endif

};

class LumpWriter
{
	private:
		const char *m_pszName;
		EpromStream &m_rStream;

		uint32_t	m_uStartIndex;

	public:
		LumpWriter(EpromStream &stream, const char *lumpName);

		~LumpWriter();
};

struct Storage
{
  static bool LoadConfig();

  static void SaveConfig();

  static void Dump();
};

#endif
