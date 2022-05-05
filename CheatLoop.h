#pragma once
#include <Windows.h>
#include "Game.h"
#include "Menu.h"
#include "driver.hpp"
#include "process.hpp"
#include "Structs.h"


//Just temporary will fix later
int Width;
int Height;


FTransform GetBoneIndex(DWORD_PTR mesh, int index) {
	DWORD_PTR bonearray = Read<DWORD_PTR>(mesh + 0x4B8);
	if (bonearray == NULL) {
		bonearray = Read<DWORD_PTR>(mesh + 0x4B8 + 0x10);
	}
	return Read<FTransform>(bonearray + (index * 0x30));
}
Vector3 GetBoneWithRotation(DWORD_PTR mesh, int id) {
	FTransform bone = GetBoneIndex(mesh, id);
	FTransform ComponentToWorld = Read<FTransform>(mesh + 0x1C0);
	D3DMATRIX Matrix;
	Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());
	return Vector3(Matrix._41, Matrix._42, Matrix._43);
}
D3DMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0)) {
	float radPitch = (rot.x * float(M_PI) / 180.f);
	float radYaw = (rot.y * float(M_PI) / 180.f);
	float radRoll = (rot.z * float(M_PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	D3DMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}

extern Vector3 CameraEXT(0, 0, 0);
float FovAngle;

Vector3 ProjectWorldToScreen(Vector3 WorldLocation) {
	Vector3 Screenlocation = Vector3(0, 0, 0);
	Vector3 Camera;
	auto chain69 = Read<uintptr_t>(Localplayer + 0xa8);
	uint64_t chain699 = Read<uintptr_t>(chain69 + 8);
	Camera.x = Read<float>(chain699 + 0x7E8);
	Camera.y = Read<float>(Rootcomp + 0x12C);
	float test = asin(Camera.x);
	float degrees = test * (180.0 / M_PI);
	Camera.x = degrees;
	if (Camera.y < 0)
		Camera.y = 360 + Camera.y;
	D3DMATRIX tempMatrix = Matrix(Camera);
	Vector3 vAxisX, vAxisY, vAxisZ;
	vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	uint64_t chain = Read<uint64_t>(Localplayer + 0x70);
	uint64_t chain1 = Read<uint64_t>(chain + 0x98);
	uint64_t chain2 = Read<uint64_t>(chain1 + 0x140);

	Vector3 vDelta = WorldLocation - Read<Vector3>(chain2 + 0x10);
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));
	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;

	float zoom = Read<float>(chain699 + 0x580);
	float FovAngle = 80.0f / (zoom / 1.19f);
	float ScreenCenterX = Width / 2;
	float ScreenCenterY = Height / 2;
	float ScreenCenterZ = Height / 2;
	Screenlocation.x = ScreenCenterX + vTransformed.x * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
	Screenlocation.y = ScreenCenterY - vTransformed.y * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
	Screenlocation.z = ScreenCenterZ - vTransformed.z * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
	return Screenlocation;
}

void FNCache() {

	while (true) {

		Uworld = Read<DWORD_PTR>(process_base + 0xB8C8CA0);
		DWORD_PTR Gameinstance = Read<DWORD_PTR>(Uworld + 0x190);
		DWORD_PTR LocalPlayers = Read<DWORD_PTR>(Gameinstance + 0x38);
		Localplayer = Read<DWORD_PTR>(LocalPlayers);
		PlayerController = Read<DWORD_PTR>(Localplayer +0x30);
		LocalPawn = Read<DWORD_PTR>(PlayerController + 0x260);

		PlayerState = Read<DWORD_PTR>(LocalPawn + 0x240);
		Rootcomp = Read<DWORD_PTR>(LocalPawn + 0x138); 

		if (LocalPawn != 0) {
			localplayerID = Read<int>(LocalPawn + 0x18);
		}

		Persistentlevel = Read<DWORD_PTR>(Uworld + 0x30);
		DWORD ActorCount = Read<DWORD>(Persistentlevel + 0xA0);
		DWORD_PTR AActors = Read<DWORD_PTR>(Persistentlevel + 0x98);

		for (int i = 0; i < ActorCount; i++) {
			uint64_t CurrentActor = Read<uint64_t>(AActors + i * 0x8);

			int curactorid = Read<int>(CurrentActor + 0x18);

			if (curactorid == localplayerID || curactorid == localplayerID + 765) {
				Game::Entity fnlEntity{ };
				fnlEntity.Actor = CurrentActor;
				fnlEntity.mesh = Read<uint64_t>(CurrentActor + 0x288);
				fnlEntity.ID = curactorid;
				templist.push_back(fnlEntity);
			}
		}

		Game::list = templist;
	}
}

void Aimbot() {
	//Got deleted when windows corruped
}

DWORD Menu(LPVOID in)
{
	Sleep(250);
	while (1)
	{
		if (GetAsyncKeyState(VK_RSHIFT) & 1) {
			function.tab = 0;
			function.vistab = 0;
			vis = false;
			function.menu = !function.menu;
		}
	}
}

void ActorLoop() {

	for (unsigned long i = 0; i < Game::entitylist.size(); ++i)
	{

		/* ;
		if (SDK::FortPawn.IsVisible(Objects::pawn.mesh)) {
			VisColor = D3DCOLOR_RGBA(0, 255, 0, 255);
		}
		else {
			VisColor = D3DCOLOR_RGBA(255, 0, 0, 255);
		}*/

		if (settings.chams)
		{
			uintptr_t MyState = Read<uintptr_t>(LocalPawn + 0x240);
			if (!MyState) continue;

			MyTeamIndex = Read<uintptr_t>(MyState + 0xF50);
			if (!MyTeamIndex) continue;

			uintptr_t SquadID = Read<uintptr_t>(MyState + 0x1124);
			if (!SquadID) break;

			uintptr_t EnemyState = Read<uintptr_t>(LocalPawn + 0x240);
			if (!EnemyState) continue;

			Write<uintptr_t>(EnemyState + 0xF50, MyTeamIndex);
			Write<uintptr_t>(EnemyState + 0x1124, SquadID);
		}

		if (settings.playerfly)
		{
			Game::Pawn::FZiplinePawnState ZiplinePawnState = Read<Game::Pawn::FZiplinePawnState>(LocalPawn + 0x18D0);
			ZiplinePawnState.bIsZiplining = true;
			ZiplinePawnState.AuthoritativeValue = 10000.f;

			Write<Game::Pawn::FZiplinePawnState>(LocalPawn + 0x18D0, ZiplinePawnState);
		}

		static ULONG PlayerCamManager;

		if (settings.FOVChanger)
		{
			Write<float>(PlayerCamManager + 0x23c, settings.FOVChangerValue);
		}

		if (settings.JumpADS) {
			Write<bool>(LocalPawn + 0x3F41, true);
		}

		if (settings.RapidFire) {
			float FireTime = 0;
			float FireTimeVerified = 0;
			uintptr_t CurrentWeapon = Read<uintptr_t>(LocalPawn + 0x620);
			if (CurrentWeapon) {
				FireTime = Read<float>(CurrentWeapon + 0x9F4);
				FireTimeVerified = Read<float>(CurrentWeapon + 0x9F8);
				Write<float>(CurrentWeapon + 0x9F4, FireTime + FireTimeVerified - settings.RapidFireValue);
			}
		}

		if (settings.InstaRevive) {
			Write<float>(LocalPawn + 0x3788, settings.ReviveScale);
		};
	}


}

bool initdrv() {

	ProcessId = GetProcessID(_T("FortniteClient-Win64-Shipping.exe"));

	while (!ProcessId) {
		ProcessId = GetProcessID(_T("FortniteClient-Win64-Shipping.exe"));
	}

	while (!process_base) {
		process_base = GetProcessBase();
	}

	return true;
}