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

#include <vector>
#include <string>

#include <dcclite/FileSystem.h>
#include <dcclite/FolderObject.h>

#include "DccAddress.h"
#include "IDevice.h"
#include "Guid.h"

#include <rapidjson/document.h>

namespace dcclite::broker
{
	class Decoder;
	class IDccLite_DeviceServices;	

	class Device : public dcclite::FolderObject, IDevice_DecoderServices
	{
		public:
			Device(RName name, IDccLite_DeviceServices &dccService, const rapidjson::Value &params);
			Device(RName name, IDccLite_DeviceServices &dccService);	

			virtual ~Device();			

			//
			// IDeviceDEcoderServices
			//
			//

			RName GetDeviceName() const noexcept override
			{
				return this->GetName();
			}

			Decoder &CreateInternalDecoder(const char *className, DccAddress address, RName name, const rapidjson::Value &params);

			inline dcclite::Guid GetConfigToken() const noexcept
			{
				return m_ConfigToken;
			}

			void ConstVisitDecoders(ConstVisitor_t visitor) const;

		protected:
			void Load();
			void Unload();

			virtual void OnUnload();

			virtual void CheckLoadedDecoder(Decoder &decoder) = 0;	
			[[nodiscard]] virtual bool IsInternalDecoderAllowed() const noexcept = 0;

		private:
			void RegisterDecoder(Decoder &decoder);

		protected:
			std::vector<Decoder *>	m_vecDecoders;

			IDccLite_DeviceServices &m_clDccService;
	
			//
			//
			//Storage data
			const std::string		m_strConfigFileName;
			const dcclite::fs::path m_pathConfigFile;

			dcclite::Guid			m_ConfigToken;
	};

}