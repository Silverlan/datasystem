// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#pragma once

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
