/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __DATASYSTEM_VECTOR_H__
#define __DATASYSTEM_VECTOR_H__

#include "datasystem.h"
#include <mathutil/uvec.h>

namespace ds
{
	class DLLDATASYSTEM Vector
		: public Value
	{
	public:
		Vector(ds::Settings &dataSettings,const std::string &value);
		Vector(ds::Settings &dataSettings,const Vector3 &value);
		virtual Vector *Copy() override;
		const Vector3 &GetValue() const;

		virtual std::string GetString() const override;
		virtual std::string GetTypeString() const override;
		virtual int GetInt() const override;
		virtual float GetFloat() const override;
		virtual bool GetBool() const override;
	private:
		Vector3 m_value;
	};
};

#endif
