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
#include <string_view>
#include <variant>

#include "Object.h"

#include <rapidjson/document.h>

namespace dcclite::broker
{ 

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
	class TerminalCmd;


	/**

	The terminal context is a little helper to store the cmd current location, last acessed object etc


	*/
	class TerminalContext
	{
		public:		
			explicit TerminalContext(dcclite::FolderObject &root):
				m_rclRoot(root)
			{
				//empty
			}

			TerminalContext(const TerminalContext &) = delete;

			TerminalContext(TerminalContext &&other) noexcept:
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

			/**

				returns:
					The item pointed by the m_pthLocation

			*/
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


	/**
		The cmd host is responsbible for storing the registered cmds

	*/
	class TerminalCmdHost: public dcclite::FolderObject
	{
		public:		
			TerminalCmdHost();
			virtual ~TerminalCmdHost();

			virtual IObject *AddChild(std::unique_ptr<IObject> obj);
			TerminalCmd *AddCmd(std::unique_ptr<TerminalCmd> cmd);
			void AddAlias(std::string name, TerminalCmd &target);

			TerminalCmd *TryFindCmd(std::string_view name);

			virtual const char *GetTypeName() const noexcept
			{
				return "TerminalCmdHost";
			}		
	};

	class TerminalCmdFiber
	{
		public:
			typedef JsonCreator::Object<JsonCreator::StringWriter> Result_t;

		public:
			TerminalCmdFiber(const CmdId_t id) :
				m_tId(id)
			{
				//empty
			}

			virtual bool Run(TerminalContext& context) noexcept = 0;

		protected:
			const CmdId_t m_tId;

	};


	/**

	A cmd that can be invoked thought terminals

	*/
	class TerminalCmd: public dcclite::IObject
	{
		public:
			typedef JsonCreator::Object<JsonCreator::StringWriter> Result_t;
			typedef std::variant<std::string, std::unique_ptr<TerminalCmdFiber>> CmdResult_t;

		public:
			TerminalCmd(std::string name);

			TerminalCmd(const TerminalCmd &) = delete;
			TerminalCmd(TerminalCmd &&) = delete;

			~TerminalCmd();

			[[nodiscard]]
			virtual CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) = 0;

			virtual const char *GetTypeName() const noexcept
			{
				return "TerminalCmd";
			}
	};
}