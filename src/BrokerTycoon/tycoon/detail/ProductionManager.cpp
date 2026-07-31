#include "ProductionManager.h"

#include <algorithm>
#include <numeric>
#include <random>

#include <fmt/format.h>

#include <dcclite/FmtUtils.h>
#include <dcclite/Log.h>

#include "../Cargo.h"

static thread_local std::mt19937 g_clRandomGenerator{ std::random_device{}() };

namespace dcclite::broker::tycoon::detail
{
	void ProductionManager::LoadProduce(TycoonService &tycoon, const rapidjson::Value &params)
	{
		m_vecProduces.emplace_back(tycoon, params);
	}

	void ProductionManager::AdjustProductionChances()
	{
		m_uTotalChance = 0;
		for (auto i = 0; i < m_vecProduces.size(); ++i)
		{
			m_vecProduces[i].SetCumulativeChance(m_vecProduces[i].GetChance() + m_uTotalChance);
			m_uTotalChance += m_vecProduces[i].GetChance();
		}
	}

	void ProductionManager::Load(TycoonService &tycoon, const rapidjson::Value &params, const RName industryName)
	{
		auto singleProduce = params.FindMember("produce");
		if (singleProduce != params.MemberEnd())
		{
			if (!singleProduce->value.IsObject())
			{
				throw std::invalid_argument(fmt::format("[ProductionManager::Load] [{}]: produce must be an object", industryName));
			}

			this->LoadProduce(tycoon, singleProduce->value);
		}

		auto producesValue = params.FindMember("produces");
		if (producesValue == params.MemberEnd())
		{
			if (!m_vecProduces.empty())
			{
				return;
			}

			throw std::invalid_argument(fmt::format("[ProductionManager::Load] [{}]: either produce or produces must be specified", industryName));
		}

		if (producesValue->value.IsObject())
		{
			//redudant... but...
			this->LoadProduce(tycoon, producesValue->value);

			return;
		}

		if (!producesValue->value.IsArray())
		{
			throw std::invalid_argument(fmt::format("[ProductionManager::Load] [{}]: produces must be either an object or an array", industryName));
		}

		auto producesData = producesValue->value.GetArray();
		m_vecProduces.reserve(producesData.Size());
		for (auto &it : producesData)
		{
			if (!it.IsObject())
			{
				throw std::invalid_argument(fmt::format("[ProductionManager::Load] [{}]: each produce in produces array must be an object", industryName));
			}

			this->LoadProduce(tycoon, it);
		}

		if (m_vecProduces.empty())
		{
			throw std::invalid_argument(fmt::format("[ProductionManager::Load] [{}]: at least one produce must be specified", industryName));
		}

		//
		//make sure we have a single type
		for (size_t i = 0, sz = m_vecProduces.size() - 1; i < sz; ++i)
		{
			auto it = std::find_if(
				m_vecProduces.begin() + i + 1,
				m_vecProduces.end(),
				[this, i](const detail::CargoInfo &ci)
				{
					return &ci.GetCargo() == &m_vecProduces[i].GetCargo();
				}
			);

			if (it != m_vecProduces.end())
			{
				throw std::invalid_argument(fmt::format("[ProductionManager::Load] [{}]: multiple produces with the same cargo {} are not allowed", industryName, m_vecProduces[i].GetCargo().GetName()));
			}
		}

		this->AdjustProductionChances();
	}

	unsigned ProductionManager::CalculateTotalCargoStored() const noexcept
	{
		return std::accumulate(
			m_vecProduces.begin(),
			m_vecProduces.end(),
			0u,
			[](unsigned acc, const detail::CargoInfo &info)
			{
				return acc + info.GetTotal();
			}
		);
	}

	const Cargo *ProductionManager::TryGetCargoByCargoInfoIndex(CargoIndex index) const noexcept
	{
		if (index.GetIndex() >= m_vecProduces.size())
			return nullptr;

		return &m_vecProduces[index.GetIndex()].GetCargo();
	}

	std::optional<CargoIndex> ProductionManager::TryGetCargoInfoIndexByCargoName(RName rname) const noexcept
	{
		auto it = std::ranges::find_if(
			m_vecProduces,
			[rname](const detail::CargoInfo &ci) { return ci.GetCargo().GetName() == rname; }
		);

		if (it == m_vecProduces.end())
			return std::nullopt;

		return CargoIndex(static_cast<int>(std::distance(m_vecProduces.begin(), it)));
	}

	std::optional<CargoIndex> ProductionManager::TryGetCargoInfoIndexByCargoName(std::string_view name) const noexcept
	{
		RName cargoName = RName::TryGetName(name);
		if (!cargoName)
			return std::nullopt;

		return this->TryGetCargoInfoIndexByCargoName(cargoName);
	}

	CargoIndex ProductionManager::RandomSelectCargoToProduce(RName industryName) const noexcept
	{
		if (m_vecProduces.size() == 1)
			return CargoIndex(0);

		std::uniform_int_distribution<> dist(0, m_uTotalChance - 1);

		unsigned randomChance = dist(g_clRandomGenerator);

		//CDF — Cumulative Distribution Function
		for (unsigned i = 0, lowerBound = 0; i < m_vecProduces.size(); ++i)
		{
			auto &cargoInfo = m_vecProduces[i];
			auto cumulativeChance = cargoInfo.GetCumulativeChance();
			if ((randomChance >= lowerBound) && (randomChance < cumulativeChance))
				return CargoIndex(i);

			lowerBound = cumulativeChance;
		}

		dcclite::Log::Error("[Tycoon::ProductionManager::RandomSelectCargoToProduce] [{}]: Could not find cargo", industryName);

		std::uniform_int_distribution<> safeDist(0, (int)m_vecProduces.size() - 1);

		return CargoIndex(safeDist(g_clRandomGenerator));
	}

	CargoIndex ProductionManager::FindCargoInfoIndexByCargoName(RName cargoName, RName industryName) const
	{
		auto it = std::ranges::find_if(
			m_vecProduces,
			[cargoName](const detail::CargoInfo &ci) { return ci.GetCargo().GetName() == cargoName; }
		);

		if (it == m_vecProduces.end())
			throw std::runtime_error(fmt::format("[ProductionManager::FindCargoInfoIndexByCargoName] [{}]: Cargo {} not found", industryName, cargoName));

		return CargoIndex(std::distance(m_vecProduces.begin(), it));
	}

	CargoQuantity ProductionManager::GetCargoQuantity(RName cargoName, RName industryName) const
	{
		auto index = this->FindCargoInfoIndexByCargoName(cargoName, industryName);

		return { m_vecProduces[index.GetIndex()].GetQuantity(), m_vecProduces[index.GetIndex()].GetReservedQuantity()};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	// ProductionManager Serialization
	//
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ProductionManager::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		auto cargoInfoData = stream.AddArray("produces");
		for (auto &it : m_vecProduces)
		{
			auto obj = cargoInfoData.AddObject();

			it.Serialize(obj);
		}
	}

	void ProductionManager::SerializeDelta(dcclite::JsonOutputStream_t &stream) const
	{
		auto cargoInfoData = stream.AddArray("produces");
		for (size_t i = 0, sz = m_vecProduces.size(); i < sz; ++i)
		{
			auto obj = cargoInfoData.AddObject();

			m_vecProduces[i].SerializeDelta(obj);
		}
	}

	void ProductionManager::SerializeCargoInfoDelta(dcclite::JsonOutputStream_t &stream, CargoIndex cargoInfoIndex) const
	{
		m_vecProduces[cargoInfoIndex.GetIndex()].SerializeDelta(stream);
		stream.AddInt64Value("index", cargoInfoIndex.GetIndex());
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	// ProductionManager State
	//
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ProductionManager::SaveState(dcclite::JsonOutputStream_t &stream) const
	{
		for (size_t i = 0, sz = m_vecProduces.size(); i < sz; ++i)
		{
			auto produceData = stream.AddObject(m_vecProduces[i].GetCargo().GetName().GetData());

			m_vecProduces[i].SaveState(produceData);
		}
	}

	bool ProductionManager::LoadState(const rapidjson::Value &params)
	{
		for (auto &it : params.GetObject())
		{
			auto cargoNameStr = std::string_view{ it.name.GetString(), it.name.GetStringLength() };
			auto cargoName = RName::TryGetName(cargoNameStr);
			if (!cargoName)
			{
				dcclite::Log::Warn("[ProductionManager::LoadState] Cargo {} name is not even registered, skipping", cargoNameStr);

				//This is not so bad, we can live with that, maybe user removed a product from industry after the state was saved...
				continue;
			}

			if (auto cargoInfoIndex = this->TryGetCargoInfoIndexByCargoName(cargoName))
			{
				m_vecProduces[cargoInfoIndex.value().GetIndex()].LoadState(it.value);
				
				continue;
			}
			
			//This is not so bad, we can live with that, maybe user removed a product from industry after the state was saved...
			dcclite::Log::Warn("[ProductionManager::LoadState] Cargo {} in production state not found, skipping", cargoName);			
		}

		//If all production loaded, lets check if user added a new product to this industry
		for (size_t i = 0, sz = m_vecProduces.size(); i < sz; ++i)
		{
			auto &cargoInfo = m_vecProduces[i];
			auto cargoName = cargoInfo.GetCargo().GetNameData();
			if (!params.HasMember(cargoName.data()))
			{
				dcclite::Log::Error("[ProductionManager::LoadState] Cargo {} was not found in production state", cargoName);

				return false;
			}
		}

		return true;
	}

	void ProductionManager::ResetState()
	{
		std::ranges::for_each(m_vecProduces, [](detail::CargoInfo &ci) { ci.Reset(); });
	}
}
