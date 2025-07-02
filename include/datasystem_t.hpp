// SPDX-FileCopyrightText: © 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __DATASYSTEM_T_HPP__
#define __DATASYSTEM_T_HPP__

#include "datasystem.h"
#include "datasystem_color.h"
#include "datasystem_vector.h"

namespace ds {
	template<typename T, class TDs>
	    requires(std::is_same_v<TDs, ds::String> || std::is_same_v<TDs, ds::Int> || std::is_same_v<TDs, ds::Float> || std::is_same_v<TDs, ds::Bool> || std::is_same_v<TDs, ds::Color> || std::is_same_v<TDs, ds::Vector2> || std::is_same_v<TDs, ds::Vector> || std::is_same_v<TDs, ds::Vector4>)
	void Block::AddValue(const std::string &name, const T &value, const char *typeName)
	{
		auto dsVal = GetDataValue(name);
		if(dsVal) {
			if(typeid(dsVal) == typeid(TDs)) {
				auto *f = static_cast<TDs *>(dsVal.get());
				f->SetValue(value);
				return;
			}
			RemoveValue(name);
		}
		if constexpr(std::is_same_v<TDs, ds::String>)
			AddValue(typeName, name, value);
		else if constexpr(std::is_same_v<TDs, ds::Int> || std::is_same_v<TDs, ds::Float> || std::is_same_v<TDs, ds::Bool>)
			AddValue(typeName, name, std::to_string(value));
		else if constexpr(std::is_same_v<TDs, ds::Color>)
			AddValue(typeName, name, std::to_string(value.r) + ' ' + std::to_string(value.g) + ' ' + std::to_string(value.b) + ' ' + std::to_string(value.a));
		else if constexpr(std::is_same_v<TDs, ds::Vector2>)
			AddValue(typeName, name, std::to_string(value.x) + ' ' + std::to_string(value.y));
		else if constexpr(std::is_same_v<TDs, ds::Vector>)
			AddValue(typeName, name, std::to_string(value.x) + ' ' + std::to_string(value.y) + ' ' + std::to_string(value.z));
		else if constexpr(std::is_same_v<TDs, ds::Vector4>)
			AddValue(typeName, name, std::to_string(value.x) + ' ' + std::to_string(value.y) + ' ' + std::to_string(value.z) + ' ' + std::to_string(value.w));
		else
			static_assert(false, "Unknown type");
	}
};

#endif
