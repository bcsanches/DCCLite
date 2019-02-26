#pragma once

#include <string_view>

#include "json.hpp"
#include "Object.h"

#include <JsonCreator/Object.h>
#include <JsonCreator/StringWriter.h>

//add drives?
		//add Providers?
		//Add HomeLocation for Providers?

		/*
		Providers:
			- ItemProviders
				Get-Item -> return item info
				Set-Item -> change the value of a item
				Clear-Item -> delete contents of a item, but not the item
				Remove-Item -> delete the item
				Invoke-item -> run item default action (throw turnout?)
			- DriverProviders
			- ContainerProviders
				- Copy-Item
				- Get-ChildItem
				- New-Item
				- Remove-Item
				- Rename-Item

		https://docs.microsoft.com/en-us/powershell/developer/cmdlet/approved-verbs-for-windows-powershell-commands
		New vs. Set
The New verb is used to create a new resource. The Set verb is used to modify an existing resource, optionally creating the resource if it does not exist, such as the Set-Variable cmdlet.

Find vs. Search
The Find verb is used to look for an object. The Search verb is used to create a reference to a resource in a container.

Get vs. Read
The Get verb is used to retrieve a resource, such as a file. The Read verb is used to get information from a source, such as a file.

Invoke vs. Start
The Invoke verb is used to perform an operation that is generally a synchronous operation, such as running a command. The Start verb is used to begin an operation that is generally an asynchronous operation, such as starting a process.
*/


/*

Responses:
{
	"jsonrpc": "2.0", 
	"result":
	{
		"class":"something"
		"data":[...] or "data":{...}"
	}
	"id": 4
}

*/

typedef int CmdId_t;


class TerminalContext
{
	public:		
		TerminalContext(dcclite::FolderObject &root):
			m_rclRoot(root)
		{
			//empty
		}

		TerminalContext(const TerminalContext &) = delete;

		TerminalContext(TerminalContext &&other) :
			m_rclRoot(other.m_rclRoot),
			m_pthLocation(std::move(other.m_pthLocation))
		{
			//empty
		}

		void SetLocation(const dcclite::Path_t &newLocation);		

		inline const dcclite::Path_t &GetLocation() const
		{
			return m_pthLocation;
		}		

		dcclite::IObject *GetItem() const;

		TerminalContext &operator=(TerminalContext &rhs) = delete;
		TerminalContext &operator=(TerminalContext &&rhs) = delete;

	private:
		dcclite::FolderObject &m_rclRoot;
		dcclite::Path_t m_pthLocation;
};

class TerminalCmdException: public std::exception
{
	public:
		TerminalCmdException(std::string &&what, CmdId_t id) :
			m_iId(id),
			m_strWhat(std::move(what))
		{
			//empty
		}


		virtual const char *what() const noexcept
		{
			return m_strWhat.c_str();
		}

		CmdId_t GetId() const noexcept
		{
			return m_iId;
		}

	private:
		CmdId_t		m_iId;
		std::string m_strWhat;
};

class TerminalCmd
{
	public:
		typedef JsonCreator::Object<JsonCreator::StringWriter> Result_t;

	public:
		TerminalCmd(std::string_view name);

		TerminalCmd(const TerminalCmd &) = delete;
		TerminalCmd(TerminalCmd &&) = delete;

		~TerminalCmd();

		virtual void Run(TerminalContext &context, Result_t &results, const CmdId_t id, const nlohmann::json &request) = 0;

		static TerminalCmd *TryFindCmd(std::string_view name);

	private:
		std::string m_strName;
};
