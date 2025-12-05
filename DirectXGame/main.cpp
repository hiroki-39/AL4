#include "GameScene.h"
#include "KamataEngine.h"
#include "TitleScene.h"
#include <Windows.h>

using namespace KamataEngine;

TitleScene* titleScene = nullptr;
GameScene* gameScene = nullptr;

// シーン
enum class Scene {
	kUnknown = 0,
	kTitle,
	kSelect,
	kGame,
	kclear,
};

// 現在のシーン
Scene scene = Scene::kUnknown;

// シーン切り替え処理
void ChangeScene() {

	switch (scene) {
	case Scene::kTitle:

		if (titleScene->IsFinished()) {
			// シーン変更
			scene = Scene::kGame;
			// 前のシーンを削除
			delete titleScene;

			// 新しいシーンの作成と初期化
			titleScene = nullptr;
			gameScene = new GameScene;
			gameScene->Initialize();
		}

		break;
	case Scene::kGame:

		if (gameScene->IsFinished()) {
			// シーン変更
			scene = Scene::kTitle;
			delete gameScene;

			// 前のシーンの生成と初期化
			gameScene = nullptr;
			titleScene = new TitleScene;
			titleScene->Initialize();
		}

		break;
	}
}

// シーンの更新
void UpdateScene() {

	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	}
}

// シーンの描画
void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw();
		break;
	case Scene::kGame:
		gameScene->Draw();
		break;
	}
}


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"LE2B_04_カトウ_ヒロキ_襲撃");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// ImGuiManagerインスタンスの取得
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	// 最初のシーンの初期化
	scene = Scene::kTitle;
	titleScene = new TitleScene;
	titleScene->Initialize();

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// ImGui受付開始
		imguiManager->Begin();

		// シーン切り替え
		ChangeScene();


		// シーン更新
		UpdateScene();

		// ImGui受付終了
		imguiManager->End();

		// 描画開始
		dxCommon->PreDraw();

		// シーンの描画
		DrawScene();

		// ImGui描画
		imguiManager->Draw();

		// 描画終了
		dxCommon->PostDraw();
	}


	delete titleScene;
	// ゲームシーンの解放
	delete gameScene;
	// nullptrの代入
	gameScene = nullptr;

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}
