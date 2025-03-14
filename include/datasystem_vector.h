/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __DATASYSTEM_VECTOR_H__
#define __DATASYSTEM_VECTOR_H__

#include "datasystem.h"
#include <mathutil/uvec.h>

namespace ds {
	class DLLDATASYSTEM Vector : public Value {
	  public:
		Vector(ds::Settings &dataSettings, const std::string &value);
		Vector(ds::Settings &dataSettings, const Vector3 &value);
		virtual Vector *Copy() override;
		const Vector3 &GetValue() const;
		void SetValue(const Vector3 &value);

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
		Vector3 m_value;
	};
	class DLLDATASYSTEM Vector4 : public Value {
	  public:
		Vector4(ds::Settings &dataSettings, const std::string &value);
		Vector4(ds::Settings &dataSettings, const ::Vector4 &value);
		virtual Vector4 *Copy() override;
		const ::Vector4 &GetValue() const;
		void SetValue(const ::Vector4 &value);

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
		::Vector4 m_value;
	};
	class DLLDATASYSTEM Vector2 : public Value {
	  public:
		Vector2(ds::Settings &dataSettings, const std::string &value);
		Vector2(ds::Settings &dataSettings, const ::Vector2 &value);
		virtual Vector2 *Copy() override;
		const ::Vector2 &GetValue() const;
		void SetValue(const ::Vector2 &value);

		virtual std::string GetString() const override;
		virtual std::string GetTypeString() const override;
		virtual ValueType GetType() const override;
		virtual int GetInt() const override;
		virtual float GetFloat() const override;
		virtual bool GetBool() const override;
		virtual ::Color GetColor() const override;
		virtual ::Vector2 GetVector2() const override;
		virtual ::Vector3 GetVector() const override;
		virtual ::Vector4 GetVector4() const override;
	  private:
		::Vector2 m_value;
	};
};

#endif
