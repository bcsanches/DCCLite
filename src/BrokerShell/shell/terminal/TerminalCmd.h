// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <variant>

#include <rapidjson/document.h>

#include <dcclite/FolderObject.h>

namespace dcclite::broker::shell::terminal
{ 

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

			https://docs.microsoft.com/pt-br/powershell/scripting/developer/cmdlet/approved-verbs-for-windows-powershell-commands?view=powershell-7.1
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

	class TerminalContext;	

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

	/**
	* 
	* This is not a real fiber, but we call it as it stays "alive" waiting a for most cases 
	* a network device task to complete
	*
	*
	*/
	class TerminalCmdFiber
	{
		public:
			typedef JsonCreator::Object<JsonCreator::StringWriter> Result_t;

		public:
			TerminalCmdFiber(const CmdId_t id, TerminalContext &context) :
				m_tCmdId(id),
				m_rclContext(context)
			{
				//empty
			}		

			virtual ~TerminalCmdFiber() = default;

		protected:
			const CmdId_t m_tCmdId;
			TerminalContext &m_rclContext;

	};


	/**

	A cmd that can be invoked thought terminals

	*/
	class TerminalCmd: public dcclite::Object
	{
		public:
			typedef JsonCreator::Object<JsonCreator::StringWriter> Result_t;
			typedef std::variant<std::string, std::unique_ptr<TerminalCmdFiber>> CmdResult_t;

		public:
			TerminalCmd(RName name):
				Object{ name }
			{
				//empty
			}

			TerminalCmd(const TerminalCmd &) = delete;
			TerminalCmd(TerminalCmd &&) = delete;
			
			[[nodiscard]] virtual CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) = 0;

			virtual const char *GetTypeName() const noexcept
			{
				return "TerminalCmd";
			}		
	};
}
