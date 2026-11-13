#include "Decoration.h"
#include "Logger.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include <algorithm>

#ifdef USE_IMGUI
#include "imgui.h"
#endif

void Decoration::Initialize(EngineContext* ctx, const Transform& transform) {
	modelManager_ = ctx->modelManager;
	transform_ = transform;
	SetModel("Cube.obj");
}

CollisionTag Decoration::GetTag() const { return CollisionTag::Wall; }

std::string Decoration::GetName() const { return "Decoration (" + modelName_ + ")"; }

Model* Decoration::GetModel() const { return model_; }

Matrix4x4 Decoration::GetModelWorldMatrix() const {
	return model_->GetModelData().rootNode.localMatrix *
	       MathUtility::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
}

AABB Decoration::GetCollision() const { return GetAABBForGizmo(); }

AABB Decoration::GetAABBForGizmo() const {
	const auto& bounds = model_->GetLocalBounds();
	auto world = GetModelWorldMatrix();
	AABB result{};
	for (int corner = 0; corner < 8; ++corner) {
		Vector3 local{
		    (corner & 1) ? bounds.max.x : bounds.min.x,
		    (corner & 2) ? bounds.max.y : bounds.min.y,
		    (corner & 4) ? bounds.max.z : bounds.min.z,
		};

		const auto point = MathUtility::Transform(local, world);

		if (corner == 0) {
			result = {point, point};
		} else {
			result.min = {std::min(result.min.x, point.x), std::min(result.min.y, point.y), std::min(result.min.z, point.z)};
			result.max = {std::max(result.max.x, point.x), std::max(result.max.y, point.y), std::max(result.max.z, point.z)};
		}
	}

	return result;
}

void Decoration::Draw() {}

void Decoration::ToExtraJson(Json& j) const { j["model"] = modelName_; }

void Decoration::FromExtraJson(const Json& j) {
	const auto model = j.find("model");

	if (model != j.end() && model->is_string() && SetModel(model->get_ref<const std::string&>())) {
		return;
	}

	Logger::Log("[Decoration] Missing or unknown OBJ model. Using Cube.obj.", LogLevel::Warning);
}

void Decoration::DrawExtraImGui() {
#ifdef USE_IMGUI
	if (ImGui::BeginCombo("Model", modelName_.c_str())) {
		for (const auto& name : modelManager_->GetObjModelNames()) {
			if (ImGui::Selectable(name.c_str(), name == modelName_)) {
				SetModel(name);
			}
		}
		ImGui::EndCombo();
	}
#endif
}

bool Decoration::SetModel(const std::string& name) {
	if (!name.ends_with(".obj")) {
		return false;
	}

	auto* model = modelManager_->FindModel(name);

	if (!model) {
		return false;
	}

	model_ = model;
	modelName_ = name;
	return true;
}
