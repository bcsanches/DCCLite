// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "MapObject.h"
#include "Panel.h"
#include "RailObject.h"

#include <stdexcept>

namespace LitePanel
{
	enum Layers
	{
		kTEMP_LAYER,
		kRAIL_LAYER,		

		kNUM_LAYERS
	};

	Panel::Panel(const TileCoord_t size, const char *name):
		m_mapTileMap(size, kNUM_LAYERS),
		m_strName{ name }
	{
		if (m_strName.empty())
			throw std::invalid_argument("[LitePanel::Panel] Name cannot be blank");
	}

	Panel::Panel(const rapidjson::Value& data) :
		m_mapTileMap(data["tileMap"]),
		m_strName{ data["name"].GetString() }
	{
		if (m_mapTileMap.GetNumLayers() != kNUM_LAYERS)
			throw std::invalid_argument("[Panel::Panel] File contains invalid number of layers");		
	}

	RailObject &Panel::RegisterRail(std::unique_ptr<RailObject> object)
	{		
#if 0
		if (auto *junction = dynamic_cast<JunctionRailObject *>(object.get()))
		{

		}
		else if (auto *simpleRail = dynamic_cast<SimpleRailObject *>(object.get()))
		{
			auto &mapSize = m_mapTileMap.GetSize();

			auto nt = simpleRail->GetNeighborTiles(mapSize);

			BlockIndex blockIndex;
			Block *block = nullptr;

			//Do we have a left neighbor tile?
			if ((simpleRail->GetBlockSplit() != BlockSplitTypes::LEFT) && nt.first)
			{
				auto leftObj = m_mapTileMap.TryGetMapObject(nt.first.value(), kRAIL_LAYER);
				auto leftRail = dynamic_cast<const SimpleRailObject *>(leftObj);

				//is left neighbor tile a rail object?
				if (leftRail)
				{
					//Is the left neighbor rail object connected to us and not a spliter?
					auto rightTile = leftRail->GetRightNeighbor(mapSize);
					if (rightTile && rightTile == object->GetPosition() && (leftRail->GetBlockSplit() != BlockSplitTypes::RIGHT))
					{
						//Yes, we are connected
						blockIndex = leftRail->GetBlockIndex();
						block = &m_clBlockManager.GetBlock(blockIndex);

						block->ExpandRight(rightTile.value());
					}
				}
			}

			if((simpleRail->GetBlockSplit() != BlockSplitTypes::RIGHT) && nt.second)
			{
				auto rightObj = m_mapTileMap.TryGetMapObject(nt.second.value(), kRAIL_LAYER);
				auto rightRail = dynamic_cast<const SimpleRailObject *>(rightObj);

				//is right neighbor a rail object?
				if (rightRail)
				{
					//Is the right neighbor connected to us and not a splitter?
					auto leftTile = rightRail->GetLeftNeighbor(mapSize);
					if (leftTile && leftTile == object->GetPosition() && (rightRail->GetBlockSplit() != BlockSplitTypes::LEFT))
					{
						//yes, we are connected...

						if (block)
						{
							//we already have a left block, so we got merge the blocks
							//TODO: left block will expand to the right block
							//follow all elements on block and set their block index
							assert(0);
						}
						else
						{
							blockIndex = rightRail->GetBlockIndex();
							block = &m_clBlockManager.GetBlock(blockIndex);

							block->ExpandLeft(leftTile.value());
						}
					}
				}
			}

			if (!blockIndex)
			{
				//create a new block
				auto pos = object->GetPosition();
				blockIndex = m_clBlockManager.AllocBlock(pos, pos);				
			}

			object->SetBlockIndex(blockIndex);
		}
#endif

		auto *ptr = object.get();

		m_mapTileMap.RegisterObject(std::move(object), kRAIL_LAYER);

		if (auto *junction = dynamic_cast<JunctionRailObject *>(ptr))
		{

		}
		else if (auto *simpleRail = dynamic_cast<SimpleRailObject *>(ptr))
		{
			auto &mapSize = m_mapTileMap.GetSize();

			auto nt = simpleRail->GetNeighborTiles(mapSize);
			const auto &railPosition = simpleRail->GetPosition();

			BlockIndex blockIndex;
			Block *block = nullptr;
			
			//do we have a left tile?
			if (nt.first)
			{
				auto leftObj = m_mapTileMap.TryGetMapObject(nt.first.value(), kRAIL_LAYER);				

				//Do we have a object on the left tile and is this object rail?
				if (auto leftRail = dynamic_cast<const SimpleRailObject *>(leftObj))
				{
					auto leftRailRightNeighborTile = leftRail->GetRightNeighbor(mapSize);

					//is the leftRail right neighbor the same as we are? 
					if (leftRailRightNeighborTile && (leftRailRightNeighborTile.value() == railPosition))
					{
						//Yes, the rail on the left side connects to us...						
						blockIndex = leftRail->GetBlockIndex();
						block = &m_clBlockManager.GetBlock(blockIndex);

						//expand block
						block->ExpandRight(railPosition);
					}
				}
			}

			if (!blockIndex)
			{
				//create a new block
				blockIndex = m_clBlockManager.AllocBlock(railPosition, railPosition);
			}

			simpleRail->SetBlockIndex(blockIndex);
		}

		return *ptr;
	}

	std::unique_ptr<RailObject> Panel::TryUnregisterRail(const TileCoord_t &position)
	{
		auto tmp = m_mapTileMap.TryUnregisterObject(position, kRAIL_LAYER);

		if (auto *simpleRail = dynamic_cast<SimpleRailObject *>(tmp.get()))
		{
			auto nt = simpleRail->GetNeighborTiles(m_mapTileMap.GetSize());

			if (nt.first)
			{
				auto leftNeighbor = m_mapTileMap.TryGetMapObject(nt.first.value(), kRAIL_LAYER);
				auto leftRail = dynamic_cast<const SimpleRailObject *>(leftNeighbor);
				if (leftRail)
				{

				}
			}


		}

		

		return std::unique_ptr<RailObject>(static_cast<RailObject *>(tmp.release()));
	}

	bool Panel::IsRailTileOccupied(const TileCoord_t &position) const noexcept
	{
		return m_mapTileMap.IsTileOccupied(position, kRAIL_LAYER);
	}

	void Panel::RegisterTempObject(std::unique_ptr<MapObject> object)
	{
		m_mapTileMap.RegisterObject(std::move(object), kTEMP_LAYER);
	}

	std::unique_ptr<MapObject> Panel::UnregisterTempObject(const MapObject& object)
	{
		return m_mapTileMap.UnregisterObject(object, kTEMP_LAYER);
	}

	void Panel::SetTempObjectPosition(MapObject& object, const TileCoord_t& newPosition)
	{		
		auto obj = this->UnregisterTempObject(object);
		
		obj->SetPosition(newPosition);

		this->RegisterTempObject(std::move(obj));
	}

	void Panel::Save(JsonOutputStream_t &stream) const
	{
		auto tileMapObj = stream.AddObject("tileMap");

		m_mapTileMap.Save(tileMapObj);

		stream.AddStringValue("name", m_strName);
	}

	void Panel::Load(const rapidjson::Value &data)
	{
		m_mapTileMap.Load(data["tileMap"]);

		m_strName = data["name"].GetString();
	}
}
