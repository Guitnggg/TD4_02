#include <Windows.h>
#include <KamataEngine.h>

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

    // エンジンの初期化
    KamataEngine::Initialize(L""); {
        // 描画開始/終了を行うDirectX共通クラス
        KamataEngine::DirectXCommon* dxCommon = KamataEngine::DirectXCommon::GetInstance();

        // メインループ
        while (true) {
            // エンジン更新
            if (KamataEngine::Update()) { break; }

            // 描画処理開始
            dxCommon->PreDraw();



            // 描画処理終了
            dxCommon->PostDraw();
        }
    }

    // KamataEngineの終了
    KamataEngine::Finalize();

    return 0;
}
