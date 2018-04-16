#include "Storage.h"

#if 0
#include "Device.h"
#include "DeviceManager.h"
#endif

#include <stdio.h>
#include <string.h>

#include <EEPROM.h>

#include "Console.h"

#include "Storage.h"

#define STORAGE_MAGIC "Bcs0001"

#define MODULE_NAME "Storage"

//#define SKIP_BYTE

struct StorageData
{
    char m_archMagic[sizeof(STORAGE_MAGIC)];

    unsigned short m_nNumObjects;
};

static StorageData  gData;

static void Clear()
{
	Console::SendLog(MODULE_NAME, "Clear");

	sprintf(gData.m_archMagic, STORAGE_MAGIC);
	gData.m_nNumObjects = 0;

	EEPROM.put(0, gData);
}

void Storage::Dump()
{
	Console::SendLn("");

	char number[16];

	for(unsigned int i = 0;i < EEPROM.length(); ++i)
	{
		unsigned char ch = EEPROM.read(i);

		sprintf(number, "%02x ", ch);

		Console::Send(number);
	}

	Console::SendLn("");
	Console::SendLn("");
}

bool Storage::LoadConfig()
{
    Console::SendLog(MODULE_NAME, "init %d", sizeof(STORAGE_MAGIC));

    memset(&gData, 0, sizeof(gData));

    EpromStream stream(0);

	Console::SendLog(MODULE_NAME, gData.m_archMagic);

    stream.Get(gData.m_archMagic, sizeof(gData.m_archMagic));
    stream.Get(gData.m_nNumObjects);

	Console::SendLog(MODULE_NAME, gData.m_archMagic);

    if(strncmp(gData.m_archMagic, STORAGE_MAGIC, sizeof(STORAGE_MAGIC)))
    {
        Console::SendLog(MODULE_NAME, "invalid header");

        //Garbage or nothing at storage, so we clear it
	    Clear();

        return false;
    }
    else
    {
        Console::SendLog(MODULE_NAME, "load %s %u objs", gData.m_archMagic, gData.m_nNumObjects);
#if 0
		unsigned short loadedObjects = 0;
		while(loadedObjects < gData.m_nNumObjects)
		{
			char className[MAX_CLASS_NAME];

			stream.get(className, sizeof(className));
			unsigned short numDevices;

			stream.get(numDevices);

			auto deviceClass = DeviceClass::find(className);
			if(!deviceClass)
			{
				NetClient::sendLog(MODULE_NAME, "class not found: %s", className);

				//invalid contents, clear it and abort
				clear();

				return false;
			}

			NetClient::sendLog(MODULE_NAME, "load %s - %u objs", className, numDevices);

			for(unsigned int i = 0;i < numDevices; ++i)
			{
				DeviceId_t id;

				stream.get(id);

				Device *dev = deviceClass->create(id);
				if(!dev)
				{
					NetClient::sendLog(MODULE_NAME, "out of mem", className, numDevices);

					return false;
				}

        		NetClient::sendLog(MODULE_NAME, "load %u obj", id);

       			++loadedObjects;

				if(!dev->load(stream))
				{
					NetClient::sendLog(MODULE_NAME, "load of dev %u failed", id);

					delete dev;
					continue;
				}
			}	
		}
#endif		
    }

    Console::SendLog(MODULE_NAME, "ok");

    return true;
}

void Storage::SaveConfig()
{
    unsigned short numDevices = 0;

#if 0
    //count how many devices we have in total
    for(auto dev = Device::getHead(); dev; dev = dev->getNext(), ++numDevices);

    gData.m_nNumObjects = numDevices;

    NetClient::sendLog(MODULE_NAME, "saving %u objs", gData.m_nNumObjects);

    EpromStream stream(0);

	//unsigned short test = 0xff;

	//stream.put(test);

    stream.put(STORAGE_MAGIC);
    stream.put(gData.m_nNumObjects);

    for(const DeviceClass *deviceClass = (DeviceClass::getHead()); deviceClass; deviceClass = deviceClass->getNext())
    {
        //first, count how many devices for this type we have
        numDevices = 0;
        for(auto dev = Device::getHead(); dev; dev = dev->getNext())
        {
            if(&dev->getClass() == deviceClass)
            {
                ++numDevices;
            }
        }

        NetClient::sendLog(MODULE_NAME, "saving %s - %u objs", deviceClass->getClassName(), numDevices);

        //if nothing, ignore, dont waste EEPROM
        if(!numDevices)
            continue;

        stream.put(deviceClass->getClassName());
        stream.put(numDevices);
		//stream.put((unsigned char)'Z');

        //now write the devices
        for(auto dev = Device::getHead(); dev; dev = dev->getNext())
        {
            if(&dev->getClass() == deviceClass)
            {
                NetClient::sendLog(MODULE_NAME, "saving device %u", dev->getId());
                dev->save(stream);
            }
        }
    }
#endif
    Console::SendLog(MODULE_NAME, "save done");
}



EpromStream::EpromStream(unsigned int index):
    m_uIndex(index)
{
    //empty
}

void EpromStream::Get(unsigned char &byte)
{
	byte = EEPROM.read(m_uIndex);

	++m_uIndex;
}

void EpromStream::Get(unsigned short &number)
{

#ifdef SKIP_BYTE
	if(m_uIndex & 1)
		++m_uIndex;
#endif

	EEPROM.get(m_uIndex, number);

	m_uIndex += sizeof(number);
}

unsigned int EpromStream::Get(char *name, unsigned int nameSize)
{
	if(!nameSize)
		return 0;

	for(unsigned int i = 0;i < nameSize; ++i)
	{
		char ch = EEPROM.read(m_uIndex);
		++m_uIndex;

		name[i] = ch;

		if(ch == 0)
		{
      		//NetClient::sendLog(MODULE_NAME, "BLAi %s", name);

			return i;
		}
	}

	//reached the end of the buffer
	name[nameSize-1] = 0;

  	//Console::SendLog(MODULE_NAME, "BLA %s", name);

	return nameSize-1;
}

void EpromStream::Put(unsigned char byte)
{
	//NetClient::sendLog(MODULE_NAME, "w 1 byte %c at %u", (char)byte, m_uIndex);

    EEPROM.put(m_uIndex, byte);
    m_uIndex += sizeof(byte);
}

void EpromStream::Put(unsigned short number)
{
	//NetClient::sendLog(MODULE_NAME, "w %u bytes (short) %u at %u", sizeof(number), number,  m_uIndex);

#ifdef SKIP_BYTE
	if(m_uIndex & 1)
	{
		EEPROM.put(m_uIndex, (char)0);

		++m_uIndex;
		NetClient::sendLog(MODULE_NAME, "SKIP");
	}
#endif

    EEPROM.put(m_uIndex, number);
    m_uIndex += sizeof(number);
}

#if 0
void EpromStream::Put(const char *str)
{
	//NetClient::sendLog(MODULE_NAME, "w str %s at %u", str,  m_uIndex);
    while(*str)
    {
        this->Put(static_cast<unsigned char>(*str));
        ++str;
    }

    //make sure 0 goes thought
    this->Put(static_cast<unsigned char>(*str));
}
#endif