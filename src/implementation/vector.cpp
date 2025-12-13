// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

module pragma.datasystem;

import :vector;
import pragma.string;

pragma::datasystem::Vector::Vector(Settings &dataSettings, const std::string &value) : Value(dataSettings), m_value(uvec::create(value)) {}
pragma::datasystem::Vector::Vector(Settings &dataSettings, const Vector3 &value) : Value(dataSettings), m_value(value) {}
pragma::datasystem::Vector *pragma::datasystem::Vector::Copy() { return new Vector(*m_dataSettings, m_value); }
pragma::datasystem::ValueType pragma::datasystem::Vector::GetType() const { return ValueType::Vector3; }
const Vector3 &pragma::datasystem::Vector::GetValue() const { return m_value; }
void pragma::datasystem::Vector::SetValue(const Vector3 &value) { m_value = value; }
std::string pragma::datasystem::Vector::GetTypeString() const { return "vector"; }
std::string pragma::datasystem::Vector::GetString() const
{
	std::stringstream ss;
	ss << m_value.x << " " << m_value.y << " " << m_value.z;
	return ss.str();
}
int pragma::datasystem::Vector::GetInt() const { return 0; }
float pragma::datasystem::Vector::GetFloat() const { return 0.f; }
bool pragma::datasystem::Vector::GetBool() const { return false; }
Color pragma::datasystem::Vector::GetColor() const { return ::Color {m_value}; }
Vector3 pragma::datasystem::Vector::GetVector() const { return m_value; }
Vector2 pragma::datasystem::Vector::GetVector2() const { return ::Vector2 {m_value.x, m_value.y}; }
Vector4 pragma::datasystem::Vector::GetVector4() const { return ::Vector4 {m_value, 0.f}; }

/////////////

pragma::datasystem::Vector4::Vector4(Settings &dataSettings, const std::string &value) : Value(dataSettings) { pragma::string::string_to_array<::Vector4::value_type>(value, &m_value[0], pragma::string::cstring_to_number<float>, 4); }
pragma::datasystem::Vector4::Vector4(Settings &dataSettings, const ::Vector4 &value) : Value(dataSettings), m_value(value) {}
std::string pragma::datasystem::Vector4::GetTypeString() const { return "vector4"; }
pragma::datasystem::Vector4 *pragma::datasystem::Vector4::Copy() { return new Vector4(*m_dataSettings, m_value); }
pragma::datasystem::ValueType pragma::datasystem::Vector4::GetType() const { return ValueType::Vector4; }
const Vector4 &pragma::datasystem::Vector4::GetValue() const { return m_value; }
void pragma::datasystem::Vector4::SetValue(const ::Vector4 &value) { m_value = value; }

std::string pragma::datasystem::Vector4::GetString() const
{
	std::stringstream ss;
	ss << m_value[0] << " " << m_value[1] << " " << m_value[2] << " " << m_value[3];
	return ss.str();
}
int pragma::datasystem::Vector4::GetInt() const { return 0; }
float pragma::datasystem::Vector4::GetFloat() const { return 0.f; }
bool pragma::datasystem::Vector4::GetBool() const { return false; }
Color pragma::datasystem::Vector4::GetColor() const { return ::Color {m_value}; }
Vector3 pragma::datasystem::Vector4::GetVector() const { return m_value; }
Vector2 pragma::datasystem::Vector4::GetVector2() const { return {m_value.x, m_value.y}; }
Vector4 pragma::datasystem::Vector4::GetVector4() const { return m_value; }

/////////////

pragma::datasystem::Vector2::Vector2(Settings &dataSettings, const std::string &value) : Value(dataSettings) { pragma::string::string_to_array<::Vector2::value_type>(value, &m_value[0], pragma::string::cstring_to_number<float>, 2); }
pragma::datasystem::Vector2::Vector2(Settings &dataSettings, const ::Vector2 &value) : Value(dataSettings), m_value(value) {}
pragma::datasystem::Vector2 *pragma::datasystem::Vector2::Copy() { return new Vector2(*m_dataSettings, m_value); }
pragma::datasystem::ValueType pragma::datasystem::Vector2::GetType() const { return ValueType::Vector2; }
std::string pragma::datasystem::Vector2::GetTypeString() const { return "vector2"; }
const Vector2 &pragma::datasystem::Vector2::GetValue() const { return m_value; }
void pragma::datasystem::Vector2::SetValue(const ::Vector2 &value) { m_value = value; }

std::string pragma::datasystem::Vector2::GetString() const
{
	std::stringstream ss;
	ss << m_value[0] << " " << m_value[1];
	return ss.str();
}
int pragma::datasystem::Vector2::GetInt() const { return 0; }
float pragma::datasystem::Vector2::GetFloat() const { return 0.f; }
bool pragma::datasystem::Vector2::GetBool() const { return false; }
Color pragma::datasystem::Vector2::GetColor() const { return ::Color {Vector3 {m_value, 0}}; }
Vector3 pragma::datasystem::Vector2::GetVector() const { return Vector3 {m_value, 0}; }
Vector2 pragma::datasystem::Vector2::GetVector2() const { return m_value; }
Vector4 pragma::datasystem::Vector2::GetVector4() const { return ::Vector4 {m_value, 0, 0}; }
