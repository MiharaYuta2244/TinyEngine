#include "EffectEditorScene.h"
#include "EaseNames.h"
#include "EffectBuilder.h"
#include "EffectLibrary.h"
#include "SceneManager.h"
#include <algorithm>
#include <filesystem>
#include <numbers>

using namespace TinyEngine;

namespace {
#ifdef USE_IMGUI
bool DragRange(const char* label, FloatRange& r, float speed = 0.01f, float lo = -1000.0f, float hi = 1000.0f) {
	float v[2] = {r.min, r.max};
	if (ImGui::DragFloat2(label, v, speed, lo, hi)) {
		r.min = v[0];
		r.max = v[1];
		return true;
	}
	return false;
}

bool EaseCombo(const char* label, EaseType& e) {
	int v = static_cast<int>(e);
	if (ImGui::Combo(label, &v, kEaseNames, static_cast<int>(std::size(kEaseNames)))) {
		e = static_cast<EaseType>(v);
		return true;
	}
	return false;
}
#endif
} // namespace

void EffectEditorScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;
	commonData_ = ctx_.sceneManager->GetCommonData();

	// 他シーンのポストエフェクトを持ち越さない
	ctx_.engineContext->postEffectPipeline->SetEffects({PostEffectType::FullScreen});

	// Particleは生成時にデフォルトカメラを掴むので先に設定する
	ctx_.currentCamera->SetRotate({0.5f, 0.0f, 0.0f});
	ctx_.currentCamera->SetTranslation({0.0f, 10.0f, -20.0f});
	ctx_.engineContext->object3dCommon->SetDefaultCamera(ctx_.currentCamera);
	ctx_.engineContext->particleCommon->SetDefaultCamera(ctx_.currentCamera);

	// 床
	floor_ = std::make_unique<ObjectRender>();
	floor_->Initialize(ctx_.engineContext, "plane.obj");
	floor_->SetColor({0.05f, 0.05f, 0.08f, 1.0f});
	floorTransform_.scale = {30.0f, 30.0f, 1.0f};
	floorTransform_.rotate = {-std::numbers::pi_v<float> / 2.0f, 0.0f, 0.0f};
	floorTransform_.translate = {0.0f, 0.0f, 0.0f};

	RefreshTextureList();

	// 先頭のエフェクトを選択
	auto names = EffectLibrary::Get().GetNames();
	if (!names.empty()) {
		SelectEffect(names.front());
	}
}

void EffectEditorScene::Update() {
	float deltaTime = ctx_.timeManager->GetDeltaTime();

#ifdef USE_IMGUI
	DrawEffectList();
	DrawEmitterList();
	DrawInspector();
	DrawPreviewControls();
	HandleShortcuts();
#endif

	UpdatePreview(deltaTime);
	floor_->Update(floorTransform_);
}

void EffectEditorScene::Draw() {
	floor_->Draw();
	for (auto& p : preview_) {
		p->Draw();
	}
}

void EffectEditorScene::Finalize() { preview_.clear(); }

// ---------------------------------------------------------------
// プレビュー
// ---------------------------------------------------------------
void EffectEditorScene::SelectEffect(const std::string& name) {
	selectedEffect_ = name;
	selectedEmitter_ = 0;
	std::snprintf(nameBuf_, sizeof(nameBuf_), "%s", name.c_str());
	RestartPreview();
}

void EffectEditorScene::MarkDirty() {
	dirty_.insert(selectedEffect_);
	restartDebounce_ = 0.25f; // 操作が止まってから作り直す
}

void EffectEditorScene::RestartPreview() {
	preview_.clear();
	replayTimer_ = 0.0f;
	restartDebounce_ = -1.0f;

	const EffectData* data = EffectLibrary::Get().Find(selectedEffect_);
	if (data) {
		preview_ = EffectBuilder::Build(ctx_.engineContext, *data, previewPos_);
	}
}

void EffectEditorScene::UpdatePreview(float deltaTime) {
	if (restartDebounce_ > 0.0f) {
		restartDebounce_ -= deltaTime;
		if (restartDebounce_ <= 0.0f) {
			RestartPreview();
		}
	}

	for (auto& p : preview_) {
		p->Update();
	}

	bool finished = !preview_.empty() && std::all_of(preview_.begin(), preview_.end(), [](const auto& p) { return p->IsFinished(); });
	if (autoReplay_ && finished) {
		replayTimer_ += deltaTime;
		if (replayTimer_ >= replayInterval_) {
			RestartPreview();
		}
	}
}

void EffectEditorScene::RefreshTextureList() {
	textureNames_.clear();
	for (const auto& entry : std::filesystem::directory_iterator("resources/textures/")) {
		if (!entry.is_regular_file()) {
			continue;
		}
		auto ext = entry.path().extension().string();
		if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga") {
			textureNames_.push_back(entry.path().filename().string());
		}
	}
	std::sort(textureNames_.begin(), textureNames_.end());
}

// ---------------------------------------------------------------
// ImGui
// ---------------------------------------------------------------
#ifdef USE_IMGUI
void EffectEditorScene::HandleShortcuts() {
	if (ImGui::GetIO().WantTextInput) {
		return;
	}
	bool ctrl = ctx_.keyboard->KeyDown(DIK_LCONTROL) || ctx_.keyboard->KeyDown(DIK_RCONTROL);
	if (ctrl && ctx_.keyboard->KeyTriggered(DIK_S) && !selectedEffect_.empty()) {
		if (EffectLibrary::Get().Save(selectedEffect_)) {
			dirty_.erase(selectedEffect_);
		}
	}
	if (ctx_.keyboard->KeyTriggered(DIK_SPACE)) {
		RestartPreview();
	}
}

void EffectEditorScene::DrawEffectList() {
	auto& lib = EffectLibrary::Get();
	ImGui::Begin("Effect List");

	for (const std::string& name : lib.GetNames()) {
		std::string label = name + (dirty_.count(name) ? " *" : "");
		if (ImGui::Selectable(label.c_str(), name == selectedEffect_)) {
			SelectEffect(name);
		}
	}

	ImGui::Separator();
	ImGui::InputText("Name", nameBuf_, sizeof(nameBuf_));

	if (ImGui::Button("New")) {
		if (lib.Create(nameBuf_)) {
			dirty_.insert(nameBuf_);
			SelectEffect(nameBuf_);
		}
	}

	if (!selectedEffect_.empty()) {
		ImGui::SameLine();
		if (ImGui::Button("Duplicate")) {
			std::string copyName = selectedEffect_ + "_Copy";
			while (lib.Find(copyName)) {
				copyName += "_";
			}
			if (lib.Duplicate(selectedEffect_, copyName)) {
				dirty_.insert(copyName);
				SelectEffect(copyName);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Rename")) {
			std::string old = selectedEffect_;
			if (lib.Rename(old, nameBuf_)) {
				dirty_.erase(old);
				SelectEffect(nameBuf_);
			}
		}

		if (ImGui::Button("Save")) {
			if (lib.Save(selectedEffect_)) {
				dirty_.erase(selectedEffect_);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Save All")) {
			lib.SaveAll();
			dirty_.clear();
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete")) {
			ImGui::OpenPopup("DeleteEffect");
		}
	}

	if (ImGui::BeginPopupModal("DeleteEffect", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("'%s' を削除します(JSONファイルも削除されます)。", selectedEffect_.c_str());
		if (ImGui::Button("OK", ImVec2(120, 0))) {
			lib.Remove(selectedEffect_);
			dirty_.erase(selectedEffect_);
			selectedEffect_.clear();
			selectedEmitter_ = -1;
			preview_.clear();
			auto names = lib.GetNames();
			if (!names.empty()) {
				SelectEffect(names.front());
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}

void EffectEditorScene::DrawEmitterList() {
	ImGui::Begin("Emitters");
	EffectData* fx = EffectLibrary::Get().FindMutable(selectedEffect_);
	if (!fx) {
		ImGui::TextUnformatted("エフェクトが選択されていません");
		ImGui::End();
		return;
	}

	auto& list = fx->emitters;
	for (int i = 0; i < static_cast<int>(list.size()); ++i) {
		ImGui::PushID(i);
		if (ImGui::Selectable(list[i].name.c_str(), i == selectedEmitter_)) {
			selectedEmitter_ = i;
		}
		ImGui::PopID();
	}

	ImGui::Separator();
	bool changed = false;
	if (ImGui::Button("Add")) {
		list.emplace_back();
		selectedEmitter_ = static_cast<int>(list.size()) - 1;
		changed = true;
	}

	bool valid = selectedEmitter_ >= 0 && selectedEmitter_ < static_cast<int>(list.size());
	if (valid) {
		ImGui::SameLine();
		if (ImGui::Button("Duplicate")) {
			EmitterData copy = list[selectedEmitter_];
			copy.name += "_Copy";
			list.insert(list.begin() + selectedEmitter_ + 1, copy);
			++selectedEmitter_;
			changed = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete")) {
			list.erase(list.begin() + selectedEmitter_);
			selectedEmitter_ = std::min(selectedEmitter_, static_cast<int>(list.size()) - 1);
			changed = true;
		}
		if (ImGui::Button("Up") && selectedEmitter_ > 0) {
			std::swap(list[selectedEmitter_], list[selectedEmitter_ - 1]);
			--selectedEmitter_;
			changed = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Down") && selectedEmitter_ + 1 < static_cast<int>(list.size())) {
			std::swap(list[selectedEmitter_], list[selectedEmitter_ + 1]);
			++selectedEmitter_;
			changed = true;
		}
	}

	if (changed) {
		MarkDirty();
	}
	ImGui::End();
}

void EffectEditorScene::DrawInspector() {
	ImGui::Begin("Inspector");
	EffectData* fx = EffectLibrary::Get().FindMutable(selectedEffect_);
	if (!fx || selectedEmitter_ < 0 || selectedEmitter_ >= static_cast<int>(fx->emitters.size())) {
		ImGui::TextUnformatted("エミッターが選択されていません");
		ImGui::End();
		return;
	}

	EmitterData& e = fx->emitters[selectedEmitter_];
	bool changed = DrawEmitterSettings(e);

	if (e.moduleType == "Generic") {
		changed |= DrawParams(e.params);
	} else {
		ImGui::Separator();
		ImGui::TextWrapped("既存モジュール '%s' は固定パラメータです。ModuleをGenericにすると細かく編集可能。", e.moduleType.c_str());
	}

	if (changed) {
		MarkDirty();
	}
	ImGui::End();
}

bool EffectEditorScene::TextureCombo(EmitterData& e) {
	bool changed = false;
	if (ImGui::BeginCombo("Texture", e.texture.c_str())) {
		for (const auto& name : textureNames_) {
			if (ImGui::Selectable(name.c_str(), name == e.texture)) {
				e.texture = name;
				changed = true;
			}
		}
		ImGui::EndCombo();
	}

	// サムネイル
	auto* srv = ctx_.engineContext->srvManager;
	uint32_t index = ctx_.engineContext->textureManager->GetSrvIndex(e.texture);
	ImGui::Image((ImTextureID)srv->GetGPUDescriptorHandle(index).ptr, ImVec2(64, 64));
	return changed;
}

bool EffectEditorScene::DrawEmitterSettings(EmitterData& e) {
	bool c = false;

	char buf[64];
	std::snprintf(buf, sizeof(buf), "%s", e.name.c_str());
	if (ImGui::InputText("Name", buf, sizeof(buf))) {
		e.name = buf;
		c = true;
	}

	c |= TextureCombo(e);

	static const char* kMeshNames[] = {"Square", "Ring", "Cylinder"};
	int mesh = static_cast<int>(e.mesh);
	if (ImGui::Combo("Mesh", &mesh, kMeshNames, IM_ARRAYSIZE(kMeshNames))) {
		e.mesh = static_cast<ParticleMeshType>(mesh);
		c = true;
	}

	static const char* kBlendNames[] = {"Additive", "Alpha"};
	int blend = static_cast<int>(e.blend);
	if (ImGui::Combo("Blend", &blend, kBlendNames, IM_ARRAYSIZE(kBlendNames))) {
		e.blend = static_cast<ParticleBlendMode>(blend);
		c = true;
	}

	// モジュール種別
	const auto& types = EffectBuilder::GetModuleTypeNames();
	if (ImGui::BeginCombo("Module", e.moduleType.c_str())) {
		for (const auto& t : types) {
			if (ImGui::Selectable(t.c_str(), t == e.moduleType)) {
				e.moduleType = t;
				c = true;
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Separator();
	c |= ImGui::Checkbox("Burst (1回だけ発生)", &e.burst);
	int count = static_cast<int>(e.count);
	if (ImGui::DragInt("Count", &count, 1.0f, 1, 1024)) {
		e.count = static_cast<uint32_t>(std::max(count, 1));
		c = true;
	}
	if (!e.burst) {
		c |= ImGui::Checkbox("Loop", &e.loop);
		if (!e.loop) {
			c |= ImGui::DragFloat("Duration", &e.duration, 0.01f, 0.01f, 30.0f);
		}
		c |= ImGui::DragFloat("Frequency", &e.frequency, 0.005f, 0.001f, 10.0f);
	}
	c |= ImGui::DragFloat3("Offset", &e.offset.x, 0.05f);
	c |= ImGui::DragFloat3("Extent", &e.extent.x, 0.05f, 0.0f, 100.0f);
	return c;
}

bool EffectEditorScene::DrawParams(ParticleParams& p) {
	bool c = false;

	if (ImGui::CollapsingHeader("Lifetime", ImGuiTreeNodeFlags_DefaultOpen)) {
		c |= DragRange("Life (min,max)", p.life, 0.01f, 0.01f, 30.0f);
	}

	if (ImGui::CollapsingHeader("Velocity", ImGuiTreeNodeFlags_DefaultOpen)) {
		static const char* kShapeNames[] = {"Cone", "CircleXZ"};
		int shape = static_cast<int>(p.shape);
		if (ImGui::Combo("Shape", &shape, kShapeNames, IM_ARRAYSIZE(kShapeNames))) {
			p.shape = static_cast<EmitShape>(shape);
			c = true;
		}
		if (p.shape == EmitShape::Cone) {
			c |= ImGui::DragFloat3("Direction", &p.direction.x, 0.01f);
			c |= ImGui::SliderFloat("Spread (deg)", &p.spreadAngle, 0.0f, 180.0f);
			c |= ImGui::Checkbox("Use Spawn Direction", &p.useSpawnDirection);
		} else {
			c |= DragRange("Y Velocity", p.yVelocity, 0.05f);
		}
		c |= DragRange("Speed (min,max)", p.speed, 0.05f, 0.0f, 200.0f);
		c |= ImGui::DragFloat3("Gravity", &p.gravity.x, 0.05f);
		c |= ImGui::SliderFloat("Damping", &p.damping, 0.5f, 1.0f, "%.3f");
	}

	if (ImGui::CollapsingHeader("Size", ImGuiTreeNodeFlags_DefaultOpen)) {
		c |= DragRange("Start Size", p.startSize, 0.01f, 0.0f, 100.0f);
		c |= ImGui::DragFloat3("Size Axis", &p.sizeAxis.x, 0.01f, 0.0f, 100.0f);
		c |= ImGui::DragFloat("End Scale", &p.endScale, 0.01f, 0.0f, 100.0f);
		c |= EaseCombo("Size Ease", p.sizeEase);
	}

	if (ImGui::CollapsingHeader("Rotation")) {
		c |= ImGui::DragFloat3("Initial Rotate (deg)", &p.initialRotateDeg.x, 1.0f);
		c |= DragRange("Spin (deg/s)", p.spinDeg, 1.0f, -3600.0f, 3600.0f);
	}

	if (ImGui::CollapsingHeader("Color", ImGuiTreeNodeFlags_DefaultOpen)) {
		c |= ImGui::ColorEdit4("Start A", &p.colorStartA.x);
		c |= ImGui::ColorEdit4("Start B", &p.colorStartB.x);
		c |= ImGui::ColorEdit4("End", &p.colorEnd.x);
		c |= EaseCombo("Color Ease", p.colorEase);
	}
	return c;
}

void EffectEditorScene::DrawPreviewControls() {
	ImGui::Begin("Preview");
	if (ImGui::Button("Play / Restart (Space)")) {
		RestartPreview();
	}
	ImGui::Checkbox("Auto Replay", &autoReplay_);
	ImGui::DragFloat("Replay Interval", &replayInterval_, 0.05f, 0.0f, 10.0f);
	if (ImGui::DragFloat3("Position", &previewPos_.x, 0.05f)) {
		RestartPreview();
	}
	ImGui::Separator();
	ImGui::TextUnformatted("Ctrl+S: 保存 / 右ドラッグ: 回転 / ホイール: 前後");
	if (ImGui::Button("Back to Title")) {
		RequestSceneChange("Title");
	}
	ImGui::End();
}
#endif // USE_IMGUI