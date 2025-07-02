// SPDX-FileCopyrightText: © 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "datasystem.h"
#include "datasystem_color.h"
#include "datasystem_vector.h"
#include "datasystem_t.hpp"
#include <fsys/filesystem.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <sharedutils/util_ifile.hpp>
#include <algorithm>
#include <sstream>
#include <mathutil/color.h>
#include <exprtk.hpp>
#include <fsys/ifile.hpp>

static ds::ValueTypeMap *g_DataValueFactoryMap = nullptr;

std::shared_ptr<ds::Settings> ds::create_data_settings(const std::unordered_map<std::string, std::string> &enums) { return std::make_shared<ds::Settings>(enums); }
void ds::register_data_value_type(const std::string &type, const std::function<Value *(Settings &, const std::string &)> &factory)
{
	auto ltype = type;
	ustring::to_lower(ltype);
	if(g_DataValueFactoryMap == nullptr) {
		static ds::ValueTypeMap map;
		g_DataValueFactoryMap = &map;
	}
	g_DataValueFactoryMap->AddFactory(ltype, factory);
}
ds::ValueTypeMap *ds::get_data_value_type_map() { return g_DataValueFactoryMap; }

void ds::ValueTypeMap::AddFactory(const std::string &name, const std::function<Value *(Settings &, const std::string &)> &factory)
{
	auto lname = name;
	ustring::to_lower(lname);
	m_factories.insert(decltype(m_factories)::value_type(lname, factory));
}

std::function<ds::Value *(ds::Settings &, const std::string &)> ds::ValueTypeMap::FindFactory(const std::string &name)
{
	auto it = m_factories.find(name);
	if(it == m_factories.end())
		return nullptr;
	return it->second;
}

////////////////////////

class ds::Settings : public std::enable_shared_from_this<ds::Settings> {
  public:
	Settings(const std::unordered_map<std::string, std::string> &enums)
	{
		exprtk::symbol_table<float> fSymbolTable {};
		for(auto &pair : enums)
			fSymbolTable.add_constant(pair.first, util::to_float(pair.second));
		m_expression.register_symbol_table(fSymbolTable);
	}
	bool ParseExpression(const std::string &expression, float &outResult)
	{
		auto r = m_parser.compile(expression, m_expression);
		if(r)
			outResult = m_expression.value();
		return r;
	}
	bool ParseExpression(const std::string &expression, int32_t &outResult)
	{
		auto r = m_parser.compile(expression, m_expression);
		if(r)
			outResult = umath::round(m_expression.value());
		return r;
	}
  private:
	exprtk::expression<float> m_expression = {};
	exprtk::parser<float> m_parser = {};
};

ds::Base::Base(ds::Settings &dataSettings) : m_dataSettings(dataSettings.shared_from_this()) {}
const ds::Settings &ds::Base::GetDataSettings() const { return const_cast<Base *>(this)->GetDataSettings(); }
ds::Settings &ds::Base::GetDataSettings() { return *m_dataSettings; }
ds::Base *ds::Base::Copy() { return new ds::Base(*this); }
bool ds::Base::IsBlock() const { return false; }
bool ds::Base::IsContainer() const { return false; }
ds::Base::~Base() {}
std::shared_ptr<ds::Block> ds::Base::GetBlock(const std::string_view &, unsigned int) { return nullptr; }
std::shared_ptr<ds::Block> ds::Base::GetBlock(const std::string_view &name) { return GetBlock(name, 0); }

////////////////////////

ds::Iterator::Iterator(ds::Base &data) : m_target(data), m_index(0) {}

bool ds::Iterator::IsValid()
{
	if(m_target.IsBlock()) {
		if(m_index > 0)
			return false;
		return true;
	}
	if(!m_target.IsContainer())
		return false;
	auto &container = static_cast<ds::Container &>(m_target);
	if(m_index >= container.GetBlockCount())
		return false;
	return true;
}

void ds::Iterator::operator++(int) { m_index++; }

ds::Block *ds::Iterator::get()
{
	if(!IsValid())
		return nullptr;
	if(m_target.IsBlock())
		return &static_cast<ds::Block &>(m_target);
	return static_cast<ds::Container &>(m_target).GetBlock(m_index).get();
}

ds::Block *ds::Iterator::operator->() { return get(); }

////////////////////////

ds::Block::Block(Settings &dataSettings) : ds::Base(dataSettings) {}
ds::Block::~Block() { m_data.clear(); }
std::shared_ptr<ds::Block> ds::Block::GetBlock(const std::string_view &name, unsigned int id)
{
	auto &data = GetValue(name);
	if(data == nullptr || (!data->IsBlock() && !data->IsContainer()))
		return nullptr;
	if(data->IsBlock())
		return std::static_pointer_cast<ds::Block>(data);
	return static_cast<ds::Container *>(data.get())->GetBlock(id);
}
bool ds::Block::IsEmpty() const { return m_data.empty(); }
void ds::Block::RemoveValue(const std::string &key)
{
	auto it = m_data.find(key);
	if(it == m_data.end())
		return;
	m_data.erase(it);
}
void ds::Block::DetachData(ds::Base &val)
{
	auto it = std::find_if(m_data.begin(), m_data.end(), [&val](const std::pair<std::string, std::shared_ptr<Base>> &pair) { return pair.second.get() == &val; });
	if(it == m_data.end())
		return;
	m_data.erase(it);
}
ds::Block *ds::Block::Copy()
{
	auto *cpy = new ds::Block(*m_dataSettings);
	for(auto it = m_data.begin(); it != m_data.end(); it++)
		cpy->AddData(it->first, std::shared_ptr<ds::Base>(it->second->Copy()));
	return cpy;
}
std::string ds::Block::ToString(const std::optional<std::string> &rootIdentifier, uint8_t tabDepth) const
{
	std::stringstream ss;
	if(rootIdentifier.has_value()) {
		ss << "\"" << *rootIdentifier << "\"\n{\n";
		++tabDepth;
	}

	std::string t(tabDepth, '\t');
	std::function<void(const ds::Block &, const std::string &)> fIterateDataBlock = nullptr;
	fIterateDataBlock = [&ss, &fIterateDataBlock](const ds::Block &block, const std::string &t) {
		auto *data = block.GetData();
		if(data == nullptr)
			return;
		for(auto &pair : *data) {
			if(pair.second->IsBlock()) {
				auto &block = static_cast<ds::Block &>(*pair.second);
				ss << t << "\"" << pair.first << "\"\n" << t << "{\n";
				fIterateDataBlock(block, t + '\t');
				ss << t << "}\n";
				continue;
			}
			if(pair.second->IsContainer()) {
				auto &container = static_cast<ds::Container &>(*pair.second);
				ss << t << "\"" << pair.first << "\"\n" << t << "{\n";
				for(auto &block : container.GetBlocks()) {
					if(block->IsContainer() || block->IsBlock())
						throw std::invalid_argument {"Data set block may only contain values!"};
					auto *dsValue = dynamic_cast<ds::Value *>(pair.second.get());
					if(dsValue == nullptr)
						throw std::invalid_argument {"Unexpected data set type!"};
					ss << t << "\t\"" << dsValue->GetString() << "\"\n";
				}
				ss << t << "}\n";
				continue;
			}
			auto *dsValue = dynamic_cast<ds::Value *>(pair.second.get());
			if(dsValue == nullptr)
				throw std::invalid_argument {"Unexpected data set type!"};
			ss << t << "$" << dsValue->GetTypeString() << " \"" << pair.first << "\" \"" << dsValue->GetString() << "\"\n";
		}
	};
	fIterateDataBlock(*this, t);
	if(rootIdentifier.has_value())
		ss << "}\n";
	return ss.str();
}
bool ds::Block::IsBlock() const { return true; }
const ds::Block::DataMap *ds::Block::GetData() const { return &m_data; }
void ds::Block::AddData(const std::string &name, const std::shared_ptr<Base> &data)
{
	auto lname = name;
	//ustring::to_lower(lname);
	auto it = m_data.find(lname);
	if(it == m_data.end()) {
		data->m_dataSettings = m_dataSettings;
		m_data[lname] = data;
		return;
	}
	if(!data->IsBlock()) {
		it->second = data;
		return;
	}
	if(it->second->IsContainer()) {
		static_cast<ds::Container &>(*it->second).AddData(std::static_pointer_cast<ds::Block>(data));
		return;
	}
	auto container = std::shared_ptr<ds::Container>(new ds::Container(*m_dataSettings));
	container->AddData(std::static_pointer_cast<ds::Block>(it->second));
	container->AddData(std::static_pointer_cast<ds::Block>(data));
	it->second = container;
}
const std::shared_ptr<ds::Base> &ds::Block::GetValue(const std::string_view &key) const
{
	static std::shared_ptr<ds::Base> nptr = nullptr;
	auto it = m_data.find(key);
	if(it == m_data.end())
		return nptr;
	return it->second;
}
std::shared_ptr<ds::Value> ds::Block::GetDataValue(const std::string_view &key) const
{
	auto it = m_data.find(key);
	if(it == m_data.end() || it->second->IsBlock() || it->second->IsContainer())
		return nullptr;
	return std::static_pointer_cast<ds::Value>(it->second);
}
std::shared_ptr<ds::Block> ds::Block::AddBlock(const std::string &name)
{
	auto val = GetValue(name);
	if(val && val->IsBlock())
		return std::static_pointer_cast<ds::Block>(val);
	auto block = std::make_shared<ds::Block>(GetDataSettings());
	AddData(name, block);
	return block;
}
void ds::Block::AddValue(const std::string &name, const std::string &value) { AddValue<std::string, ds::String>(name, value, "string"); }
void ds::Block::AddValue(const std::string &name, const ::Color &value) { AddValue<::Color, ds::Color>(name, value, "color"); }
void ds::Block::AddValue(const std::string &name, const ::Vector2 &value) { AddValue<::Vector2, ds::Vector2>(name, value, "vector2"); }
void ds::Block::AddValue(const std::string &name, const ::Vector3 &value) { AddValue<::Vector3, ds::Vector>(name, value, "vector"); }
void ds::Block::AddValue(const std::string &name, const ::Vector4 &value) { AddValue<::Vector4, ds::Vector4>(name, value, "vector4"); }
std::shared_ptr<ds::Base> ds::Block::AddValue(const std::string &type, const std::string &name, const std::string &value)
{
	static std::shared_ptr<ds::Base> nptr = nullptr;
	auto ltype = type;
	ustring::to_lower(ltype);
	auto factory = g_DataValueFactoryMap->FindFactory(ltype);
	if(factory == nullptr)
		return nptr;
	auto data = std::static_pointer_cast<ds::Base>(std::shared_ptr<ds::Value>(factory(*m_dataSettings, value)));
	AddData(name, data);
	return data;
}
bool ds::Block::GetString(const std::string_view &key, std::string *data) const
{
	auto &val = GetValue(key);
	if(val == nullptr || val->IsBlock())
		return false;
	auto *vdata = static_cast<ds::Value *>(val.get());
	*data = vdata->GetString();
	return true;
}
bool ds::Block::GetInt(const std::string_view &key, int *data) const
{
	auto &val = GetValue(key);
	if(val == nullptr || val->IsBlock())
		return 0;
	auto *vdata = static_cast<ds::Value *>(val.get());
	*data = vdata->GetInt();
	return true;
}
bool ds::Block::GetFloat(const std::string_view &key, float *data) const
{
	auto &val = GetValue(key);
	if(val == nullptr || val->IsBlock())
		return 0.f;
	auto *vdata = static_cast<ds::Value *>(val.get());
	*data = vdata->GetFloat();
	return true;
}
bool ds::Block::GetBool(const std::string_view &key, bool *data) const
{
	auto &val = GetValue(key);
	if(val == nullptr || val->IsBlock())
		return false;
	auto *vdata = static_cast<ds::Value *>(val.get());
	*data = vdata->GetBool();
	return true;
}
bool ds::Block::GetColor(const std::string_view &key, ::Color *data) const
{
	auto &val = GetValue(key);
	if(val == nullptr || val->IsBlock())
		return false;
	auto *vdata = static_cast<ds::Value *>(val.get());
	*data = vdata->GetColor();
	return true;
}
bool ds::Block::GetVector3(const std::string_view &key, ::Vector3 *data) const
{
	auto &val = GetValue(key);
	if(val == nullptr || val->IsBlock())
		return false;
	auto *vdata = static_cast<ds::Value *>(val.get());
	*data = vdata->GetVector();
	return true;
}
bool ds::Block::GetVector2(const std::string_view &key, ::Vector2 *data) const
{
	auto &val = GetValue(key);
	if(val == nullptr || val->IsBlock())
		return false;
	auto *vdata = static_cast<ds::Value *>(val.get());
	*data = vdata->GetVector2();
	return true;
}
bool ds::Block::GetVector4(const std::string_view &key, ::Vector4 *data) const
{
	auto &val = GetValue(key);
	if(val == nullptr || val->IsBlock())
		return false;
	auto *vdata = static_cast<ds::Value *>(val.get());
	*data = vdata->GetVector4();
	return true;
}
bool ds::Block::HasValue(const std::string_view &key) const { return GetValue(key) != nullptr; }
std::string ds::Block::GetString(const std::string_view &key, const std::string &def) const
{
	std::string value;
	if(!GetString(key, &value))
		value = def;
	return value;
}
int ds::Block::GetInt(const std::string_view &key, int def) const
{
	int value;
	if(!GetInt(key, &value))
		value = def;
	return value;
}
float ds::Block::GetFloat(const std::string_view &key, float def) const
{
	float value;
	if(!GetFloat(key, &value))
		value = def;
	return value;
}
bool ds::Block::GetBool(const std::string_view &key, bool def) const
{
	bool value;
	if(!GetBool(key, &value))
		value = def;
	return value;
}
::Color ds::Block::GetColor(const std::string_view &key, const ::Color &def) const
{
	::Color value;
	if(!GetColor(key, &value))
		value = def;
	return value;
}
::Vector3 ds::Block::GetVector3(const std::string_view &key, const ::Vector3 &def) const
{
	::Vector3 value;
	if(!GetVector3(key, &value))
		value = def;
	return value;
}
::Vector2 ds::Block::GetVector2(const std::string_view &key, const ::Vector2 &def) const
{
	::Vector2 value;
	if(!GetVector2(key, &value))
		value = def;
	return value;
}
::Vector4 ds::Block::GetVector4(const std::string_view &key, const ::Vector4 &def) const
{
	::Vector4 value;
	if(!GetVector4(key, &value))
		value = def;
	return value;
}

bool ds::Block::IsString(const std::string_view &key) const { return IsType<ds::String>(key); }
bool ds::Block::IsInt(const std::string_view &key) const { return IsType<ds::Int>(key); }
bool ds::Block::IsFloat(const std::string_view &key) const { return IsType<ds::Float>(key); }
bool ds::Block::IsBool(const std::string_view &key) const { return IsType<ds::Bool>(key); }
bool ds::Block::IsColor(const std::string_view &key) const { return IsType<ds::Color>(key); }
bool ds::Block::IsVector2(const std::string_view &key) const { return IsType<ds::Vector2>(key); }
bool ds::Block::IsVector3(const std::string_view &key) const { return IsType<ds::Vector>(key); }
bool ds::Block::IsVector4(const std::string_view &key) const { return IsType<ds::Vector4>(key); }
bool ds::Block::GetRawString(const std::string_view &key, std::string *v) const
{
	auto data = GetRawType<ds::String>(key);
	if(data == nullptr)
		return false;
	*v = data->GetString();
	return true;
}
bool ds::Block::GetRawInt(const std::string_view &key, int *v) const
{
	auto data = GetRawType<ds::Int>(key);
	if(data == nullptr)
		return false;
	*v = data->GetInt();
	return true;
}
bool ds::Block::GetRawFloat(const std::string_view &key, float *v) const
{
	auto data = GetRawType<ds::Float>(key);
	if(data == nullptr)
		return false;
	*v = data->GetFloat();
	return true;
}
bool ds::Block::GetRawBool(const std::string_view &key, bool *v) const
{
	auto data = GetRawType<ds::Bool>(key);
	if(data == nullptr)
		return false;
	*v = data->GetBool();
	return true;
}
bool ds::Block::GetRawColor(const std::string_view &key, ::Color *v) const
{
	auto data = GetRawType<ds::Color>(key);
	if(data == nullptr)
		return false;
	*v = data->GetColor();
	return true;
}
bool ds::Block::GetRawVector3(const std::string_view &key, ::Vector3 *v) const
{
	auto data = GetRawType<ds::Vector>(key);
	if(data == nullptr)
		return false;
	*v = data->GetVector();
	return true;
}
bool ds::Block::GetRawVector2(const std::string_view &key, ::Vector2 *v) const
{
	auto data = GetRawType<ds::Vector2>(key);
	if(data == nullptr)
		return false;
	*v = data->GetVector2();
	return true;
}
bool ds::Block::GetRawVector4(const std::string_view &key, ::Vector4 *v) const
{
	auto data = GetRawType<ds::Vector4>(key);
	if(data == nullptr)
		return false;
	*v = data->GetVector4();
	return true;
}

////////////////////////

ds::Container::Container(Settings &dataSettings) : ds::Base(dataSettings) {}
ds::Container::~Container() { m_dataBlocks.clear(); }
bool ds::Container::IsContainer() const { return true; }
void ds::Container::AddData(const std::shared_ptr<Block> &data)
{
	m_dataBlocks.push_back(data);
	data->m_dataSettings = m_dataSettings;
}
std::shared_ptr<ds::Block> ds::Container::GetBlock(unsigned int id)
{
	if(id >= m_dataBlocks.size())
		return nullptr;
	return m_dataBlocks[id];
}
std::vector<std::shared_ptr<ds::Block>> &ds::Container::GetBlocks() { return m_dataBlocks; }
uint32_t ds::Container::GetBlockCount() const { return static_cast<unsigned int>(m_dataBlocks.size()); }

////////////////////////

ds::Value::Value(Settings &dataSettings) : ds::Base(dataSettings) {}

////////////////////////

#define IsEOF(c) (c == EOF || f.Eof())
static unsigned long long FindFirstOf(ufile::IFile &f, const char *s)
{
	if(f.Eof())
		return static_cast<unsigned long long>(EOF);
	unsigned char c;
	int cur = 0;
	do {
		c = static_cast<unsigned char>(f.ReadChar());
		if(!IsEOF(c)) {
			while(s[cur] != '\0') {
				if(c == s[cur])
					return c;
				cur++;
			}
		}
	} while(!IsEOF(c));
	return static_cast<unsigned long long>(EOF);
}
static unsigned long long FindFirstNotOf(ufile::IFile &f, const char *s)
{
	if(f.Eof())
		return static_cast<unsigned long long>(EOF);
	unsigned char c;
	int cur = 0;
	do {
		c = static_cast<unsigned char>(f.ReadChar());
		if(!IsEOF(c)) {
			bool ret = true;
			while(s[cur] != '\0') {
				if(c == s[cur]) {
					ret = false;
					break;
				}
				cur++;
			}
			if(ret)
				return c;
			cur = 0;
		}
	} while(!IsEOF(c));
	return static_cast<unsigned long long>(EOF);
}
static std::string ReadUntil(ufile::IFile &f, const char *s)
{
	std::string ret = "";
	if(f.Eof())
		return ret;
	unsigned char c;
	int cur = 0;
	do {
		c = static_cast<unsigned char>(f.ReadChar());
		if(!IsEOF(c)) {
			while(s[cur] != '\0') {
				if(c == s[cur]) {
					f.Seek(f.Tell() - 1);
					return ret;
				}
				cur++;
			}
			cur = 0;
			ret += char(c);
		}
	} while(!IsEOF(c));
	return ret;
}

static unsigned long long FindFirstOf(ufile::IFile &f, char c)
{
	char s[2] = {c, '\0'};
	return FindFirstOf(f, s);
}
static unsigned long long FindFirstNotOf(ufile::IFile &f, char c)
{
	char s[2] = {c, '\0'};
	return FindFirstNotOf(f, s);
}
static std::string ReadUntil(ufile::IFile &f, char c)
{
	char s[2] = {c, '\0'};
	return ReadUntil(f, s);
}
static unsigned long long FindFirstOf(ufile::IFile &f, std::string s) { return FindFirstOf(f, s.c_str()); }
static unsigned long long FindFirstNotOf(ufile::IFile &f, std::string s) { return FindFirstNotOf(f, s.c_str()); }
static std::string ReadUntil(ufile::IFile &f, std::string s) { return ReadUntil(f, s.c_str()); }

static std::string ReadValue(unsigned long long c, ufile::IFile &f)
{
	std::string val;
	if(c == EOF)
		return val;
	if(c == '\"') {
		auto val = ReadUntil(f, '\"');
		f.Seek(f.Tell() + 1);
		return val;
	}
	f.Seek(f.Tell() - 1);
	val = ReadUntil(f, ustring::WHITESPACE + "},");
	return val;
}

static bool read_block_data(ds::Block &block, const std::unordered_map<std::string, std::string> &enums, const std::shared_ptr<ds::Settings> &dataSettings, ufile::IFile &f, int &listID, std::string blockType = "", bool bMainBlock = false)
{
	if(f.Eof())
		return false;
	auto c = FindFirstNotOf(f, ustring::WHITESPACE);
	if(c == ustring::NOT_FOUND)
		return false;
	if(c == '}') {
		f.Seek(f.Tell() - 1);
		return false;
	}
	auto ident = ReadValue(c, f);
	switch(ident[0]) {
	case '$':
		{
			if(!blockType.empty())
				return false;
			f.Seek(f.Tell() + 1);
			c = FindFirstNotOf(f, ustring::WHITESPACE);
			if(c == -1)
				return false;
			auto name = ReadValue(c, f);
			ustring::remove_quotes(name);
			//ustring::to_lower(name);
			//f.Seek(f.Tell() +1);
			c = FindFirstNotOf(f, ustring::WHITESPACE);
			if(c == -1)
				return false;
			ident = ident.substr(1);
			ustring::to_lower(ident);
			if(c != '{') {
				auto value = ReadValue(c, f);
				ustring::remove_quotes(value);
				auto it = enums.find(value);
				if(it != enums.end())
					value = it->second;
				block.AddValue(ident, name, value);
				//f.Seek(f.Tell() +1);
				break;
			}
			else {
				blockType = ident;
				ident = name;
				f.Seek(f.Tell() - 1);
			}
		}
	default:
		{
			auto c = FindFirstNotOf(f, ustring::WHITESPACE);
			if(c == ustring::NOT_FOUND)
				return false;
			switch(c) {
			case '{':
				{
					auto sub = std::make_shared<ds::Block>(*dataSettings);
					bool r;
					do
						r = read_block_data(*sub, enums, dataSettings, f, listID, blockType);
					while(r == true);
					auto subBase = std::static_pointer_cast<ds::Base>(sub);
					block.AddData(ident, subBase);
					listID = 0;
					f.Seek(f.Tell() + 1);
					break;
				}
			case ',':
				{
					c = FindFirstNotOf(f, ustring::WHITESPACE);
					if(c == -1)
						return false;
				}
			default:
				{
					if(blockType.empty())
						blockType = "string";
					auto it = enums.find(ident);
					if(it != enums.end())
						ident = it->second;
					block.AddValue(blockType, std::to_string(listID), ident);
					f.Seek(f.Tell() - 1);
					listID++;
					break;
				}
			}
			break;
		}
	}
	if(bMainBlock == true)
		read_block_data(block, enums, dataSettings, f, listID, blockType, bMainBlock);
	return true;
}

/*
#include <iostream>
void PrintBlocks(std::string name,DataBase *data,std::string t="\t")
{
	if(!data->IsBlock())
	{
		DataValue *val = static_cast<DataValue*>(data);
		std::cout<<t<<name<<" = "<<val->GetString()<<std::endl;
		return;
	}
	DataBlock *block = static_cast<DataBlock*>(data);
	std::cout<<t<<name<<":"<<std::endl;
	t += '\t';
	std::unordered_map<std::string,DataBase*> *sub = block->GetData();
	std::unordered_map<std::string,DataBase*>::iterator i;
	for(i=sub->begin();i!=sub->end();i++)
		PrintBlocks(i->first,i->second,t);
}*/

std::shared_ptr<ds::Block> ds::System::ReadData(ufile::IFile &f, const std::unordered_map<std::string, std::string> &enums)
{
	auto dataSettings = ds::create_data_settings(enums);

	auto data = std::make_shared<ds::Block>(*dataSettings);
	auto listID = 0;
	// f.IgnoreComments("//");
	// f.IgnoreComments("/*","*/");

	if(read_block_data(*data, enums, dataSettings, f, listID, "", true) == false)
		return nullptr;
	return data;
}
std::shared_ptr<ds::Block> ds::System::LoadData(const char *path, const std::unordered_map<std::string, std::string> &enums)
{
	auto f = FileManager::OpenFile(path, "r");
	if(f == nullptr)
		return nullptr;
	fsys::File fp {f};
	return ReadData(fp, enums);
}

////////////////////////

ds::String::String(Settings &dataSettings, const std::string &value) : ds::Value(dataSettings), m_value(value) {}
ds::Value *ds::String::Copy() { return new ds::String(*m_dataSettings, GetValue()); }
ds::ValueType ds::String::GetType() const { return ValueType::String; }

const std::string &ds::String::GetValue() const { return m_value; }
void ds::String::SetValue(const std::string &value) { m_value = value; }

std::string ds::String::GetString() const { return m_value; }
int ds::String::GetInt() const { return util::to_int(m_value); }
float ds::String::GetFloat() const { return util::to_float(m_value); }
bool ds::String::GetBool() const { return util::to_boolean(m_value); }
::Color ds::String::GetColor() const { return ::Color {m_value}; }
::Vector3 ds::String::GetVector() const { return uvec::create(m_value); }
::Vector2 ds::String::GetVector2() const
{
	auto v = uvec::create(m_value);
	return ::Vector2 {v.x, v.y};
}
::Vector4 ds::String::GetVector4() const { return uvec::create_v4(m_value); }
REGISTER_DATA_TYPE(ds::String, string)

////////////////////////

ds::Int::Int(Settings &dataSettings, const std::string &value) : ds::Value(dataSettings)
{
	if(GetDataSettings().ParseExpression(value, m_value) == false)
		m_value = util::to_int(value);
}
ds::Int::Int(Settings &dataSettings, int32_t value) : ds::Value(dataSettings), m_value(value) {}
ds::Value *ds::Int::Copy() { return new ds::Int(*m_dataSettings, m_value); }
ds::ValueType ds::Int::GetType() const { return ValueType::Int; }
int32_t ds::Int::GetValue() const { return m_value; }
void ds::Int::SetValue(int32_t value) { m_value = value; }

std::string ds::Int::GetString() const { return std::to_string(m_value); }
int ds::Int::GetInt() const { return m_value; }
float ds::Int::GetFloat() const { return static_cast<float>(m_value); }
bool ds::Int::GetBool() const { return (m_value != 0) ? true : false; }
::Color ds::Int::GetColor() const { return ::Color {static_cast<int16_t>(m_value), static_cast<int16_t>(m_value), static_cast<int16_t>(m_value), 255}; }
::Vector3 ds::Int::GetVector() const { return ::Vector3 {m_value, m_value, m_value}; }
::Vector2 ds::Int::GetVector2() const { return ::Vector2 {m_value, m_value}; }
::Vector4 ds::Int::GetVector4() const { return ::Vector4 {m_value, m_value, m_value, m_value}; }
REGISTER_DATA_TYPE(ds::Int, int)

////////////////////////

ds::Float::Float(Settings &dataSettings, const std::string &value) : ds::Value(dataSettings)
{
	if(GetDataSettings().ParseExpression(value, m_value) == false)
		m_value = util::to_int(value);
}
ds::Float::Float(Settings &dataSettings, float value) : ds::Value(dataSettings), m_value(value) {}
ds::Value *ds::Float::Copy() { return new ds::Float(*m_dataSettings, m_value); }
ds::ValueType ds::Float::GetType() const { return ValueType::Float; }
float ds::Float::GetValue() const { return m_value; }
void ds::Float::SetValue(float value) { m_value = value; }

std::string ds::Float::GetString() const { return std::to_string(m_value); }
int ds::Float::GetInt() const { return m_value; }
float ds::Float::GetFloat() const { return static_cast<float>(m_value); }
bool ds::Float::GetBool() const { return (m_value != 0) ? true : false; }
::Color ds::Float::GetColor() const
{
	auto v = static_cast<int16_t>(m_value * 255.f);
	return ::Color {v, v, v, 255};
}
::Vector3 ds::Float::GetVector() const { return ::Vector3 {m_value, m_value, m_value}; }
::Vector2 ds::Float::GetVector2() const { return ::Vector2 {m_value, m_value}; }
::Vector4 ds::Float::GetVector4() const { return ::Vector4 {m_value, m_value, m_value, m_value}; }
REGISTER_DATA_TYPE(ds::Float, float)

////////////////////////

ds::Bool::Bool(Settings &dataSettings, const std::string &value) : ds::Value(dataSettings), m_value(util::to_boolean(value)) {}
ds::Bool::Bool(Settings &dataSettings, bool value) : ds::Value(dataSettings), m_value(value) {}
ds::Value *ds::Bool::Copy() { return new ds::Bool(*m_dataSettings, m_value); }
ds::ValueType ds::Bool::GetType() const { return ValueType::Bool; }
bool ds::Bool::GetValue() const { return m_value; }
void ds::Bool::SetValue(bool value) { m_value = value; }

std::string ds::Bool::GetString() const { return std::to_string(m_value); }
int ds::Bool::GetInt() const { return m_value; }
float ds::Bool::GetFloat() const { return static_cast<float>(m_value); }
bool ds::Bool::GetBool() const { return (m_value != 0) ? true : false; }
::Color ds::Bool::GetColor() const { return ::Color {m_value ? 255 : 0, m_value ? 255 : 0, m_value ? 255 : 0, 255}; }
::Vector3 ds::Bool::GetVector() const { return ::Vector3 {m_value, m_value, m_value}; }
::Vector2 ds::Bool::GetVector2() const { return ::Vector2 {m_value, m_value}; }
::Vector4 ds::Bool::GetVector4() const { return ::Vector4 {m_value, m_value, m_value, m_value}; }
REGISTER_DATA_TYPE(ds::Bool, bool)
