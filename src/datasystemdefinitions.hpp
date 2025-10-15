// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#pragma once;

#ifdef DATASYSTEM_DLL
#ifdef __linux__
#define DLLDATASYSTEM __attribute__((visibility("default")))
#else
#define DLLDATASYSTEM __declspec(dllexport)
#endif
#else
#ifndef DATASYSTEM_LIB
#ifdef __linux__
#define DLLDATASYSTEM
#else
#define DLLDATASYSTEM __declspec(dllimport)
#endif
#else
#define DLLDATASYSTEM
#endif
#endif

#define REGISTER_DATA_TYPE(className, typeName)                                                                                                                                                                                                                                                  \
	auto *_reg_datatype_##typeName = new ds::__reg_datatype(#typeName, [](ds::Settings &dataSettings, const std::string &value) -> ds::Value * { return new className(dataSettings, value); });                                                                                                  \
	std::string className::GetTypeString() const { return #typeName; }
