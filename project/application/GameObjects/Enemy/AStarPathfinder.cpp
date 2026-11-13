#include "AStarPathfinder.h"
#include <algorithm>
#include <cmath>

// グリッド生成用の定数設定
const float NODE_RADIUS = 0.5f;                 // 1マスの半径
const float NODE_DIAMETER = NODE_RADIUS * 2.0f; // 1マスのサイズ
const int GRID_SIZE_X = 41;                     // 視界半径20を両方向に含める
const int GRID_SIZE_Z = 41;
const int ENDPOINT_SEARCH_RADIUS = 2;          // 壁際で代替マスを探す範囲

namespace {
bool CanConnect(const Vector3& start, const Vector3& end, const std::vector<AABB>& obstacles) {
	for (const auto& obstacle : obstacles) {
		float enter = 0.0f;
		float exit = 1.0f;
		auto clipAxis = [&](float origin, float delta, float minimum, float maximum) {
			if (delta == 0.0f) {
				return origin >= minimum && origin <= maximum;
			}
			float nearTime = (minimum - origin) / delta;
			float farTime = (maximum - origin) / delta;
			if (nearTime > farTime) {
				std::swap(nearTime, farTime);
			}
			enter = std::max(enter, nearTime);
			exit = std::min(exit, farTime);
			return enter <= exit;
		};
		if (clipAxis(start.x, end.x - start.x, obstacle.min.x, obstacle.max.x) &&
		    clipAxis(start.z, end.z - start.z, obstacle.min.z, obstacle.max.z) && exit > 0.0f && enter < 1.0f) {
			return false;
		}
	}
	return true;
}

float DistanceSquaredXZ(const Vector3& a, const Vector3& b) {
	const float dx = a.x - b.x;
	const float dz = a.z - b.z;
	return dx * dx + dz * dz;
}
}

std::vector<Vector3> AStarPathfinder::FindPath(const Vector3& startPos, const Vector3& targetPos, const std::vector<AABB>& obstacles) {
	// 探索用グリッドの動的生成
	Vector3 gridBottomLeft = startPos;
	gridBottomLeft.x -= (GRID_SIZE_X * NODE_RADIUS);
	gridBottomLeft.z -= (GRID_SIZE_Z * NODE_RADIUS);
	gridBottomLeft.y = 0.0f;

	// 2次元配列でグリッドを確保
	std::vector<std::vector<PathNode>> grid;
	grid.resize(GRID_SIZE_X);
	for (int x = 0; x < GRID_SIZE_X; ++x) {
		grid[x].reserve(GRID_SIZE_Z);
		for (int z = 0; z < GRID_SIZE_Z; ++z) {
			Vector3 worldPoint = gridBottomLeft;
			worldPoint.x += (x * NODE_DIAMETER + NODE_RADIUS);
			worldPoint.z += (z * NODE_DIAMETER + NODE_RADIUS);

			// マスが壁と重なっているかチェック
			bool walkable = CheckWalkable(worldPoint, NODE_RADIUS, obstacles);
			grid[x].emplace_back(walkable, worldPoint, x, z);
		}
	}

	// スタートとターゲットのノードを取得
	PathNode* startNode = GetNodeFromWorldPoint(startPos, grid, gridBottomLeft, GRID_SIZE_X, GRID_SIZE_Z, NODE_DIAMETER);
	PathNode* targetNode = GetNodeFromWorldPoint(targetPos, grid, gridBottomLeft, GRID_SIZE_X, GRID_SIZE_Z, NODE_DIAMETER);

	// 探索範囲外（グリッドの外）の場合は空の経路を返す
	if (!startNode || !targetNode)
		return {};

	// A*の探索準備 (OpenリストとClosedリスト)
	std::vector<PathNode*> openSet; // 評価候補のノード
	// Closedリストは検索を高速化するため bool の二次元配列を使用
	std::vector<std::vector<bool>> closedSet(GRID_SIZE_X, std::vector<bool>(GRID_SIZE_Z, false));

	if (startNode->isWalkable) {
		openSet.push_back(startNode);
	} else {
		// 実座標から壁を横切らずに入れるマスを探索の起点にする
		const int minX = std::max(0, startNode->gridX - ENDPOINT_SEARCH_RADIUS);
		const int maxX = std::min(GRID_SIZE_X - 1, startNode->gridX + ENDPOINT_SEARCH_RADIUS);
		const int minZ = std::max(0, startNode->gridZ - ENDPOINT_SEARCH_RADIUS);
		const int maxZ = std::min(GRID_SIZE_Z - 1, startNode->gridZ + ENDPOINT_SEARCH_RADIUS);
		for (int x = minX; x <= maxX; ++x) {
			for (int z = minZ; z <= maxZ; ++z) {
				auto& node = grid[x][z];
				if (!node.isWalkable || !CanConnect(startPos, node.worldPosition, obstacles)) {
					continue;
				}
				node.gCost = 10.0f * std::sqrt(DistanceSquaredXZ(startPos, node.worldPosition)) / NODE_DIAMETER;
				node.hCost = GetDistance(&node, targetNode);
				openSet.push_back(&node);
			}
		}
	}

	PathNode* reachedNode = nullptr;
	float closestDistanceSq = 0.0f;

	// メインループ
	while (!openSet.empty()) {
		// Openリストの中で最もfCostが低いノードを取り出す
		PathNode* currentNode = openSet[0];
		int currentIndex = 0;
		for (int i = 1; i < openSet.size(); ++i) {
			if (openSet[i]->fCost() < currentNode->fCost() || (openSet[i]->fCost() == currentNode->fCost() && openSet[i]->hCost < currentNode->hCost)) {
				currentNode = openSet[i];
				currentIndex = i;
			}
		}

		// 現在のノードをOpenから外し、Closedに入れる
		openSet.erase(openSet.begin() + currentIndex);
		closedSet[currentNode->gridX][currentNode->gridZ] = true;

		// ゴールに到達したか判定
		if (currentNode == targetNode) {
			reachedNode = currentNode;
			break;
		}

		// 目標マスが壁際なら、到達可能な候補のうち実座標に最も近いマスを選ぶ
		if (!targetNode->isWalkable && std::abs(currentNode->gridX - targetNode->gridX) <= ENDPOINT_SEARCH_RADIUS &&
		    std::abs(currentNode->gridZ - targetNode->gridZ) <= ENDPOINT_SEARCH_RADIUS) {
			const float distanceSq = DistanceSquaredXZ(currentNode->worldPosition, targetPos);
			if ((!reachedNode || distanceSq < closestDistanceSq) && CanConnect(currentNode->worldPosition, targetPos, obstacles)) {
				reachedNode = currentNode;
				closestDistanceSq = distanceSq;
			}
		}

		// 隣接する周囲8方向のマスを調べる
		for (PathNode* neighbor : GetNeighbors(currentNode, grid, GRID_SIZE_X, GRID_SIZE_Z)) {
			// 歩けない（壁）、または既に評価済み(Closed)ならスキップ
			if (!neighbor->isWalkable || closedSet[neighbor->gridX][neighbor->gridZ]) {
				continue;
			}

			// 現在のノード経由で隣のマスへ行く時のコスト
			float newMovementCostToNeighbor = currentNode->gCost + GetDistance(currentNode, neighbor);

			// Openリストに含まれているか
			auto it = std::find(openSet.begin(), openSet.end(), neighbor);
			bool isInOpenSet = (it != openSet.end());

			// より短い経路が見つかった、もしくはまだOpenリストに入っていない場合
			if (newMovementCostToNeighbor < neighbor->gCost || !isInOpenSet) {
				neighbor->gCost = newMovementCostToNeighbor;
				neighbor->hCost = GetDistance(neighbor, targetNode);
				neighbor->parent = currentNode;

				if (!isInOpenSet) {
					openSet.push_back(neighbor);
				}
			}
		}
	}

	if (!reachedNode) {
		return {};
	}

	std::vector<Vector3> path;
	if (reachedNode == targetNode) {
		path.push_back(targetPos);
	}
	for (PathNode* node = reachedNode; node && node != startNode; node = node->parent) {
		path.push_back(node->worldPosition);
	}
	std::reverse(path.begin(), path.end());
	return path;
}

PathNode*
    AStarPathfinder::GetNodeFromWorldPoint(const Vector3& worldPosition, std::vector<std::vector<PathNode>>& grid, const Vector3& gridBottomLeft, int gridSizeX, int gridSizeZ, float nodeDiameter) {
	const float x = (worldPosition.x - gridBottomLeft.x) / nodeDiameter;
	const float z = (worldPosition.z - gridBottomLeft.z) / nodeDiameter;
	if (x >= 0.0f && x < gridSizeX && z >= 0.0f && z < gridSizeZ) {
		return &grid[static_cast<int>(x)][static_cast<int>(z)];
	}
	return nullptr;
}

std::vector<PathNode*> AStarPathfinder::GetNeighbors(PathNode* node, std::vector<std::vector<PathNode>>& grid, int gridSizeX, int gridSizeZ) {
	std::vector<PathNode*> neighbors;

	// 周囲8方向をループ
	for (int x = -1; x <= 1; ++x) {
		for (int z = -1; z <= 1; ++z) {
			if (x == 0 && z == 0)
				continue; // 自分自身は除外

			int checkX = node->gridX + x;
			int checkZ = node->gridZ + z;

			// グリッドの配列範囲内かチェック
			if (checkX >= 0 && checkX < gridSizeX && checkZ >= 0 && checkZ < gridSizeZ) {
				// 斜め移動では隣の壁の角を横切らない
				if (x != 0 && z != 0 && (!grid[checkX][node->gridZ].isWalkable || !grid[node->gridX][checkZ].isWalkable)) {
					continue;
				}
				neighbors.push_back(&grid[checkX][checkZ]);
			}
		}
	}

	return neighbors;
}

float AStarPathfinder::GetDistance(PathNode* nodeA, PathNode* nodeB) {
	int dstX = std::abs(nodeA->gridX - nodeB->gridX);
	int dstZ = std::abs(nodeA->gridZ - nodeB->gridZ);

	// A*の定番の距離計算（直線移動コストを10、斜め移動コストを14とする近似計算）
	if (dstX > dstZ)
		return 14.0f * dstZ + 10.0f * (dstX - dstZ);
	return 14.0f * dstX + 10.0f * (dstZ - dstX);
}

bool AStarPathfinder::CheckWalkable(const Vector3& nodePos, float nodeRadius, const std::vector<AABB>& obstacles) {
	// マス目の2D AABB
	float minX = nodePos.x - nodeRadius;
	float maxX = nodePos.x + nodeRadius;
	float minZ = nodePos.z - nodeRadius;
	float maxZ = nodePos.z + nodeRadius;

	for (const auto& wallCol : obstacles) {
		// 2D平面でのAABB同士の交差判定
		if (minX <= wallCol.max.x && maxX >= wallCol.min.x && minZ <= wallCol.max.z && maxZ >= wallCol.min.z) {
			return false; // 壁と重なっているので歩けない
		}
	}

	return true; // どの壁とも重なっていない
}
