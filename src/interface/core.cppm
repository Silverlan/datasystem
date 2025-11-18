// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.datasystem:core;

export import pragma.filesystem;
export import pragma.math;

export {
#pragma warning(push)
#pragma warning(disable : 4251)
	namespace ds {
		enum class ValueType : uint8_t {
			Invalid,
			String,
			Int,
			Float,
			Bool,
			Color,
			Vector2,
			Vector3,
			Vector4,
			Texture,
			User,

			Count,
		};

		class String;
		class Int;
		class Float;
		class Bool;
		class Color;
		class Vector2;
		class Vector;
		class Vector4;

		class Settings;
		class Block;
		class Container;
		class DLLDATASYSTEM Base : public std::enable_shared_from_this<Base> {
		  protected:
			friend Container;
			friend Block;
			Base(Settings &dataSettings);
			std::shared_ptr<Settings> m_dataSettings = nullptr;
		  public:
			virtual bool IsBlock() const;
			virtual bool IsContainer() const;
			virtual bool IsValue() const { return false; }
			virtual ~Base();
			virtual Base *Copy();
			virtual std::shared_ptr<Block> GetBlock(const std::string_view &name, unsigned int id);
			std::shared_ptr<Block> GetBlock(const std::string_view &name);

			const Settings &GetDataSettings() const;
			Settings &GetDataSettings();
		};

		class DLLDATASYSTEM Iterator {
		  private:
			Base &m_target;
			unsigned int m_index;
			std::unordered_map<std::string, Base *> it;
		  public:
			Iterator(Base &data);
			bool IsValid();
			void operator++(int);
			Block *operator->();
			Block *get();
		};

		class DLLDATASYSTEM Value;
		class DLLDATASYSTEM Block : public Base {
		  public:
			using DataMap = std::unordered_map<std::string, std::shared_ptr<Base>, util::hl_string_hash, std::equal_to<>>;
		  private:
			DataMap m_data;

			template<typename T, class TDs>
			    requires(
			      std::is_same_v<TDs, ds::String> || std::is_same_v<TDs, ds::Int> || std::is_same_v<TDs, ds::Float> || std::is_same_v<TDs, ds::Bool> || std::is_same_v<TDs, ds::Color> || std::is_same_v<TDs, ds::Vector2> || std::is_same_v<TDs, ds::Vector> || std::is_same_v<TDs, ds::Vector4>)
			void AddValue(const std::string &name, const T &value, const char *typeName);
		  public:
			Block(Settings &dataSettings);
			virtual ~Block() override;
			virtual bool IsBlock() const override;
			const DataMap *GetData() const;
			void DetachData(Base &val);
			void RemoveValue(const std::string &key);
			bool IsEmpty() const;
			// Creates a copy of all data contained in this block
			Block *Copy() override;
			;
			std::string ToString(const std::optional<std::string> &rootIdentifier, uint8_t tabDepth = 0) const;
			virtual void AddData(const std::string &name, const std::shared_ptr<Base> &data);
			std::shared_ptr<ds::Base> AddValue(const std::string &type, const std::string &name, const std::string &value);

			template<typename T>
			    requires(std::is_arithmetic_v<T>)
			void AddValue(const std::string &name, const T &value)
			{
				if constexpr(std::is_same_v<std::remove_cvref_t<T>, bool>)
					AddValue<bool, ds::Bool>(name, value, "bool");
				else if constexpr(std::is_floating_point_v<T>)
					AddValue<float, ds::Float>(name, value, "float");
				else
					AddValue<int, ds::Int>(name, value, "int");
			}
			void AddValue(const std::string &name, const std::string &value);
			void AddValue(const std::string &name, const ::Color &value);
			void AddValue(const std::string &name, const ::Vector2 &value);
			void AddValue(const std::string &name, const ::Vector3 &value);
			void AddValue(const std::string &name, const ::Vector4 &value);
			template<typename T>
			    requires(std::is_enum_v<T>)
			void AddValue(const std::string &name, T value)
			{
				AddValue(name, std::string {magic_enum::enum_name(value)});
			}
			std::shared_ptr<Block> AddBlock(const std::string &name);
			const std::shared_ptr<Base> &GetValue(const std::string_view &key) const;
			std::shared_ptr<Value> GetDataValue(const std::string_view &key) const;
			std::shared_ptr<Block> GetBlock(const std::string_view &name, unsigned int id = 0) override;
			bool HasValue(const std::string_view &key) const;
			std::string GetString(const std::string_view &key, const std::string &def = "") const;
			int GetInt(const std::string_view &key, int def = 0) const;
			float GetFloat(const std::string_view &key, float def = 0.f) const;
			bool GetBool(const std::string_view &key, bool def = false) const;
			::Color GetColor(const std::string_view &key, const ::Color &def = colors::White) const;
			::Vector2 GetVector2(const std::string_view &key, const ::Vector2 &def = {}) const;
			::Vector3 GetVector3(const std::string_view &key, const ::Vector3 &def = {}) const;
			::Vector4 GetVector4(const std::string_view &key, const ::Vector4 &def = {}) const;

			bool GetString(const std::string_view &key, std::string *data) const;
			bool GetInt(const std::string_view &key, int *data) const;
			bool GetFloat(const std::string_view &key, float *data) const;
			bool GetBool(const std::string_view &key, bool *data) const;
			bool GetColor(const std::string_view &key, ::Color *data) const;
			bool GetVector2(const std::string_view &key, ::Vector2 *data) const;
			bool GetVector3(const std::string_view &key, ::Vector3 *data) const;
			bool GetVector4(const std::string_view &key, ::Vector4 *data) const;

			bool IsString(const std::string_view &key) const;
			bool IsInt(const std::string_view &key) const;
			bool IsFloat(const std::string_view &key) const;
			bool IsBool(const std::string_view &key) const;
			bool IsColor(const std::string_view &key) const;
			bool IsVector2(const std::string_view &key) const;
			bool IsVector3(const std::string_view &key) const;
			bool IsVector4(const std::string_view &key) const;

			bool GetRawString(const std::string_view &key, std::string *v) const;
			bool GetRawInt(const std::string_view &key, int *v) const;
			bool GetRawFloat(const std::string_view &key, float *v) const;
			bool GetRawBool(const std::string_view &key, bool *v) const;
			bool GetRawColor(const std::string_view &key, ::Color *v) const;
			bool GetRawVector2(const std::string_view &key, ::Vector2 *v) const;
			bool GetRawVector3(const std::string_view &key, ::Vector3 *v) const;
			bool GetRawVector4(const std::string_view &key, ::Vector4 *v) const;

			template<typename T>
			bool GetType(const std::string_view &key, T &outValue) const;

			template<class TType>
			bool IsType(const std::string_view &key) const;
			template<class TType>
			std::shared_ptr<TType> GetRawType(const std::string_view &key) const;
		};

		class DLLDATASYSTEM Container : public Base {
		  public:
			friend Block;
			virtual ~Container() override;
		  protected:
			std::vector<std::shared_ptr<Block>> m_dataBlocks;
		  public:
			Container(Settings &dataSettings);
			virtual bool IsContainer() const override;
			void AddData(const std::shared_ptr<Block> &data);
			std::shared_ptr<Block> GetBlock(unsigned int id = 0);
			std::vector<std::shared_ptr<Block>> &GetBlocks();
			uint32_t GetBlockCount() const;
		};

		class DLLDATASYSTEM Value : public Base {
		  protected:
			Value(Settings &dataSettings);
		  public:
			virtual std::string GetString() const = 0;
			virtual bool IsValue() const override { return true; }
			virtual std::string GetTypeString() const = 0;
			virtual ValueType GetType() const { return ValueType::User; }
			virtual int GetInt() const = 0;
			virtual float GetFloat() const = 0;
			virtual bool GetBool() const = 0;
			virtual ::Color GetColor() const = 0;
			virtual ::Vector3 GetVector() const = 0;
			virtual ::Vector2 GetVector2() const = 0;
			virtual ::Vector4 GetVector4() const = 0;
		};

		class DLLDATASYSTEM System {
		  public:
			static std::shared_ptr<Block> ReadData(ufile::IFile &f, const std::unordered_map<std::string, std::string> &enums = {});
			static std::shared_ptr<Block> LoadData(const char *path, const std::unordered_map<std::string, std::string> &enums = {});
		};

		class DLLDATASYSTEM ValueTypeMap {
		  private:
			std::unordered_map<std::string, std::function<Value *(Settings &, const std::string &)>> m_factories;
		  public:
			void AddFactory(const std::string &name, const std::function<Value *(Settings &, const std::string &)> &factory);
			std::function<Value *(Settings &, const std::string &)> FindFactory(const std::string &name);
		};

		DLLDATASYSTEM void register_data_value_type(const std::string &type, const std::function<Value *(Settings &, const std::string &)> &factory);
		template<typename T>
		void register_data_value_type(const std::string &type)
		{
			register_data_value_type(type, [](ds::Settings &dataSettings, const std::string &value) -> ds::Value * { return new T {dataSettings, value}; });
		}

		DLLDATASYSTEM void register_base_types();
		DLLDATASYSTEM ValueTypeMap *get_data_value_type_map();
		DLLDATASYSTEM std::shared_ptr<Settings> create_data_settings(const std::unordered_map<std::string, std::string> &enums);
		DLLDATASYSTEM void close();

		class DLLDATASYSTEM String : public Value {
		  public:
			String(Settings &dataSettings, const std::string &value);
			virtual Value *Copy() override;
			const std::string &GetValue() const;
			void SetValue(const std::string &value);

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
			std::string m_value;
		};

		class DLLDATASYSTEM Int : public Value {
		  public:
			Int(Settings &dataSettings, const std::string &value);
			Int(Settings &dataSettings, int32_t value);
			virtual Value *Copy() override;
			int32_t GetValue() const;
			void SetValue(int32_t value);

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
			int32_t m_value;
		};

		class DLLDATASYSTEM Float : public Value {
		  public:
			Float(Settings &dataSettings, const std::string &value);
			Float(Settings &dataSettings, float value);
			virtual Value *Copy() override;
			float GetValue() const;
			void SetValue(float value);

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
			float m_value;
		};

		class DLLDATASYSTEM Bool : public Value {
		  public:
			Bool(Settings &dataSettings, const std::string &value);
			Bool(Settings &dataSettings, bool value);
			virtual Value *Copy() override;
			bool GetValue() const;
			void SetValue(bool value);

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
			bool m_value;
		};
	};
#pragma warning(pop)

	namespace ds {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpotentially-evaluated-expression"
#endif
		template<class TType>
		bool Block::IsType(const std::string_view &key) const
		{
			auto v = GetDataValue(key);
			if(v == nullptr)
				return false;
			if(typeid(*v) != typeid(TType))
				return false;
			return true;
		}
#ifdef __clang__
#pragma clang diagnostic pop
#endif

		template<class TType>
		std::shared_ptr<TType> Block::GetRawType(const std::string_view &key) const
		{
			return std::dynamic_pointer_cast<TType>(GetDataValue(key));
		}
	}

	namespace ds {
		template<typename T, class TDs>
		    requires(
		      std::is_same_v<TDs, ds::String> || std::is_same_v<TDs, ds::Int> || std::is_same_v<TDs, ds::Float> || std::is_same_v<TDs, ds::Bool> || std::is_same_v<TDs, ds::Color> || std::is_same_v<TDs, ds::Vector2> || std::is_same_v<TDs, ds::Vector> || std::is_same_v<TDs, ds::Vector4>)
		void Block::AddValue(const std::string &name, const T &value, const char *typeName)
		{
			auto dsVal = GetDataValue(name);
			if(dsVal) {
				if(typeid(dsVal) == typeid(TDs)) {
					auto *f = static_cast<TDs *>(dsVal.get());
					f->SetValue(value);
					return;
				}
				RemoveValue(name);
			}
			if constexpr(std::is_same_v<TDs, ds::String>)
				AddValue(typeName, name, value);
			else if constexpr(std::is_same_v<TDs, ds::Int> || std::is_same_v<TDs, ds::Float> || std::is_same_v<TDs, ds::Bool>)
				AddValue(typeName, name, std::to_string(value));
			else if constexpr(std::is_same_v<TDs, ds::Color>)
				AddValue(typeName, name, std::to_string(value.r) + ' ' + std::to_string(value.g) + ' ' + std::to_string(value.b) + ' ' + std::to_string(value.a));
			else if constexpr(std::is_same_v<TDs, ds::Vector2>)
				AddValue(typeName, name, std::to_string(value.x) + ' ' + std::to_string(value.y));
			else if constexpr(std::is_same_v<TDs, ds::Vector>)
				AddValue(typeName, name, std::to_string(value.x) + ' ' + std::to_string(value.y) + ' ' + std::to_string(value.z));
			else if constexpr(std::is_same_v<TDs, ds::Vector4>)
				AddValue(typeName, name, std::to_string(value.x) + ' ' + std::to_string(value.y) + ' ' + std::to_string(value.z) + ' ' + std::to_string(value.w));
			else
				static_assert(false, "Unknown type");
		}
	};
}
