#include <Windows.h>
#include <KamataEngine.h>

#include "Scene/SceneManager.h"
#include "Scene/Title/TitleScene.h"

#include <memory>

using namespace KamataEngine;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

    Initialize(L"一手先"); {
        DirectXCommon* dxCommon = DirectXCommon::GetInstance();

        SceneManager sceneManager(std::make_unique<TitleScene>());

        while (true) {
            if (Update()) { break; }

            sceneManager.Update();
            if (sceneManager.IsEnd()) { break; }

#ifdef USE_IMGUI
            ImGuiManager::GetInstance()->Begin();
#endif

            dxCommon->PreDraw();
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
