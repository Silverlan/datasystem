// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

module pragma.datasystem;

import :color;

pragma::datasystem::Color::Color(Settings &dataSettings, const std::string &value) : Value(dataSettings), m_value(value) {}
pragma::datasystem::Color::Color(Settings &dataSettings, const ::Color &value) : Value(dataSettings), m_value(value) {}
pragma::datasystem::Color *pragma::datasystem::Color::Copy() { return new Color(*m_dataSettings, m_value); }
pragma::datasystem::ValueType pragma::datasystem::Color::GetType() const { return ValueType::Color; }
const Color &pragma::datasystem::Color::GetValue() const { return m_value; }
void pragma::datasystem::Color::SetValue(const ::Color &value) { m_value = value; }
std::string pragma::datasystem::Color::GetTypeString() const { return "color"; }

std::string pragma::datasystem::Color::GetString() const
{
	std::stringstream ss;
	ss << m_value.r << " " << m_value.g << " " << m_value.b << " " << m_value.a;
	return ss.str();
}
int pragma::datasystem::Color::GetInt() const { return 0; }
float pragma::datasystem::Color::GetFloat() const { return 0.f; }
bool pragma::datasystem::Color::GetBool() const { return false; }
Color pragma::datasystem::Color::GetColor() const { return m_value; }
Vector3 pragma::datasystem::Color::GetVector() const { return m_value.ToVector3(); }
Vector2 pragma::datasystem::Color::GetVector2() const
{
	auto v = m_value.ToVector3();
	return ::Vector2 {v.x, v.y};
}
Vector4 pragma::datasystem::Color::GetVector4() const { return m_value.ToVector4(); }
