#include <Windows.h>
#include <KamataEngine.h>

#include "Scene/SceneManager.h"
#include "Scene/Title/TitleScene.h"

#include <memory>

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

    // エンジンの初期化
    Initialize(L""); {
        // 描画開始/終了を行うDirectX共通クラス
        DirectXCommon* dxCommon = DirectXCommon::GetInstance();

        // シーン管理
        SceneManager sceneManager(std::make_unique<TitleScene>());

        // メインループ
        while (true) {
            // エンジン更新
            if (Update()) { break; }

            sceneManager.Update();
            if (sceneManager.IsEnd()) { break; }

#ifdef USE_IMGUI
            ImGuiManager::GetInstance()->Begin();
#endif

            // 描画処理開始
            dxCommon->PreDraw();
            sceneManager.Draw();

#ifdef USE_IMGUI
            ImGuiManager::GetInstance()->End();
            ImGuiManager::GetInstance()->Draw();
#endif

            // 描画処理終了
            dxCommon->PostDraw();
        }
    }

    // KamataEngineの終了
    Finalize();

    return 0;
}
