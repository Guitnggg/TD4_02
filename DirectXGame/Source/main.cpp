#include <Windows.h>
#include <KamataEngine.h>

#include "Scene/SceneManager.h"
#include "Scene/Title/TitleScene.h"

#include <memory>

using namespace KamataEngine;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// すべてのシーンと描画リソースをエンジンの初期化・終了処理内で管理する。

    Initialize(L"4244_一手先"); {
        // DebugTextはKamataEngine::Initialize()では初期化されないため、個別に初期化する。
        DebugText::GetInstance()->Initialize();
        DirectXCommon* dxCommon = DirectXCommon::GetInstance();

        SceneManager sceneManager(std::make_unique<TitleScene>());

        while (true) {
			// 入力・ウィンドウ更新から終了要求が返された場合はループを抜ける。
            if (Update()) { break; }

            sceneManager.Update();
            if (sceneManager.IsEnd()) { break; }

#ifdef USE_IMGUI
			// 各シーンのデバッグUIはBeginからEndまでの間に登録する。
            ImGuiManager::GetInstance()->Begin();
#endif

            dxCommon->PreDraw();
			// シーン描画では3D空間と2DのUIをまとめて描画する。
            sceneManager.Draw();

#ifdef USE_IMGUI
            ImGuiManager::GetInstance()->End();
            ImGuiManager::GetInstance()->Draw();
#endif

            dxCommon->PostDraw();
        }
    }

    Finalize();

    return 0;
}
