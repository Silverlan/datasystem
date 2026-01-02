// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.datasystem:color;

export import :core;

export namespace pragma::datasystem {
	class DLLDATASYSTEM Color : public Value {
	  public:
		Color(Settings &dataSettings, const std::string &value);
		Color(Settings &dataSettings, const ::Color &value);
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
		virtual Vector3 GetVector() const override;
		virtual ::Vector2 GetVector2() const override;
		virtual ::Vector4 GetVector4() const override;
	  private:
		::Color m_value;
	};
};
