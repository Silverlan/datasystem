// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"
#include <exprtk.hpp>

module pragma.datasystem;

import :core;
import pragma.filesystem;

static pragma::datasystem::ValueTypeMap *g_DataValueFactoryMap = nullptr;

static pragma::datasystem::ValueTypeMap map;
void pragma::datasystem::close() { map = {}; }

std::shared_ptr<pragma::datasystem::Settings> pragma::datasystem::create_data_settings(const std::unordered_map<std::string, std::string> &enums) { return std::make_shared<Settings>(enums); }
void pragma::datasystem::register_data_value_type(const std::string &type, const std::function<Value *(Settings &, const std::string &)> &factory)
{
	auto ltype = type;
	pragma::string::to_lower(ltype);
	if(g_DataValueFactoryMap == nullptr) {
		g_DataValueFactoryMap = &map;
	}
	g_DataValueFactoryMap->AddFactory(ltype, factory);
}
pragma::datasystem::ValueTypeMap *pragma::datasystem::get_data_value_type_map() { return g_DataValueFactoryMap; }

void pragma::datasystem::ValueTypeMap::AddFactory(const std::string &name, const std::function<Value *(Settings &, const std::string &)> &factory)
{
	auto lname = name;
	pragma::string::to_lower(lname);
	m_factories.insert(decltype(m_factories)::value_type(lname, factory));
}

std::function<pragma::datasystem::Value *(pragma::datasystem::Settings &, const std::string &)> pragma::datasystem::ValueTypeMap::FindFactory(const std::string &name)
{
	auto it = m_factories.find(name);
	if(it == m_factories.end())
		return nullptr;
	return it->second;
}

void pragma::datasystem::register_base_types()
{
	register_data_value_type<Bool>("bool");
	register_data_value_type<Float>("float");
	register_data_value_type<Int>("int");
	register_data_value_type<String>("string");

	register_data_value_type<Vector>("vector");
	register_data_value_type<Vector2>("vector2");
	register_data_value_type<Vector4>("vector4");

	register_data_value_type<Color>("color");
}

////////////////////////

class pragma::datasystem::Settings : public std::enable_shared_from_this<Settings> {
  public:
	Settings(const std::unordered_map<std::string, std::string> &enums)
	{
		exprtk::symbol_table<float> fSymbolTable {};
		for(auto &pair : enums)
			fSymbolTable.add_constant(pair.first, pragma::util::to_float(pair.second));
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
			outResult = pragma::math::round(m_expression.value());
		return r;
	}
  private:
	exprtk::expression<float> m_expression = {};
	exprtk::parser<float> m_parser = {};
};

pragma::datasystem::Base::Base(Settings &dataSettings) : m_dataSettings(dataSettings.shared_from_this()) {}
const pragma::datasystem::Settings &pragma::datasystem::Base::GetDataSettings() const { return const_cast<Base *>(this)->GetDataSettings(); }
pragma::datasystem::Settings &pragma::datasystem::Base::GetDataSettings() { return *m_dataSettings; }
pragma::datasystem::Base *pragma::datasystem::Base::Copy() { return new Base(*this); }
bool pragma::datasystem::Base::IsBlock() const { return false; }
bool pragma::datasystem::Base::IsContainer() const { return false; }
pragma::datasystem::Base::~Base() {}
std::shared_ptr<pragma::datasystem::Block> pragma::datasystem::Base::GetBlock(const std::string_view &, unsigned int) { return nullptr; }
std::shared_ptr<pragma::datasystem::Block> pragma::datasystem::Base::GetBlock(const std::string_view &name) { return GetBlock(name, 0); }

////////////////////////

pragma::datasystem::Iterator::Iterator(Base &data) : m_target(data), m_index(0) {}

bool pragma::datasystem::Iterator::IsValid()
{
	if(m_target.IsBlock()) {
		if(m_index > 0)
			return false;
		return true;
	}
	if(!m_target.IsContainer())
		return false;
	auto &container = static_cast<Container &>(m_target);
	if(m_index >= container.GetBlockCount())
		return false;
	return true;
}

void pragma::datasystem::Iterator::operator++(int) { m_index++; }

pragma::datasystem::Block *pragma::datasystem::Iterator::get()
{
	if(!IsValid())
		return nullptr;
	if(m_target.IsBlock())
		return &static_cast<Block &>(m_target);
	return static_cast<Container &>(m_target).GetBlock(m_index).get();
}

pragma::datasystem::Block *pragma::datasystem::Iterator::operator->() { return get(); }

////////////////////////

pragma::datasystem::Block::Block(Settings &dataSettings) : Base(dataSettings) {}
pragma::datasystem::Block::~Block() { m_data.clear(); }
std::shared_ptr<pragma::datasystem::Block> pragma::datasystem::Block::GetBlock(const std::string_view &name, unsigned int id)
{
	auto &data = GetValue(name);
	if(data == nullptr || (!data->IsBlock() && !data->IsContainer()))
		return nullptr;
	if(data->IsBlock())
		return std::static_pointer_cast<Block>(data);
	return static_cast<Container *>(data.get())->GetBlock(id);
}
bool pragma::datasystem::Block::IsEmpty() const { return m_data.empty(); }
void pragma::datasystem::Block::RemoveValue(const std::string &key)
{
	auto it = m_data.find(key);
	if(it == m_data.end())
		return;
	m_data.erase(it);
}
void pragma::datasystem::Block::DetachData(Base &val)
{
	auto it = std::find_if(m_data.begin(), m_data.end(), [&val](const std::pair<std::string, std::shared_ptr<Base>> &pair) { return pair.second.get() == &val; });
	if(it == m_data.end())
		return;
	m_data.erase(it);
}
pragma::datasystem::Block *pragma::datasystem::Block::Copy()
{
	auto *cpy = new Block(*m_dataSettings);
	for(auto it = m_data.begin(); it != m_data.end(); it++)
		cpy->AddData(it->first, std::shared_ptr<Base>(it->second->Copy()));
	return cpy;
}
std::string pragma::datasystem::Block::ToString(const std::optional<std::string> &rootIdentifier, uint8_t tabDepth) const
{
	std::stringstream ss;
	if(rootIdentifier.has_value()) {
		ss << "\"" << *rootIdentifier << "\"\n{\n";
		++tabDepth;
	}

	std::string t(tabDepth, '\t');
	std::function<void(const Block &, const std::string &)> fIterateDataBlock = nullptr;
	fIterateDataBlock = [&ss, &fIterateDataBlock](const Block &block, const std::string &t) {
		auto *data = block.GetData();
		if(data == nullptr)
			return;
		for(auto &pair : *data) {
			if(pair.second->IsBlock()) {
				auto &block = static_cast<Block &>(*pair.second);
				ss << t << "\"" << pair.first << "\"\n" << t << "{\n";
				fIterateDataBlock(block, t + '\t');
				ss << t << "}\n";
				continue;
			}
			if(pair.second->IsContainer()) {
				auto &container = static_cast<Container &>(*pair.second);
				ss << t << "\"" << pair.first << "\"\n" << t << "{\n";
				for(auto &block : container.GetBlocks()) {
					if(block->IsContainer() || block->IsBlock())
						throw std::invalid_argument {"Data set block may only contain values!"};
					auto *dsValue = dynamic_cast<Value *>(pair.second.get());
					if(dsValue == nullptr)
						throw std::invalid_argument {"Unexpected data set type!"};
					ss << t << "\t\"" << dsValue->GetString() << "\"\n";
				}
				ss << t << "}\n";
				continue;
			}
			auto *dsValue = dynamic_cast<Value *>(pair.second.get());
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
bool pragma::datasystem::Block::IsBlock() const { return true; }
const pragma::datasystem::Block::DataMap *pragma::datasystem::Block::GetData() const { return &m_data; }
void pragma::datasystem::Block::AddData(const std::string &name, const std::shared_ptr<Base> &data)
{
	auto lname = name;
	//pragma::string::to_lower(lname);
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
		static_cast<Container &>(*it->second).AddData(std::static_pointer_cast<Block>(data));
		return;
	}
	auto container = std::shared_ptr<Container>(new Container(*m_dataSettings));
	container->AddData(std::static_pointer_cast<Block>(it->second));
	container->AddData(std::static_pointer_cast<Block>(data));
	it->second = container;
}
const std::shared_ptr<pragma::datasystem::Base> &pragma::datasystem::Block::GetValue(const std::string_view &key) const
{
	static std::shared_ptr<Base> nptr = nullptr;
	auto it = m_data.find(key);
	if(it == m_data.end())
		return nptr;
	return it->second;
}
std::shared_ptr<pragma::datasystem::Value> pragma::datasystem::Block::GetDataValue(const std::string_view &key) const
{
	auto it = m_data.find(key);
	if(it == m_data.end() || it->second->IsBlock() || it->second->IsContainer())
		return nullptr;
	return std::static_pointer_cast<Value>(it->second);
}
std::shared_ptr<pragma::datasystem::Block> pragma::datasystem::Block::AddBlock(const std::string &name)
{
	auto val = GetValue(name);
	if(val && val->IsBlock())
		return std::static_pointer_cast<Block>(val);
	auto block = std::make_shared<Block>(GetDataSettings());
	AddData(name, block);
	return block;
}
void pragma::datasystem::Block::AddValue(const std::string &name, const std::string &value) { AddValue<std::string, String>(name, value, "string"); }
void pragma::datasystem::Block::AddValue(const std::string &name, const ::Color &value) { AddValue<::Color, Color>(name, value, "color"); }
void pragma::datasystem::Block::AddValue(const std::string &name, const ::Vector2 &value) { AddValue<::Vector2, Vector2>(name, value, "vector2"); }
void pragma::datasystem::Block::AddValue(const std::string &name, const Vector3 &value) { AddValue<Vector3, Vector>(name, value, "vector"); }
void pragma::datasystem::Block::AddValue(const std::string &name, const ::Vector4 &value) { AddValue<::Vector4, Vector4>(name, value, "vector4"); }
std::shared_ptr<pragma::datasystem::Base> pragma::datasystem::Block::AddValue(const std::string &type, const std::string &name, const std::string &value)
{
	static std::shared_ptr<Base> nptr = nullptr;
	auto ltype = type;
	pragma::string::to_lower(ltype);
	auto factory = g_DataValueFactoryMap->FindFactory(ltype);
	if(factory == nullptr)
		return nptr;
	auto data = std::static_pointer_cast<Base>(std::shared_ptr<Value>(factory(*m_dataSettings, value)));
	AddData(name, data);
	return data;
}
bool pragma::datasystem::Block::GetString(const std::string_view &key, std::string *data) const
{
	auto &val = GetValue(key);
	if(val == nullptr || val->IsBlock())
		return false;
	auto *vdata = static_cast<Value *>(val.get());
	*data = vdata->GetString();
	return true;
}
bool pragma::datasystem::Block::GetInt(const std::string_view &key, int *data) const
{
	auto &val = GetValue(key);
	if(val == nullptr || val->IsBlock())
		return 0;
	auto *vdata = static_cast<Value *>(val.get());
	*data = vdata->GetInt();
	return true;
}
bool pragma::datasystem::Block::GetFloat(const std::string_view &key, float *data) const
{
	auto &val = GetValue(key);
	if(val == nullptr || val->IsBlock())
		return 0.f;
	auto *vdata = static_cast<Value *>(val.get());
	*data = vdata->GetFloat();
	return true;
}
bool pragma::datasystem::Block::GetBool(const std::string_view &key, bool *data) const
{
	auto &val = GetValue(key);
	if(val == nullptr || val->IsBlock())
		return false;
	auto *vdata = static_cast<Value *>(val.get());
	*data = vdata->GetBool();
	return true;
}
bool pragma::datasystem::Block::GetColor(const std::string_view &key, ::Color *data) const
{
	auto &val = GetValue(key);
	if(val == nullptr || val->IsBlock())
		return false;
	auto *vdata = static_cast<Value *>(val.get());
	*data = vdata->GetColor();
	return true;
}
bool pragma::datasystem::Block::GetVector3(const std::string_view &key, Vector3 *data) const
{
	auto &val = GetValue(key);
	if(val == nullptr || val->IsBlock())
		return false;
	auto *vdata = static_cast<Value *>(val.get());
	*data = vdata->GetVector();
	return true;
}
bool pragma::datasystem::Block::GetVector2(const std::string_view &key, ::Vector2 *data) const
{
	auto &val = GetValue(key);
	if(val == nullptr || val->IsBlock())
		return false;
	auto *vdata = static_cast<Value *>(val.get());
	*data = vdata->GetVector2();
	return true;
}
bool pragma::datasystem::Block::GetVector4(const std::string_view &key, ::Vector4 *data) const
{
	auto &val = GetValue(key);
	if(val == nullptr || val->IsBlock())
		return false;
	auto *vdata = static_cast<Value *>(val.get());
	*data = vdata->GetVector4();
	return true;
}
bool pragma::datasystem::Block::HasValue(const std::string_view &key) const { return GetValue(key) != nullptr; }
std::string pragma::datasystem::Block::GetString(const std::string_view &key, const std::string &def) const
{
	std::string value;
	if(!GetString(key, &value))
		value = def;
	return value;
}
int pragma::datasystem::Block::GetInt(const std::string_view &key, int def) const
{
	int value;
	if(!GetInt(key, &value))
		value = def;
	return value;
}
float pragma::datasystem::Block::GetFloat(const std::string_view &key, float def) const
{
	float value;
	if(!GetFloat(key, &value))
		value = def;
	return value;
}
bool pragma::datasystem::Block::GetBool(const std::string_view &key, bool def) const
{
	bool value;
	if(!GetBool(key, &value))
		value = def;
	return value;
}
Color pragma::datasystem::Block::GetColor(const std::string_view &key, const ::Color &def) const
{
	::Color value;
	if(!GetColor(key, &value))
		value = def;
	return value;
}
Vector3 pragma::datasystem::Block::GetVector3(const std::string_view &key, const Vector3 &def) const
{
	Vector3 value;
	if(!GetVector3(key, &value))
		value = def;
	return value;
}
Vector2 pragma::datasystem::Block::GetVector2(const std::string_view &key, const ::Vector2 &def) const
{
	::Vector2 value;
	if(!GetVector2(key, &value))
		value = def;
	return value;
}
Vector4 pragma::datasystem::Block::GetVector4(const std::string_view &key, const ::Vector4 &def) const
{
	::Vector4 value;
	if(!GetVector4(key, &value))
		value = def;
	return value;
}

bool pragma::datasystem::Block::IsString(const std::string_view &key) const { return IsType<String>(key); }
bool pragma::datasystem::Block::IsInt(const std::string_view &key) const { return IsType<Int>(key); }
bool pragma::datasystem::Block::IsFloat(const std::string_view &key) const { return IsType<Float>(key); }
bool pragma::datasystem::Block::IsBool(const std::string_view &key) const { return IsType<Bool>(key); }
bool pragma::datasystem::Block::IsColor(const std::string_view &key) const { return IsType<Color>(key); }
bool pragma::datasystem::Block::IsVector2(const std::string_view &key) const { return IsType<Vector2>(key); }
bool pragma::datasystem::Block::IsVector3(const std::string_view &key) const { return IsType<Vector>(key); }
bool pragma::datasystem::Block::IsVector4(const std::string_view &key) const { return IsType<Vector4>(key); }
bool pragma::datasystem::Block::GetRawString(const std::string_view &key, std::string *v) const
{
	auto data = GetRawType<String>(key);
	if(data == nullptr)
		return false;
	*v = data->GetString();
	return true;
}
bool pragma::datasystem::Block::GetRawInt(const std::string_view &key, int *v) const
{
	auto data = GetRawType<Int>(key);
	if(data == nullptr)
		return false;
	*v = data->GetInt();
	return true;
}
bool pragma::datasystem::Block::GetRawFloat(const std::string_view &key, float *v) const
{
	auto data = GetRawType<Float>(key);
	if(data == nullptr)
		return false;
	*v = data->GetFloat();
	return true;
}
bool pragma::datasystem::Block::GetRawBool(const std::string_view &key, bool *v) const
{
	auto data = GetRawType<Bool>(key);
	if(data == nullptr)
		return false;
	*v = data->GetBool();
	return true;
}
bool pragma::datasystem::Block::GetRawColor(const std::string_view &key, ::Color *v) const
{
	auto data = GetRawType<Color>(key);
	if(data == nullptr)
		return false;
	*v = data->GetColor();
	return true;
}
bool pragma::datasystem::Block::GetRawVector3(const std::string_view &key, Vector3 *v) const
{
	auto data = GetRawType<Vector>(key);
	if(data == nullptr)
		return false;
	*v = data->GetVector();
	return true;
}
bool pragma::datasystem::Block::GetRawVector2(const std::string_view &key, ::Vector2 *v) const
{
	auto data = GetRawType<Vector2>(key);
	if(data == nullptr)
		return false;
	*v = data->GetVector2();
	return true;
}
bool pragma::datasystem::Block::GetRawVector4(const std::string_view &key, ::Vector4 *v) const
{
	auto data = GetRawType<Vector4>(key);
	if(data == nullptr)
		return false;
	*v = data->GetVector4();
	return true;
}

////////////////////////

pragma::datasystem::Container::Container(Settings &dataSettings) : Base(dataSettings) {}
pragma::datasystem::Container::~Container() { m_dataBlocks.clear(); }
bool pragma::datasystem::Container::IsContainer() const { return true; }
void pragma::datasystem::Container::AddData(const std::shared_ptr<Block> &data)
{
	m_dataBlocks.push_back(data);
	data->m_dataSettings = m_dataSettings;
}
std::shared_ptr<pragma::datasystem::Block> pragma::datasystem::Container::GetBlock(unsigned int id)
{
	if(id >= m_dataBlocks.size())
		return nullptr;
	return m_dataBlocks[id];
}
std::vector<std::shared_ptr<pragma::datasystem::Block>> &pragma::datasystem::Container::GetBlocks() { return m_dataBlocks; }
uint32_t pragma::datasystem::Container::GetBlockCount() const { return static_cast<unsigned int>(m_dataBlocks.size()); }

////////////////////////

pragma::datasystem::Value::Value(Settings &dataSettings) : Base(dataSettings) {}

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
	val = ReadUntil(f, pragma::string::WHITESPACE + "},");
	return val;
}

static bool read_block_data(pragma::datasystem::Block &block, const std::unordered_map<std::string, std::string> &enums, const std::shared_ptr<pragma::datasystem::Settings> &dataSettings, ufile::IFile &f, int &listID, std::string blockType = "", bool bMainBlock = false)
{
	if(f.Eof())
		return false;
	auto c = FindFirstNotOf(f, pragma::string::WHITESPACE);
	if(c == pragma::string::NOT_FOUND)
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
			c = FindFirstNotOf(f, pragma::string::WHITESPACE);
			if(c == -1)
				return false;
			auto name = ReadValue(c, f);
			pragma::string::remove_quotes(name);
			//pragma::string::to_lower(name);
			//f.Seek(f.Tell() +1);
			c = FindFirstNotOf(f, pragma::string::WHITESPACE);
			if(c == -1)
				return false;
			ident = ident.substr(1);
			pragma::string::to_lower(ident);
			if(c != '{') {
				auto value = ReadValue(c, f);
				pragma::string::remove_quotes(value);
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
			auto c = FindFirstNotOf(f, pragma::string::WHITESPACE);
			if(c == pragma::string::NOT_FOUND)
				return false;
			switch(c) {
			case '{':
				{
					auto sub = std::make_shared<pragma::datasystem::Block>(*dataSettings);
					bool r;
					do
						r = read_block_data(*sub, enums, dataSettings, f, listID, blockType);
					while(r == true);
					auto subBase = std::static_pointer_cast<pragma::datasystem::Base>(sub);
					block.AddData(ident, subBase);
					listID = 0;
					f.Seek(f.Tell() + 1);
					break;
				}
			case ',':
				{
					c = FindFirstNotOf(f, pragma::string::WHITESPACE);
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

std::shared_ptr<pragma::datasystem::Block> pragma::datasystem::System::ReadData(ufile::IFile &f, const std::unordered_map<std::string, std::string> &enums)
{
	auto dataSettings = pragma::datasystem::create_data_settings(enums);

	auto data = std::make_shared<Block>(*dataSettings);
	auto listID = 0;
	// f.IgnoreComments("//");
	// f.IgnoreComments("/*","*/");

	if(read_block_data(*data, enums, dataSettings, f, listID, "", true) == false)
		return nullptr;
	return data;
}
std::shared_ptr<pragma::datasystem::Block> pragma::datasystem::System::LoadData(const char *path, const std::unordered_map<std::string, std::string> &enums)
{
	auto f = pragma::fs::open_file(path, pragma::fs::FileMode::Read);
	if(f == nullptr)
		return nullptr;
	fs::File fp {f};
	return ReadData(fp, enums);
}

////////////////////////

pragma::datasystem::String::String(Settings &dataSettings, const std::string &value) : Value(dataSettings), m_value(value) {}
pragma::datasystem::Value *pragma::datasystem::String::Copy() { return new String(*m_dataSettings, GetValue()); }
pragma::datasystem::ValueType pragma::datasystem::String::GetType() const { return ValueType::String; }

const std::string &pragma::datasystem::String::GetValue() const { return m_value; }
void pragma::datasystem::String::SetValue(const std::string &value) { m_value = value; }

std::string pragma::datasystem::String::GetString() const { return m_value; }
int pragma::datasystem::String::GetInt() const { return pragma::util::to_int(m_value); }
float pragma::datasystem::String::GetFloat() const { return pragma::util::to_float(m_value); }
bool pragma::datasystem::String::GetBool() const { return pragma::util::to_boolean(m_value); }
Color pragma::datasystem::String::GetColor() const { return ::Color {m_value}; }
Vector3 pragma::datasystem::String::GetVector() const { return uvec::create(m_value); }
Vector2 pragma::datasystem::String::GetVector2() const
{
	auto v = uvec::create(m_value);
	return ::Vector2 {v.x, v.y};
}
Vector4 pragma::datasystem::String::GetVector4() const { return uvec::create_v4(m_value); }
std::string pragma::datasystem::String::GetTypeString() const { return "string"; }

////////////////////////

pragma::datasystem::Int::Int(Settings &dataSettings, const std::string &value) : Value(dataSettings)
{
	if(GetDataSettings().ParseExpression(value, m_value) == false)
		m_value = pragma::util::to_int(value);
}
pragma::datasystem::Int::Int(Settings &dataSettings, int32_t value) : Value(dataSettings), m_value(value) {}
pragma::datasystem::Value *pragma::datasystem::Int::Copy() { return new Int(*m_dataSettings, m_value); }
pragma::datasystem::ValueType pragma::datasystem::Int::GetType() const { return ValueType::Int; }
int32_t pragma::datasystem::Int::GetValue() const { return m_value; }
void pragma::datasystem::Int::SetValue(int32_t value) { m_value = value; }

std::string pragma::datasystem::Int::GetString() const { return std::to_string(m_value); }
int pragma::datasystem::Int::GetInt() const { return m_value; }
float pragma::datasystem::Int::GetFloat() const { return static_cast<float>(m_value); }
bool pragma::datasystem::Int::GetBool() const { return (m_value != 0) ? true : false; }
Color pragma::datasystem::Int::GetColor() const { return ::Color {static_cast<int16_t>(m_value), static_cast<int16_t>(m_value), static_cast<int16_t>(m_value), 255}; }
Vector3 pragma::datasystem::Int::GetVector() const { return Vector3 {m_value, m_value, m_value}; }
Vector2 pragma::datasystem::Int::GetVector2() const { return ::Vector2 {m_value, m_value}; }
Vector4 pragma::datasystem::Int::GetVector4() const { return ::Vector4 {m_value, m_value, m_value, m_value}; }
std::string pragma::datasystem::Int::GetTypeString() const { return "int"; }

////////////////////////

pragma::datasystem::Float::Float(Settings &dataSettings, const std::string &value) : Value(dataSettings)
{
	if(GetDataSettings().ParseExpression(value, m_value) == false)
		m_value = pragma::util::to_int(value);
}
pragma::datasystem::Float::Float(Settings &dataSettings, float value) : Value(dataSettings), m_value(value) {}
pragma::datasystem::Value *pragma::datasystem::Float::Copy() { return new Float(*m_dataSettings, m_value); }
pragma::datasystem::ValueType pragma::datasystem::Float::GetType() const { return ValueType::Float; }
float pragma::datasystem::Float::GetValue() const { return m_value; }
void pragma::datasystem::Float::SetValue(float value) { m_value = value; }

std::string pragma::datasystem::Float::GetString() const { return std::to_string(m_value); }
int pragma::datasystem::Float::GetInt() const { return m_value; }
float pragma::datasystem::Float::GetFloat() const { return static_cast<float>(m_value); }
bool pragma::datasystem::Float::GetBool() const { return (m_value != 0) ? true : false; }
Color pragma::datasystem::Float::GetColor() const
{
	auto v = static_cast<int16_t>(m_value * 255.f);
	return ::Color {v, v, v, 255};
}
Vector3 pragma::datasystem::Float::GetVector() const { return Vector3 {m_value, m_value, m_value}; }
Vector2 pragma::datasystem::Float::GetVector2() const { return ::Vector2 {m_value, m_value}; }
Vector4 pragma::datasystem::Float::GetVector4() const { return ::Vector4 {m_value, m_value, m_value, m_value}; }
std::string pragma::datasystem::Float::GetTypeString() const { return "float"; }

////////////////////////

pragma::datasystem::Bool::Bool(Settings &dataSettings, const std::string &value) : Value(dataSettings), m_value(pragma::util::to_boolean(value)) {}
pragma::datasystem::Bool::Bool(Settings &dataSettings, bool value) : Value(dataSettings), m_value(value) {}
pragma::datasystem::Value *pragma::datasystem::Bool::Copy() { return new Bool(*m_dataSettings, m_value); }
pragma::datasystem::ValueType pragma::datasystem::Bool::GetType() const { return ValueType::Bool; }
bool pragma::datasystem::Bool::GetValue() const { return m_value; }
void pragma::datasystem::Bool::SetValue(bool value) { m_value = value; }

std::string pragma::datasystem::Bool::GetString() const { return std::to_string(m_value); }
int pragma::datasystem::Bool::GetInt() const { return m_value; }
float pragma::datasystem::Bool::GetFloat() const { return static_cast<float>(m_value); }
bool pragma::datasystem::Bool::GetBool() const { return (m_value != 0) ? true : false; }
Color pragma::datasystem::Bool::GetColor() const { return ::Color {m_value ? 255 : 0, m_value ? 255 : 0, m_value ? 255 : 0, 255}; }
Vector3 pragma::datasystem::Bool::GetVector() const { return Vector3 {m_value, m_value, m_value}; }
Vector2 pragma::datasystem::Bool::GetVector2() const { return ::Vector2 {m_value, m_value}; }
Vector4 pragma::datasystem::Bool::GetVector4() const { return ::Vector4 {m_value, m_value, m_value, m_value}; }
std::string pragma::datasystem::Bool::GetTypeString() const { return "bool"; }
