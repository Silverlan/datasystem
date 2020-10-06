/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __DATASYSTEM_H__
#define __DATASYSTEM_H__
#include <unordered_map>
#include <string>
#include <vector>
#include <typeinfo>
#include <memory>
#include <functional>
#include <optional>
#include <mathutil/color.h>
#include <mathutil/uvec.h>

#ifdef DATASYSTEM_DLL
	#ifdef __linux__
		#define DLLDATASYSTEM __attribute__((visibility("default")))
	#else
		#define DLLDATASYSTEM __declspec(dllexport)
	#endif
#else
	#ifndef DATASYSTEM_LIB
		#ifdef __linux__
			#define DLLDATASYSTEM
		#else
			#define DLLDATASYSTEM __declspec(dllimport)
		#endif
	#else
		#define DLLDATASYSTEM
	#endif
#endif

#pragma warning(push)
#pragma warning(disable : 4251)
class VFilePtrInternal;
namespace ds
{
	class Settings;
	class Block;
	class Container;
	class DLLDATASYSTEM Base
		: public std::enable_shared_from_this<Base>
	{
	protected:
		friend Container;
		friend Block;
		Base(Settings &dataSettings);
		std::shared_ptr<Settings> m_dataSettings = nullptr;
	public:
		virtual bool IsBlock() const;
		virtual bool IsContainer() const;
		virtual ~Base();
		virtual Base *Copy();
		virtual std::shared_ptr<Block> GetBlock(const std::string &name,unsigned int id);
		std::shared_ptr<Block> GetBlock(const std::string &name);

		const Settings &GetDataSettings() const;
		Settings &GetDataSettings();
	};

	class DLLDATASYSTEM Iterator
	{
	private:
		Base &m_target;
		unsigned int m_index;
		std::unordered_map<std::string,Base*> it;
	public:
		Iterator(Base &data);
		bool IsValid();
		void operator++(int);
		Block *operator->();
		Block *get();
	};

	class DLLDATASYSTEM Value;
	class DLLDATASYSTEM Block
		: public Base
	{
	private:
		std::unordered_map<std::string,std::shared_ptr<Base>> m_data;
	public:
		Block(Settings &dataSettings);
		virtual ~Block() override;
		virtual bool IsBlock() const override;
		const std::unordered_map<std::string,std::shared_ptr<Base>> *GetData() const;
		void DetachData(Base &val);
		void RemoveValue(const std::string &key);
		bool IsEmpty() const;
		// Creates a copy of all data contained in this block
		Block *Copy();
		std::string ToString(const std::optional<std::string> &rootIdentifier,uint8_t tabDepth=0) const;
		virtual void AddData(const std::string &name,const std::shared_ptr<Base> &data);
		std::shared_ptr<ds::Base> AddValue(const std::string &type,const std::string &name,const std::string &value);
		std::shared_ptr<Block> AddBlock(const std::string &name);
		const std::shared_ptr<Base> &GetValue(const std::string &key) const;
		std::shared_ptr<Value> GetDataValue(const std::string &key) const;
		std::shared_ptr<Block> GetBlock(const std::string &name,unsigned int id=0) override;
		bool HasValue(const std::string &key) const;
		std::string GetString(const std::string &key,const std::string &default="") const;
		int GetInt(const std::string &key,int default=0) const;
		float GetFloat(const std::string &key,float default=0.f) const;
		bool GetBool(const std::string &key,bool default=false) const;
		::Color GetColor(const std::string &key,const ::Color &default=::Color::White) const;
		::Vector3 GetVector3(const std::string &key,const ::Vector3 &default={}) const;
		::Vector4 GetVector4(const std::string &key,const ::Vector4 &default={}) const;

		bool GetString(const std::string &key,std::string *data) const;
		bool GetInt(const std::string &key,int *data) const;
		bool GetFloat(const std::string &key,float *data) const;
		bool GetBool(const std::string &key,bool *data) const;
		bool GetColor(const std::string &key,::Color *data) const;
		bool GetVector3(const std::string &key,::Vector3 *data) const;
		bool GetVector4(const std::string &key,::Vector4 *data) const;

		bool IsString(const std::string &key) const;
		bool IsInt(const std::string &key) const;
		bool IsFloat(const std::string &key) const;
		bool IsBool(const std::string &key) const;
		bool IsColor(const std::string &key) const;
		bool IsVector3(const std::string &key) const;
		bool IsVector4(const std::string &key) const;

		bool GetRawString(const std::string &key,std::string *v) const;
		bool GetRawInt(const std::string &key,int *v) const;
		bool GetRawFloat(const std::string &key,float *v) const;
		bool GetRawBool(const std::string &key,bool *v) const;
		bool GetRawColor(const std::string &key,::Color *v) const;
		bool GetRawVector3(const std::string &key,::Vector3 *v) const;
		bool GetRawVector4(const std::string &key,::Vector4 *v) const;

		template<typename T>
			bool GetType(const std::string &key,T &outValue) const;

		template<class TType>
			bool IsType(const std::string &key) const;
		template<class TType>
			std::shared_ptr<TType> GetRawType(const std::string &key) const;
	};

	class DLLDATASYSTEM Container
		: public Base
	{
	public:
		friend Block;
		virtual ~Container() override;
	protected:
		Container(Settings &dataSettings);
		std::vector<std::shared_ptr<Block>> m_dataBlocks;
	public:
		virtual bool IsContainer() const override;
		void AddData(const std::shared_ptr<Block> &data);
		std::shared_ptr<Block> GetBlock(unsigned int id=0);
		std::vector<std::shared_ptr<Block>> &GetBlocks();
		uint32_t GetBlockCount() const;
	};

	class DLLDATASYSTEM Value
		: public Base
	{
	protected:
		Value(Settings &dataSettings);
	public:
		virtual std::string GetString() const=0;
		virtual std::string GetTypeString() const=0;
		virtual int GetInt() const=0;
		virtual float GetFloat() const=0;
		virtual bool GetBool() const=0;
		virtual ::Color GetColor() const=0;
		virtual ::Vector3 GetVector() const=0;
		virtual ::Vector4 GetVector4() const=0;
	};

	class DLLDATASYSTEM System
	{
	public:
		static std::shared_ptr<Block> ReadData(std::shared_ptr<VFilePtrInternal> f,const std::unordered_map<std::string,std::string> &enums={});
		static std::shared_ptr<Block> LoadData(const char *path,const std::unordered_map<std::string,std::string> &enums={});
	};

	class DLLDATASYSTEM ValueTypeMap
	{
	private:
		std::unordered_map<std::string,std::function<Value*(Settings&,const std::string&)>> m_factories;
	public:
		void AddFactory(const std::string &name,const std::function<Value*(Settings&,const std::string&)> &factory);
		std::function<Value*(Settings&,const std::string&)> FindFactory(const std::string &name);
	};

	DLLDATASYSTEM void register_data_value_type(const std::string &type,const std::function<Value*(Settings&,const std::string&)> &factory);
	DLLDATASYSTEM ValueTypeMap *get_data_value_type_map();
	DLLDATASYSTEM std::shared_ptr<Settings> create_data_settings(const std::unordered_map<std::string,std::string> &enums);

	class DLLDATASYSTEM __reg_datatype
	{
	public:
		__reg_datatype(const std::string &name,const std::function<Value*(Settings&,const std::string&)> &factory)
		{
			register_data_value_type(name,factory);
			delete this;
		}
	};

	class DLLDATASYSTEM String
		: public Value
	{
	public:
		String(Settings &dataSettings,const std::string &value);
		virtual Value *Copy() override;
		const std::string &GetValue() const;

		virtual std::string GetString() const override;
		virtual std::string GetTypeString() const override;
		virtual int GetInt() const override;
		virtual float GetFloat() const override;
		virtual bool GetBool() const override;
		virtual ::Color GetColor() const override;
		virtual ::Vector3 GetVector() const override;
		virtual ::Vector4 GetVector4() const override;
	private:
		std::string m_value;
	};

	class DLLDATASYSTEM Int
		: public Value
	{
	public:
		Int(Settings &dataSettings,const std::string &value);
		Int(Settings &dataSettings,int32_t value);
		virtual Value *Copy() override;
		int32_t GetValue() const;

		virtual std::string GetString() const override;
		virtual std::string GetTypeString() const override;
		virtual int GetInt() const override;
		virtual float GetFloat() const override;
		virtual bool GetBool() const override;
		virtual ::Color GetColor() const override;
		virtual ::Vector3 GetVector() const override;
		virtual ::Vector4 GetVector4() const override;
	private:
		int32_t m_value;
	};

	class DLLDATASYSTEM Float
		: public Value
	{
	public:
		Float(Settings &dataSettings,const std::string &value);
		Float(Settings &dataSettings,float value);
		virtual Value *Copy() override;
		float GetValue() const;

		virtual std::string GetString() const override;
		virtual std::string GetTypeString() const override;
		virtual int GetInt() const override;
		virtual float GetFloat() const override;
		virtual bool GetBool() const override;
		virtual ::Color GetColor() const override;
		virtual ::Vector3 GetVector() const override;
		virtual ::Vector4 GetVector4() const override;
	private:
		float m_value;
	};

	class DLLDATASYSTEM Bool
		: public Value
	{
	public:
		Bool(Settings &dataSettings,const std::string &value);
		Bool(Settings &dataSettings,bool value);
		virtual Value *Copy() override;
		bool GetValue() const;

		virtual std::string GetString() const override;
		virtual std::string GetTypeString() const override;
		virtual int GetInt() const override;
		virtual float GetFloat() const override;
		virtual bool GetBool() const override;
		virtual ::Color GetColor() const override;
		virtual ::Vector3 GetVector() const override;
		virtual ::Vector4 GetVector4() const override;
	private:
		bool m_value;
	};
};
#pragma warning(pop)

#define REGISTER_DATA_TYPE(className,typeName) \
	auto *_reg_datatype_##typeName = new ds::__reg_datatype(#typeName,[](ds::Settings &dataSettings,const std::string &value) -> ds::Value* {return new className(dataSettings,value);}); \
	std::string className::GetTypeString() const {return #typeName;}

template<class TType>
	bool ds::Block::IsType(const std::string &key) const
{
	auto v = GetDataValue(key);
	if(v == nullptr)
		return false;
	if(typeid(*v) != typeid(TType))
		return false;
	return true;
}

template<class TType>
	std::shared_ptr<TType> ds::Block::GetRawType(const std::string &key) const
{
	return std::dynamic_pointer_cast<TType>(GetDataValue(key));
}

#endif
