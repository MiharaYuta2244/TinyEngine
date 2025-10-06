#include "Game.h"
#include "WinApp.h"
#include <memory>

#if defined(_MSC_VER) && !defined(_rotr64)
#include <intrin.h>
inline unsigned __int64 _rotr64(unsigned __int64 value, unsigned int shift) noexcept {
	shift &= 63;
	return (value >> shift) | (value << (64 - shift));
}
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	// ゲームクラス生成
	std::unique_ptr<Game> game = std::make_unique<Game>();

	// ゲームクラス初期化
	game->Initialize(hInstance);

	// ウィンドウのxボタンが押されるまでループ
	while (game->GetDxCommon()->GetWinApp()->ProcessMessage()) {
		// 更新処理
		game->Update();

		// 描画処理
		game->Draw();
	}

	// 後処理
	game->Finalize();

	return 0;
}