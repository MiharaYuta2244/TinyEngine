#pragma once
#include "DirectXCommon.h"

/// <summary>
///  モデルの共通クラス
/// </summary>
class ModelCommon {
public:
	// 初期化関数
	void Initialize(DirectXCommon* dxCommon);

	// getter
	DirectXCommon* GetDxCommon() const { return dxCommon_; }

private:
	DirectXCommon* dxCommon_;
};
