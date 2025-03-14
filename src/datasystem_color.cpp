/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "datasystem_color.h"
#include <sstream>

ds::Color::Color(ds::Settings &dataSettings, const std::string &value) : Value(dataSettings), m_value(value) {}
ds::Color::Color(ds::Settings &dataSettings, const ::Color &value) : Value(dataSettings), m_value(value) {}
ds::Color *ds::Color::Copy() { return new Color(*m_dataSettings, m_value); }
ds::ValueType ds::Color::GetType() const { return ValueType::Color; }
const Color &ds::Color::GetValue() const { return m_value; }
void ds::Color::SetValue(const ::Color &value) { m_value = value; }

std::string ds::Color::GetString() const
{
	std::stringstream ss;
	ss << m_value.r << " " << m_value.g << " " << m_value.b << " " << m_value.a;
	return ss.str();
}
int ds::Color::GetInt() const { return 0; }
float ds::Color::GetFloat() const { return 0.f; }
bool ds::Color::GetBool() const { return false; }
::Color ds::Color::GetColor() const { return m_value; }
::Vector3 ds::Color::GetVector() const { return m_value.ToVector3(); }
::Vector2 ds::Color::GetVector2() const
{
	auto v = m_value.ToVector3();
	return ::Vector2 {v.x, v.y};
}
::Vector4 ds::Color::GetVector4() const { return m_value.ToVector4(); }

REGISTER_DATA_TYPE(ds::Color, color)
