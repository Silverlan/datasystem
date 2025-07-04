// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __DATASYSTEM_COLOR_H__
#define __DATASYSTEM_COLOR_H__

#include "datasystem.h"
#include <mathutil/color.h>

namespace ds {
	class DLLDATASYSTEM Color : public Value {
	  public:
		Color(ds::Settings &dataSettings, const std::string &value);
		Color(ds::Settings &dataSettings, const ::Color &value);
		virtual Color *Copy() override;
		const ::Color &GetValue() const;
		void SetValue(const ::Color &value);

		virtual std::string GetString() const override;
		virtual std::string GetTypeString() const override;
		virtual ValueType GetType() const override;
		virtual int GetInt() const override;
		virtual float GetFloat() const override;
		virtual bool GetBool() const override;
		virtual ::Color GetColor() const override;
		virtual ::Vector3 GetVector() const override;
		virtual ::Vector2 GetVector2() const override;
		virtual ::Vector4 GetVector4() const override;
	  private:
		::Color m_value;
	};
};

#endif
