#pragma once

#ifdef _DEBUG
/// <summary>
/// Direct3Dのリソースリークをチェックするクラス
/// </summary>
class D3DResourceLeakChecker {
public:
	~D3DResourceLeakChecker();
};
#else
class D3DResourceLeakChecker {};
#endif