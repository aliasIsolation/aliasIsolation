#pragma once
#include <DirectXMath.h>
#include <Windows.h>
#include <XInput.h>

using namespace DirectX;

class Mouse
{
public:
	BYTE Pad000[0x14];
	float dX;
	float dY;
};

class CameraState
{
public:
	XMVECTOR m_position;
	XMVECTOR m_rotation;
	float m_speed;
	float m_rotationSpeed;
	float m_rollSpeed;
	float m_boostMultiplier;
	float m_fov;
};

class ControllerState
{
public:
	float dX;
	float dY;
	float dZ;

	float dPitch;
	float dYaw;
	float dRoll;

	float dBoost;
};

class Camera
{
public:
	void Init();
	void CameraHook(void* camera);
	void CameraHook2(void* pCameraCoordinates);
	void CameraHook3();
	void postProcessHook(void* postProcess);
	bool inputDisabled(void* input);
	void rotationHook(void* rotation);
	void Update(double dt);

	void toggleCamera();

	bool* isEnabled() { return &m_enabled; }

	CameraState m_camera;

private:
	bool m_enabled;
	bool findController();

	bool m_inputDisabled;

	void updateController();

	void* pCamera;
	void* pCameraCoordinates;
	void* pPostProcess;
	void* pInput;
	void* pRotation;

	Mouse* m_mouse;

	ControllerState m_controllerState;
	XINPUT_STATE m_state;
	int m_controllerID;
};

