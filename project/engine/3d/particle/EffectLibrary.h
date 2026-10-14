#pragma once
#include "EffectBuilder.h"
#include "Logger.h"
#include <map>

class EffectLibrary {
public:
	static EffectLibrary& Get();
	
	/// <summary>
	/// エフェクトのJsonファイルを全読込み
	/// </summary>
	void LoadAll(); 

	const EffectData* Find(const std::string& name) const;
	EffectData* FindMutable(const std::string& name);
	std::vector<std::string> GetNames() const;

	EffectData* Create(const std::string& name); // 新規(メモリ上のみ。Saveで書き出し)
	bool Duplicate(const std::string& src, const std::string& dst);
	bool Rename(const std::string& from, const std::string& to);
	bool Remove(const std::string& name); // ファイルも削除
	bool Save(const std::string& name);
	void SaveAll();

	// ゲーム側から使う窓口
	template<class Container> bool Spawn(EngineContext* ctx, const std::string& name, const Vector3& pos, Container& out, const EffectBuilder::SpawnOptions& opt = {}) const {
		const EffectData* data = Find(name);
		if (!data) {
			Logger::Log("[EffectLibrary] Effect not found: " + name, LogLevel::Warning);
			return false;
		}
		EffectBuilder::Spawn(ctx, *data, pos, out, opt);
		return true;
	}

	static bool IsValidName(const std::string& name);

private:
	std::map<std::string, EffectData> effects_;
};