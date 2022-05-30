#include "collision.h"
#include "judge_manager.h"
#include "framework.h"

bool Collision::HitSphere(const DirectX::XMFLOAT3& p1, float r1, const DirectX::XMFLOAT3& p2, float r2)
{
	//	半径の合算の2乗
	const float R2 = (r1 + r2) * (r1 + r2);

	//	中心同士の距離の2乗
	DirectX::XMFLOAT3 vec;
	vec.x = p1.x - p2.x;
	vec.y = p1.y - p2.y;
	vec.z = p1.z - p2.z;
	float L2 = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;

	//	衝突判定
	if (L2 < R2)	return	true;

	return	false;
}

bool	Collision::IntersectRayVsModel(const DirectX::XMFLOAT3& start,
	const DirectX::XMFLOAT3& end,
	const Model* model,
	HitResult& result)
{
	DirectX::XMVECTOR	WorldStart = DirectX::XMLoadFloat3(&start);
	DirectX::XMVECTOR	WorldEnd = DirectX::XMLoadFloat3(&end);
	DirectX::XMVECTOR	WorldRayVec = DirectX::XMVectorSubtract(WorldEnd, WorldStart);
	DirectX::XMVECTOR	WorldRayLength = DirectX::XMVector3Length(WorldRayVec);

	DirectX::XMStoreFloat(&result.distance, WorldRayLength);

	bool	hit = false;
		ModelResource* resource = model->GetResource();
	for (auto& mesh : resource->GetMeshes())
	{
		auto& node = model->GetNodes().at(mesh.node_index);

		//	ワールド空間からノードのローカル空間に変換
		DirectX::XMMATRIX	WorldTransform = DirectX::XMLoadFloat4x4(&node.worldTransform);
		DirectX::XMMATRIX	InverseWorldTransform = DirectX::XMMatrixInverse(nullptr, WorldTransform);

		DirectX::XMVECTOR	Start = DirectX::XMVector3TransformCoord(WorldStart, InverseWorldTransform);
		DirectX::XMVECTOR	End = DirectX::XMVector3TransformCoord(WorldEnd, InverseWorldTransform);
		DirectX::XMVECTOR	Vec = DirectX::XMVectorSubtract(End, Start);
		DirectX::XMVECTOR	Dir = DirectX::XMVector3Normalize(Vec);
		DirectX::XMVECTOR	Length = DirectX::XMVector3Length(Vec);

		float	neart;
		DirectX::XMStoreFloat(&neart, Length);

		int	materialIndex = -1;
		DirectX::XMVECTOR	HitPosition, HitNormal;

		DirectX::XMFLOAT3	vec;
		DirectX::XMStoreFloat3(&vec, Vec);
		if (vec.x == 0 && vec.z == 0)
		{
			materialIndex = IntersectRayVsModelY(Start, Dir, neart, mesh, HitPosition, HitNormal);
		}
		else	if (vec.x == 0 && vec.y == 0)
		{
			materialIndex = IntersectRayVsModelZ(Start, Dir, neart, mesh, HitPosition, HitNormal);
		}
		else	if (vec.y == 0 && vec.z == 0)
		{
			materialIndex = IntersectRayVsModelX(Start, Dir, neart, mesh, HitPosition, HitNormal);
		}
		else
		{
			materialIndex = IntersectRayVsModel(Start, Dir, neart, mesh, HitPosition, HitNormal);
		}
		if (materialIndex >= 0)
		{
			DirectX::XMVECTOR	WorldPosition = DirectX::XMVector3TransformCoord(HitPosition, WorldTransform);
			DirectX::XMVECTOR	WorldCrossVec = DirectX::XMVectorSubtract(WorldPosition, WorldStart);
			DirectX::XMVECTOR	WorldCrossLength = DirectX::XMVector3Length(WorldCrossVec);
			float	distance;
			DirectX::XMStoreFloat(&distance, WorldCrossLength);

			if (result.distance > distance)
			{
				DirectX::XMVECTOR	WorldNormal = DirectX::XMVector3TransformNormal(HitNormal, WorldTransform);
				result.distance = distance;
				result.materialIndex = materialIndex;
				DirectX::XMStoreFloat3(&result.position, WorldPosition);
				DirectX::XMStoreFloat3(&result.normal, DirectX::XMVector3Normalize(WorldNormal));
				hit = true;
			}
		}
	}
	return	hit;
}

//	X軸専用
int	Collision::IntersectRayVsModelX(const DirectX::XMVECTOR& Start,
	const DirectX::XMVECTOR& Dir,
	float	neart,
	const ModelResource::s_mesh& mesh,
	DirectX::XMVECTOR& hitPosition,
	DirectX::XMVECTOR& hitNormal)
{
	const	std::vector<ModelResource::s_vertex>& vertices = mesh.vertices;
	const	std::vector<int>& indices = mesh.indices;
	const	ModelResource::s_vertex* pVertices = vertices.data();
	const	int* pIndices = indices.data();

	int	materialIndex = -1;
	for (const ModelResource::s_subset& subset : mesh.subsets)
	{
		for (UINT ii = 0; ii < subset.index_count; ii += 3)
		{
			UINT	index = subset.index_start + ii;
			const	ModelResource::s_vertex& a = pVertices[pIndices[index + 0]];
			const	ModelResource::s_vertex& b = pVertices[pIndices[index + 1]];
			const	ModelResource::s_vertex& c = pVertices[pIndices[index + 2]];

			DirectX::XMVECTOR	A = DirectX::XMLoadFloat3(&a.position);
			DirectX::XMVECTOR	B = DirectX::XMLoadFloat3(&b.position);
			DirectX::XMVECTOR	C = DirectX::XMLoadFloat3(&c.position);

			DirectX::XMVECTOR	AB = DirectX::XMVectorSubtract(B, A);
			DirectX::XMVECTOR	BC = DirectX::XMVectorSubtract(C, B);
			DirectX::XMVECTOR	CA = DirectX::XMVectorSubtract(A, C);

			//	法線をレイ方向に射影(内積)
			DirectX::XMVECTOR	Normal = DirectX::XMVector3Cross(AB, BC);
			DirectX::XMVECTOR	Dot = DirectX::XMVector3Dot(Normal, Dir);
			float	dot;
			DirectX::XMStoreFloat(&dot, Dot);
			if (dot >= 0)
				continue;

			//	発射位置から3角形の一点を法線に射影
			DirectX::XMVECTOR	V = DirectX::XMVectorSubtract(A, Start);
			//	レイの長さ
			DirectX::XMVECTOR	T = DirectX::XMVectorDivide(DirectX::XMVector3Dot(Normal, V), Dot);
			float	t;
			DirectX::XMStoreFloat(&t, T);
			if (t < 0 || t > neart)
				continue;

			//	交点算出
			DirectX::XMVECTOR	Position = DirectX::XMVectorAdd(DirectX::XMVectorMultiply(Dir, T), Start);

			//	2Dの外積を利用した内点判定
			DirectX::XMFLOAT3	position, ab, bc, ca;
			DirectX::XMStoreFloat3(&position, Position);
			DirectX::XMStoreFloat3(&ab, AB);
			if ((a.position.y - position.y) * ab.z - (a.position.z - position.z) * ab.y > 0)
				continue;

			DirectX::XMStoreFloat3(&bc, BC);
			if ((b.position.y - position.y) * bc.z - (b.position.z - position.z) * bc.y > 0)
				continue;

			DirectX::XMStoreFloat3(&ca, CA);
			if ((c.position.y - position.y) * ca.z - (c.position.z - position.z) * ca.y > 0)
				continue;
			neart = t;
			materialIndex = subset.material_index;
			hitPosition = Position;
			hitNormal = Normal;
		}
	}
	return	materialIndex;
}

//	Y軸専用
int	Collision::IntersectRayVsModelY(const DirectX::XMVECTOR& Start,
	const DirectX::XMVECTOR& Dir,
	float	neart,
	const ModelResource::s_mesh& mesh,
	DirectX::XMVECTOR& hitPosition,
	DirectX::XMVECTOR& hitNormal)
{
	const	std::vector<ModelResource::s_vertex>& vertices = mesh.vertices;
	const	std::vector<int>& indices = mesh.indices;
	const	ModelResource::s_vertex* pVertices = vertices.data();
	const	int* pIndices = indices.data();

	int	materialIndex = -1;
	for (const ModelResource::s_subset& subset : mesh.subsets)
	{
		for (UINT ii = 0; ii < subset.index_count; ii += 3)
		{
			UINT	index = subset.index_start + ii;
			const	ModelResource::s_vertex& a = pVertices[pIndices[index + 0]];
			const	ModelResource::s_vertex& b = pVertices[pIndices[index + 1]];
			const	ModelResource::s_vertex& c = pVertices[pIndices[index + 2]];

			DirectX::XMVECTOR	A = DirectX::XMLoadFloat3(&a.position);
			DirectX::XMVECTOR	B = DirectX::XMLoadFloat3(&b.position);
			DirectX::XMVECTOR	C = DirectX::XMLoadFloat3(&c.position);

			DirectX::XMVECTOR	AB = DirectX::XMVectorSubtract(B, A);
			DirectX::XMVECTOR	BC = DirectX::XMVectorSubtract(C, B);
			DirectX::XMVECTOR	CA = DirectX::XMVectorSubtract(A, C);

			//	法線をレイ方向に射影(内積)
			DirectX::XMVECTOR	Normal = DirectX::XMVector3Cross(AB, BC);
			DirectX::XMVECTOR	Dot = DirectX::XMVector3Dot(Normal, Dir);
			float	dot;
			DirectX::XMStoreFloat(&dot, Dot);
			if (dot >= 0)
				continue;

			//	発射位置から3角形の一点を法線に射影
			DirectX::XMVECTOR	V = DirectX::XMVectorSubtract(A, Start);
			//	レイの長さ
			DirectX::XMVECTOR	T = DirectX::XMVectorDivide(DirectX::XMVector3Dot(Normal, V), Dot);
			float	t;
			DirectX::XMStoreFloat(&t, T);
			if (t < 0 || t > neart)
				continue;

			//	交点算出
			DirectX::XMVECTOR	Position = DirectX::XMVectorAdd(DirectX::XMVectorMultiply(Dir, T), Start);

			//	2Dの外積を利用した内点判定
			DirectX::XMFLOAT3	position, ab, bc, ca;
			DirectX::XMStoreFloat3(&position, Position);
			DirectX::XMStoreFloat3(&ab, AB);
			if ((a.position.x - position.x) * ab.z - (a.position.z - position.z) * ab.x > 0)
				continue;

			DirectX::XMStoreFloat3(&bc, BC);
			if ((b.position.x - position.x) * bc.z - (b.position.z - position.z) * bc.x > 0)
				continue;

			DirectX::XMStoreFloat3(&ca, CA);
			if ((c.position.x - position.x) * ca.z - (c.position.z - position.z) * ca.x > 0)
				continue;

			neart = t;
			materialIndex = subset.material_index;
			hitPosition = Position;
			hitNormal = Normal;
		}
	}
	return	materialIndex;
}

//	Z軸専用
int	Collision::IntersectRayVsModelZ(const DirectX::XMVECTOR& Start,
	const DirectX::XMVECTOR& Dir,
	float	neart,
	const ModelResource::s_mesh& mesh,
	DirectX::XMVECTOR& hitPosition,
	DirectX::XMVECTOR& hitNormal)
{
	const	std::vector<ModelResource::s_vertex>& vertices = mesh.vertices;
	const	std::vector<int>& indices = mesh.indices;
	const	ModelResource::s_vertex* pVertices = vertices.data();
	const	int* pIndices = indices.data();

	int	materialIndex = -1;
	for (const ModelResource::s_subset& subset : mesh.subsets)
	{
		for (UINT ii = 0; ii < subset.index_count; ii += 3)
		{
			UINT	index = subset.index_start + ii;
			const	ModelResource::s_vertex& a = pVertices[pIndices[index + 0]];
			const	ModelResource::s_vertex& b = pVertices[pIndices[index + 1]];
			const	ModelResource::s_vertex& c = pVertices[pIndices[index + 2]];

			DirectX::XMVECTOR	A = DirectX::XMLoadFloat3(&a.position);
			DirectX::XMVECTOR	B = DirectX::XMLoadFloat3(&b.position);
			DirectX::XMVECTOR	C = DirectX::XMLoadFloat3(&c.position);

			DirectX::XMVECTOR	AB = DirectX::XMVectorSubtract(B, A);
			DirectX::XMVECTOR	BC = DirectX::XMVectorSubtract(C, B);
			DirectX::XMVECTOR	CA = DirectX::XMVectorSubtract(A, C);

			//	法線をレイ方向に射影(内積)
			DirectX::XMVECTOR	Normal = DirectX::XMVector3Cross(AB, BC);
			DirectX::XMVECTOR	Dot = DirectX::XMVector3Dot(Normal, Dir);
			float	dot;
			DirectX::XMStoreFloat(&dot, Dot);
			if (dot >= 0)
				continue;

			//	発射位置から3角形の一点を法線に射影
			DirectX::XMVECTOR	V = DirectX::XMVectorSubtract(A, Start);
			//	レイの長さ
			DirectX::XMVECTOR	T = DirectX::XMVectorDivide(DirectX::XMVector3Dot(Normal, V), Dot);
			float	t;
			DirectX::XMStoreFloat(&t, T);
			if (t < 0 || t > neart)
				continue;

			//	交点算出
			DirectX::XMVECTOR	Position = DirectX::XMVectorAdd(DirectX::XMVectorMultiply(Dir, T), Start);

			//	2Dの外積を利用した内点判定
			DirectX::XMFLOAT3	position, ab, bc, ca;
			DirectX::XMStoreFloat3(&position, Position);
			DirectX::XMStoreFloat3(&ab, AB);
			if ((a.position.x - position.x) * ab.y - (a.position.y - position.y) * ab.x > 0)
				continue;

			DirectX::XMStoreFloat3(&bc, BC);
			if ((b.position.x - position.x) * bc.y - (b.position.y - position.y) * bc.x > 0)
				continue;

			DirectX::XMStoreFloat3(&ca, CA);
			if ((c.position.x - position.x) * ca.y - (c.position.y - position.y) * ca.x > 0)
				continue;

			neart = t;
			materialIndex = subset.material_index;
			hitPosition = Position;
			hitNormal = Normal;
		}
	}
	return	materialIndex;
}

//	通常版
int	Collision::IntersectRayVsModel(const DirectX::XMVECTOR& Start,
	const DirectX::XMVECTOR& Dir,
	float	neart,
	const ModelResource::s_mesh& mesh,
	DirectX::XMVECTOR& hitPosition,
	DirectX::XMVECTOR& hitNormal)
{
	const	std::vector<ModelResource::s_vertex>& vertices = mesh.vertices;
	const	std::vector<int>& indices = mesh.indices;
	const	ModelResource::s_vertex* pVertices = vertices.data();
	const	int* pIndices = indices.data();

	int	materialIndex = -1;
	for (const ModelResource::s_subset& subset : mesh.subsets)
	{
		for (UINT ii = 0; ii < subset.index_count; ii += 3)
		{
			UINT	index = subset.index_start + ii;
			const	ModelResource::s_vertex& a = pVertices[pIndices[index + 0]];
			const	ModelResource::s_vertex& b = pVertices[pIndices[index + 1]];
			const	ModelResource::s_vertex& c = pVertices[pIndices[index + 2]];

			DirectX::XMVECTOR	A = DirectX::XMLoadFloat3(&a.position);
			DirectX::XMVECTOR	B = DirectX::XMLoadFloat3(&b.position);
			DirectX::XMVECTOR	C = DirectX::XMLoadFloat3(&c.position);

			DirectX::XMVECTOR	AB = DirectX::XMVectorSubtract(B, A);
			DirectX::XMVECTOR	BC = DirectX::XMVectorSubtract(C, B);
			DirectX::XMVECTOR	CA = DirectX::XMVectorSubtract(A, C);

			DirectX::XMVECTOR	Normal = DirectX::XMVector3Cross(AB, BC);
			DirectX::XMVECTOR	Dot = DirectX::XMVector3Dot(Normal, Dir);
			float	dot;
			DirectX::XMStoreFloat(&dot, Dot);
			if (dot >= 0)
				continue;

			//	
			DirectX::XMVECTOR	V = DirectX::XMVectorSubtract(A, Start);
			DirectX::XMVECTOR	T = DirectX::XMVectorDivide(DirectX::XMVector3Dot(Normal, V), Dot);

			float	t;
			DirectX::XMStoreFloat(&t, T);
			if (t < 0 || t > neart)
				continue;

			DirectX::XMVECTOR	Position = DirectX::XMVectorAdd(DirectX::XMVectorMultiply(Dir, T)
				, Start);

			DirectX::XMVECTOR	V1 = DirectX::XMVectorSubtract(A, Position);
			DirectX::XMVECTOR	Cross1 = DirectX::XMVector3Cross(V1, AB);
			DirectX::XMVECTOR	Dot1 = DirectX::XMVector3Dot(Cross1, Normal);
			float	dot1;
			DirectX::XMStoreFloat(&dot1, Dot1);
			if (dot1 < 0)
				continue;

			DirectX::XMVECTOR	V2 = DirectX::XMVectorSubtract(B, Position);
			DirectX::XMVECTOR	Cross2 = DirectX::XMVector3Cross(V2, BC);
			DirectX::XMVECTOR	Dot2 = DirectX::XMVector3Dot(Cross2, Normal);
			float	dot2;
			DirectX::XMStoreFloat(&dot2, Dot2);
			if (dot2 < 0)
				continue;

			DirectX::XMVECTOR	V3 = DirectX::XMVectorSubtract(C, Position);
			DirectX::XMVECTOR	Cross3 = DirectX::XMVector3Cross(V3, CA);
			DirectX::XMVECTOR	Dot3 = DirectX::XMVector3Dot(Cross3, Normal);
			float	dot3;
			DirectX::XMStoreFloat(&dot3, Dot3);
			if (dot3 < 0)
				continue;

			neart = t;
			materialIndex = subset.material_index;
			hitPosition = Position;
			hitNormal = Normal;
		}
	}
	return	materialIndex;
}


void atack_manager::update()
{
	



	for (auto obj = attklist.begin(); obj != attklist.end(); ++obj)
	{
		obj->get()->attack_standby_frame--;//攻撃発生まで待機

		if (obj->get()->attack_standby_frame < 0)
		{
			if (!obj->get()->trigger)continue;
	
			if (std::shared_ptr<OBJ3D> owner =  obj->get()->owner.lock())
			{

				float dist = 0.0f;

				if (owner->tag == "PLAYER1")
				{
					dist = judge_manager::Instance().get_1pdistance();
				}

				else if (owner->tag == "PLAYER2")
				{
					dist = judge_manager::Instance().get_2pdistance();
				}



				obj->get()->_attack_range.top = owner->screen_position.y + obj->get()->a_top;
				obj->get()->_attack_range.right = owner->screen_position.x + obj->get()->a_right;
				obj->get()->_attack_range.left = owner->screen_position.x + obj->get()->a_left;
				obj->get()->_attack_range.bottom = owner->screen_position.y + obj->get()->a_bottom;


			    if (obj->get()->air && owner->isGround || owner->ishit)
				{
					obj->get()->trigger = false;
				}

			}
			

			
			obj->get()->attack_life_time--;//攻撃が発生

			if (obj->get()->attack_life_time < 0)//攻撃が終了
			{
				obj->get()->trigger = false;

			}

		}


	}


	if (attklist.size()>20)
	{
		attklist.pop_front();

	}
}

//std::shared_ptr<attack_range> atack_manager::add_attack_range(OBJ3D* owner,float left, float top, float right, float bottom, int attack_life_time, int _owner_tag,
//	unsigned int _attack_types, int hitcount,VECTOR2 vector, int standbyframe, int damage)
//{
//	std::shared_ptr<attack_range> obj = std::make_shared<attack_range>();
//
//	obj->attack_life_time = attack_life_time;
//
//	obj->a_left = left;
//	obj->a_top = top;
//	obj->a_right = right;
//	obj->a_bottom = bottom;
//
//	obj->attack_types = _attack_types;
//	obj->attack_standby_frame = standbyframe;
//
//	obj->owner_tag = _owner_tag;
//
//
//	obj->hitcount = hitcount;
//	obj->vector = vector;
//	obj->damage = damage;
//
//	obj->trigger = true;
//
//	obj->owner = owner->shared_from_this();
//
//	attklist.emplace_back(obj);
//
//
//
//	return *attklist.rbegin();
//}



std::shared_ptr<attack_range> atack_manager::add_attack_range(OBJ3D* owner, attack_range* atkinfo)
{
	std::shared_ptr<attack_range> obj = std::make_shared<attack_range>();

	float  delta_time = framework::Instance().elapased_time;



	float dist = 0.0f;

	if (owner->tag == "PLAYER1")
	{
		dist = judge_manager::Instance().get_1pdistance();
	}

	else if (owner->tag == "PLAYER2")
	{
		dist = judge_manager::Instance().get_2pdistance();
	}




	obj->attack_life_time = atkinfo->attack_life_time * delta_time;

	if (dist >= 0)
	{
		obj->a_left = atkinfo->a_left;
		obj->a_top = atkinfo->a_top;
		obj->a_right =  atkinfo->a_right;
		obj->a_bottom = atkinfo->a_bottom;
	}

	else
	{
		obj->a_left =  -atkinfo->a_right;
		obj->a_top = atkinfo->a_top;
		obj->a_right = -atkinfo->a_left;
		obj->a_bottom = atkinfo->a_bottom;
	}

	obj->attack_types =1 << atkinfo->attack_types;
	obj->attack_standby_frame = atkinfo->attack_standby_frame * delta_time;

	obj->owner_tag = owner->player_num;


	obj->hitcount = atkinfo->hitcount;
	obj->vector.x = atkinfo->vector.x * dist ;
	obj->vector.y = atkinfo->vector.y ;
	obj->damage = atkinfo->damage;

	obj->owner = owner->shared_from_this();

	obj->trigger = true;
	obj->air = atkinfo->air;
	obj->attenuation_rate = atkinfo->attenuation_rate;
	obj->stanpoint = atkinfo->stanpoint;
	obj->hitstop = atkinfo->hitstop * delta_time;


	attklist.emplace_back(obj);



	return *attklist.rbegin();
}

