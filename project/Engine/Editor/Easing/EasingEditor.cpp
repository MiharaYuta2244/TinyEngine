#include "EasingEditor.h"
#include <fstream>
#include <sstream>
#include <iostream>

#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif // USE_IMGUI

EasingEditor::EasingEditor() {
	// プレビュー用のアニメーションの初期化
	previewAnim_.Start(0.0f, 300.0f, 1.0f, EaseType::EASEOUTSINE);

	// 3Dオブジェクト用のアニメーション初期化
	objectAnim_.Start(startValue_, endValue_, duration_, EaseType::EASEOUTSINE);
}

void EasingEditor::DrawWindow(float deltaTime) {
#ifdef USE_IMGUI
	ImGui::Begin("Easing Editor");

	// イージングタイプの選択
	DrawEaseSelector();

	ImGui::Separator();

	// ファイルのセーブUI
	if(ImGui::Button("Save to CSV")){
		SaveToCSV("easing_data.csv");
	}

	ImGui::Separator();

	// ファイルのロードUI
	if (ImGui::Button("Load from CSV")) {
		LoadFromCSV("easing_data.csv");
	}

	ImGui::Separator();

	// パラメータの設定とカーブの可視化
	DrawCurveGraph();

	ImGui::Separator();

	// アニメーションのプレビュー
	DrawPreview(deltaTime);

	ImGui::End();
#endif
}

void EasingEditor::DrawEaseSelector() {
#ifdef USE_IMGUI
	// 対象の選択(SRT)
	if(ImGui::Combo("Target", &currentTargetIndex_, targetNames_, IM_ARRAYSIZE(targetNames_))){
		ResetPreview();
	}

	// 始点と終点の入力
	float* startPtr = reinterpret_cast<float*>(&startValue_);
	float* endPtr = reinterpret_cast<float*>(&endValue_);

	if(ImGui::DragFloat3("Start Value", startPtr, 0.1f)){
		ResetPreview();
	}

	if (ImGui::DragFloat3("End Value", endPtr, 0.1f)) {
		ResetPreview();
	}

	if (ImGui::Combo("Ease Type", &currentEaseIndex_, easeNames_, IM_ARRAYSIZE(easeNames_))) {
		// タイプが変更されたらプレビューをリセット
		ResetPreview();
	}

	if (ImGui::SliderFloat("Duration", &duration_, 0.1f, 5.0f, "%.2f s")) {
		ResetPreview();
	}
#endif
}

void EasingEditor::DrawCurveGraph() {
#ifdef USE_IMGUI
	ImGui::Text("Curve Preview");

	const int resolution = 100;
	float values[resolution];

	for (int i = 0; i < resolution; ++i) {
		float t = static_cast<float>(i) / (resolution - 1);
		// 選択中のイージング関数を適用してグラフ出たを生成
		values[i] = Easing::ApplyEasing(static_cast<EaseType>(currentEaseIndex_), t);
	}

	// グラフの描画
	ImGui::PlotLines("", values, resolution, 0, nullptr, -0.5f, 1.5f, ImVec2(ImGui::GetContentRegionAvail().x, 150));
#endif
}

void EasingEditor::DrawPreview(float deltaTime) {
#ifdef USE_IMGUI
	ImGui::Text("Animation Preview");

	// 再生
	if (ImGui::Button("Play")) {
		ResetPreview();
	}

	ImGui::SameLine();

	// 停止
	if (ImGui::Button("Stop")) {
		previewAnim_.Reset();
		objectAnim_.Reset();
		currentValue_ = 0.0f;
		currentObjectValue_ = startValue_;
	}

	// アニメーションの更新
	previewAnim_.Update(deltaTime, currentValue_);
	objectAnim_.Update(deltaTime, currentObjectValue_);

	// 実際の動きを視覚化
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 p = ImGui::GetCursorScreenPos();

	// 背景のレール
	float trackWidth = 300.0f;
	float boxSize = 20.0f;
	drawList->AddRectFilled(p, ImVec2(p.x + trackWidth + boxSize, p.y + boxSize), IM_COL32(50, 50, 50, 255));

	// 動くボックス
	ImVec2 boxPos(p.x + currentValue_, p.y);
	drawList->AddRectFilled(boxPos, ImVec2(boxPos.x + boxSize, boxPos.y + boxSize), IM_COL32(255, 100, 100, 255));

	// 次のUI要素が被らないようにカーソルを進める
	ImGui::Dummy(ImVec2(trackWidth, boxSize));
#endif
}

void EasingEditor::ResetPreview() {
	// 現在の設定でアニメーションをリスタート
	previewAnim_.Start(0.0f, 300.0f, duration_, static_cast<EaseType>(currentEaseIndex_));

	// 3Dオブジェクト用のアニメーションもリスタート
	objectAnim_.Start(startValue_, endValue_, duration_, static_cast<EaseType>(currentEaseIndex_));
}

void EasingEditor::SaveToCSV(const std::string& filename) {
	std::ofstream file(filename);
	if (file.is_open()) {
		// ヘッダー
		file << "TargetIndex,StartX,StartY,StartZ,EndX,EndY,EndZ,Duration,EaseTypeIndex\n";
		// データ
		file << currentTargetIndex_ << "," << startValue_.x << "," << startValue_.y << "," << startValue_.z << "," << endValue_.x << "," << endValue_.y << "," << endValue_.z << "," << duration_ << ","
		     << currentEaseIndex_ << "\n";
		file.close();
	}
}

void EasingEditor::LoadFromCSV(const std::string& filename) {
	std::ifstream file(filename);
	if (file.is_open()) {
		std::string line;
		std::getline(file, line);

		if (std::getline(file, line)) {
			std::stringstream ss(line);
			std::string item;

			std::getline(ss, item, ',');
			currentTargetIndex_ = std::stoi(item);
			std::getline(ss, item, ',');
			startValue_.x = std::stof(item);
			std::getline(ss, item, ',');
			startValue_.y = std::stof(item);
			std::getline(ss, item, ',');
			startValue_.z = std::stof(item);
			std::getline(ss, item, ',');
			endValue_.x = std::stof(item);
			std::getline(ss, item, ',');
			endValue_.y = std::stof(item);
			std::getline(ss, item, ',');
			endValue_.z = std::stof(item);
			std::getline(ss, item, ',');
			duration_ = std::stof(item);
			std::getline(ss, item, ',');
			currentEaseIndex_ = std::stoi(item);

			ResetPreview(); // ロードした設定でプレビューを更新
		}
		file.close();
	}
}
