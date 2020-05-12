/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "datasystem_vector.h"
#include <sharedutils/util_string.h>
#include <sstream>

ds::Vector::Vector(ds::Settings &dataSettings,const std::string &value)
	: Value(dataSettings),m_value(uvec::create(value))
{}
ds::Vector::Vector(ds::Settings &dataSettings,const Vector3 &value)
	: Value(dataSettings),m_value(value)
{}
ds::Vector *ds::Vector::Copy() {return new Vector(*m_dataSettings,m_value);}
const Vector3 &ds::Vector::GetValue() const {return m_value;}

std::string ds::Vector::GetString() const
{
	std::stringstream ss;
	ss<<m_value.x<<" "<<m_value.y<<" "<<m_value.z;
	return ss.str();
}
int ds::Vector::GetInt() const {return 0;}
float ds::Vector::GetFloat() const {return 0.f;}
bool ds::Vector::GetBool() const {return false;}

REGISTER_DATA_TYPE(ds::Vector,vector)

/////////////

ds::Vector4::Vector4(ds::Settings &dataSettings,const std::string &value)
	: Value(dataSettings)
{
	ustring::string_to_array<glm::vec4::value_type,Double>(value,&m_value[0],atof,4);
}
ds::Vector4::Vector4(ds::Settings &dataSettings,const ::Vector4 &value)
	: Value(dataSettings),m_value(value)
{}
ds::Vector4 *ds::Vector4::Copy() {return new Vector4(*m_dataSettings,m_value);}
const ::Vector4 &ds::Vector4::GetValue() const {return m_value;}

std::string ds::Vector4::GetString() const
{
	std::stringstream ss;
	ss<<m_value[0]<<" "<<m_value[1]<<" "<<m_value[2]<<" "<<m_value[3];
	return ss.str();
}
int ds::Vector4::GetInt() const {return 0;}
float ds::Vector4::GetFloat() const {return 0.f;}
bool ds::Vector4::GetBool() const {return false;}

REGISTER_DATA_TYPE(ds::Vector4,vector4)
