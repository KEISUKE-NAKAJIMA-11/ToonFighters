#pragma once
//min関数とmax関数を正確に使うため
#undef max
#undef min

//include↓
#include <sstream>
#include <fstream>
#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/string.hpp"
//include↑

//汎用的に使いまわせるようにテンプレート化
template<class CLASS>
inline bool Load(std::string _full_file_path, const char* _object_name, CLASS& _instance)
{
	std::string bin = "bin";
	std::string json = "json";
	size_t start = _full_file_path.find_last_of(".") + 1;
	std::string extend = _full_file_path.substr(start, _full_file_path.size() - start);
	auto openMode = (bin == extend) ? std::ios::in | std::ios::binary : std::ios::in;
	std::ifstream ifs(_full_file_path, openMode);
	if (!ifs.is_open()) { return false; }
	// else

	std::stringstream ss{};
	ss << ifs.rdbuf();

	if (bin == extend)
	{
		cereal::BinaryInputArchive binInArchive(ss);
		binInArchive(cereal::make_nvp(_object_name, _instance));
	}
	else if (json == extend)
	{
		cereal::JSONInputArchive jsonInArchive(ss);
		jsonInArchive(cereal::make_nvp(_object_name, _instance));
	}
	else
		return false;

	ifs.close();
	ss.clear();

	return true;
}

template<class CLASS>
inline bool Save(std::string _full_file_path, const char* _object_name, CLASS& _instance)
{
	size_t start = _full_file_path.find_last_of(".") + 1;
	std::string extend = _full_file_path.substr(start, _full_file_path.size() - start);

	std::stringstream ss{};
	std::string bin = "bin";
	std::string json = "json";
	if (bin == extend)
	{
		cereal::BinaryOutputArchive binOutArchive(ss);
		binOutArchive(cereal::make_nvp(_object_name, _instance));
	}
	else if (json == extend)
	{
		cereal::JSONOutputArchive jsonOutArchive(ss);
		jsonOutArchive(cereal::make_nvp(_object_name, _instance));
	}
	else
		return false;

	auto openMode = (bin == extend) ? std::ios::out | std::ios::binary : std::ios::out;
	std::ofstream ofs(_full_file_path, openMode);
	if (!ofs.is_open()) { return false; }
	// else

	ofs << ss.str();

	ofs.close();
	ss.clear();

	return true;
}
