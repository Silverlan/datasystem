/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "datasystem_vector.h"
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

REGISTER_DATA_TYPE(ds::Vector,Vector)
