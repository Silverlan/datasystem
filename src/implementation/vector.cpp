// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

module pragma.datasystem;

import :vector;
import pragma.string;

ds::Vector::Vector(ds::Settings &dataSettings, const std::string &value) : Value(dataSettings), m_value(uvec::create(value)) {}
ds::Vector::Vector(ds::Settings &dataSettings, const Vector3 &value) : Value(dataSettings), m_value(value) {}
ds::Vector *ds::Vector::Copy() { return new Vector(*m_dataSettings, m_value); }
ds::ValueType ds::Vector::GetType() const { return ValueType::Vector3; }
const Vector3 &ds::Vector::GetValue() const { return m_value; }
void ds::Vector::SetValue(const Vector3 &value) { m_value = value; }

std::string ds::Vector::GetString() const
{
	std::stringstream ss;
	ss << m_value.x << " " << m_value.y << " " << m_value.z;
	return ss.str();
}
int ds::Vector::GetInt() const { return 0; }
float ds::Vector::GetFloat() const { return 0.f; }
bool ds::Vector::GetBool() const { return false; }
::Color ds::Vector::GetColor() const { return ::Color {m_value}; }
::Vector3 ds::Vector::GetVector() const { return m_value; }
::Vector2 ds::Vector::GetVector2() const { return ::Vector2 {m_value.x, m_value.y}; }
::Vector4 ds::Vector::GetVector4() const { return ::Vector4 {m_value, 0.f}; }

REGISTER_DATA_TYPE(ds::Vector, vector)

/////////////

ds::Vector4::Vector4(ds::Settings &dataSettings, const std::string &value) : Value(dataSettings) { ustring::string_to_array<::Vector4::value_type, Double>(value, &m_value[0], atof, 4); }
ds::Vector4::Vector4(ds::Settings &dataSettings, const ::Vector4 &value) : Value(dataSettings), m_value(value) {}
ds::Vector4 *ds::Vector4::Copy() { return new Vector4(*m_dataSettings, m_value); }
ds::ValueType ds::Vector4::GetType() const { return ValueType::Vector4; }
const ::Vector4 &ds::Vector4::GetValue() const { return m_value; }
void ds::Vector4::SetValue(const ::Vector4 &value) { m_value = value; }

std::string ds::Vector4::GetString() const
{
	std::stringstream ss;
	ss << m_value[0] << " " << m_value[1] << " " << m_value[2] << " " << m_value[3];
	return ss.str();
}
int ds::Vector4::GetInt() const { return 0; }
float ds::Vector4::GetFloat() const { return 0.f; }
bool ds::Vector4::GetBool() const { return false; }
::Color ds::Vector4::GetColor() const { return ::Color {m_value}; }
::Vector3 ds::Vector4::GetVector() const { return m_value; }
::Vector2 ds::Vector4::GetVector2() const { return {m_value.x, m_value.y}; }
::Vector4 ds::Vector4::GetVector4() const { return m_value; }

REGISTER_DATA_TYPE(ds::Vector4, vector4)

/////////////

ds::Vector2::Vector2(ds::Settings &dataSettings, const std::string &value) : Value(dataSettings) { ustring::string_to_array<::Vector2::value_type, Double>(value, &m_value[0], atof, 2); }
ds::Vector2::Vector2(ds::Settings &dataSettings, const ::Vector2 &value) : Value(dataSettings), m_value(value) {}
ds::Vector2 *ds::Vector2::Copy() { return new Vector2(*m_dataSettings, m_value); }
ds::ValueType ds::Vector2::GetType() const { return ValueType::Vector2; }
const ::Vector2 &ds::Vector2::GetValue() const { return m_value; }
void ds::Vector2::SetValue(const ::Vector2 &value) { m_value = value; }

std::string ds::Vector2::GetString() const
{
	std::stringstream ss;
	ss << m_value[0] << " " << m_value[1];
	return ss.str();
}
int ds::Vector2::GetInt() const { return 0; }
float ds::Vector2::GetFloat() const { return 0.f; }
bool ds::Vector2::GetBool() const { return false; }
::Color ds::Vector2::GetColor() const { return ::Color {::Vector3 {m_value, 0}}; }
::Vector3 ds::Vector2::GetVector() const { return ::Vector3 {m_value, 0}; }
::Vector2 ds::Vector2::GetVector2() const { return m_value; }
::Vector4 ds::Vector2::GetVector4() const { return ::Vector4 {m_value, 0, 0}; }

REGISTER_DATA_TYPE(ds::Vector2, vector2)
