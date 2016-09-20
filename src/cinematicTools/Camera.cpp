#include "Camera.h"
#include "Tools\Log.h"
#include <iostream>
#include <deque>
#include "Offsets.h"

static const double PI = 3.14159265359;
XMVECTOR m_averageMovement[100];
XMFLOAT3 m_averageRotation[100];
float m_averageBoost[100];

void Camera::Init()
{
	m_camera.m_speed = 1;
	m_camera.m_rotationSpeed = 1;
	m_camera.m_rollSpeed = 0;
	m_camera.m_boostMultiplier = 2;
	XMVECTOR empty = XMVectorSet(0, 0, 0, 0);
	for (int i = 0; i < 100; i++)
		m_averageMovement[i] = empty;
}

void Camera::toggleCamera()
{
	if (pCamera == NULL)
		return;
	Log::Write("Camera: " + to_string(!m_enabled));
	if (!m_enabled)
	{
		XMVECTOR* position = (XMVECTOR*)((int)pCamera + 0x1CC);
		m_camera.m_position = XMVectorSet(position->m128_f32[0], position->m128_f32[1], position->m128_f32[2], 0);
	}
	m_enabled = !m_enabled;
	m_inputDisabled = m_enabled;
	bool* inputBool = (bool*)((int)pInput + 0x105C);
	*inputBool = !m_inputDisabled;
}

int offset = 0;
float replaceWith = 1;
bool m_postEnabled = false;
bool m_fov = false;

void Camera::Update(double dt)
{
	if (m_fov)
	{
		float* fov = (float*)((int)pCamera + 0xDC);
		*fov = m_camera.m_fov;
	}
	else if (pCamera)
	{
		float* fov = (float*)((int)pCamera + 0xDC);
		m_camera.m_fov = *fov;
		m_fov = true;
	}

	if (m_enabled)
	{
		updateController();

		if (!m_replayController.isPlaying())
		{
			XMVECTOR averageMoveVector = XMVectorSet(0,0,0,0);
			XMFLOAT3 averageRotVector(0,0,0);
			float averageBoost = 0;
			for (int i = 0; i < 100; i++)
			{
				averageMoveVector = XMVectorAdd(averageMoveVector, m_averageMovement[i]);
				averageRotVector = XMFLOAT3(averageRotVector.x + m_averageRotation[i].x, averageRotVector.y + m_averageRotation[i].y, averageRotVector.z + m_averageRotation[i].z);
				averageBoost += m_averageBoost[i];
			}

			averageMoveVector = XMVectorSet(averageMoveVector.m128_f32[0] / 100, averageMoveVector.m128_f32[1] / 100, averageMoveVector.m128_f32[2] / 100, 0);
			averageRotVector = XMFLOAT3(averageRotVector.x / 100, averageRotVector.y / 100, averageRotVector.z/100);
			averageBoost = averageBoost / 100;

			m_camera.m_rotation.m128_f32[0] -= averageRotVector.x * dt * m_camera.m_rotationSpeed;
			m_camera.m_rotation.m128_f32[1] += averageRotVector.y * dt * m_camera.m_rotationSpeed;
			m_camera.m_rotation.m128_f32[2] += averageRotVector.z * dt * m_camera.m_rollSpeed;
			m_camera.m_rotation = XMVectorClamp(m_camera.m_rotation, XMVectorSet((-(PI/2)+0.001), -100000, -100000, 0), XMVectorSet((PI/2)-0.001, 100000, 100000, 0));

			XMMATRIX* rotation = (XMMATRIX*)((int)pRotation + 0x280);

			XMMATRIX rotationMatrix;
			rotationMatrix.r[0] = XMVectorSet(rotation->r[0].m128_f32[0], rotation->r[1].m128_f32[0], rotation->r[2].m128_f32[0], 0);
			rotationMatrix.r[1] = XMVectorSet(rotation->r[0].m128_f32[1], rotation->r[1].m128_f32[1], rotation->r[2].m128_f32[1], 0);
			rotationMatrix.r[2] = XMVectorSet(rotation->r[0].m128_f32[2], rotation->r[1].m128_f32[2], rotation->r[2].m128_f32[2], 0);
			//XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(0, 0, 0);

			XMVECTOR worldForward = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
			XMVECTOR worldRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
			XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

			XMVECTOR right = DirectX::XMVector3TransformNormal(worldRight, rotationMatrix);
			XMVECTOR forward = DirectX::XMVector3TransformNormal(worldForward, rotationMatrix);
			XMVECTOR up = DirectX::XMVector3TransformNormal(worldUp, rotationMatrix);

			XMVECTOR deltaForward = XMVectorScale(forward, averageMoveVector.m128_f32[2]);
			XMVECTOR deltaRight = XMVectorScale(right, averageMoveVector.m128_f32[0]);
			XMVECTOR deltaUp = XMVectorScale(up, averageMoveVector.m128_f32[1]);
			XMVECTOR delta1 = XMVectorAdd(deltaForward, deltaRight);
			XMVECTOR delta = XMVectorAdd(delta1, deltaUp);
			XMFLOAT4 deltaFloats;
			XMStoreFloat4(&deltaFloats, delta);

			m_camera.m_position = XMVectorSet(m_camera.m_position.m128_f32[0] + deltaFloats.x*m_camera.m_speed*averageBoost*dt,
												m_camera.m_position.m128_f32[1] + deltaFloats.y*m_camera.m_speed*averageBoost*dt,
												m_camera.m_position.m128_f32[2] + deltaFloats.z*m_camera.m_speed*averageBoost*dt, 0);

			m_controllerState.dX = 0;
			m_controllerState.dY = 0;
			m_controllerState.dZ = 0;
			m_controllerState.dPitch = 0;
			m_controllerState.dYaw = 0;
			m_controllerState.dRoll = 0;
		}

		if (m_replayController.isRecording())
		{
			m_replayController.put(static_cast<CameraSnapshot&>(m_camera));
		}
		
		if (m_replayController.isPlaying())
		{
			m_replayController.get(static_cast<CameraSnapshot*>(&m_camera));
			m_replayController.advance();
		}

		if (*(bool*)(Offsets::AI_FreezeGame))
		{
			XMVECTOR* cameraPos = (XMVECTOR*)((int)pCamera + 0x1C);
			cameraPos->m128_f32[0] = m_camera.m_position.m128_f32[0];
			cameraPos->m128_f32[1] = m_camera.m_position.m128_f32[1];
			cameraPos->m128_f32[2] = m_camera.m_position.m128_f32[2];

			XMVECTOR* rotation = (XMVECTOR*)((int)pCamera + 0x1FC);
			rotation->m128_f32[0] = -m_camera.m_rotation.m128_f32[1] / PI;
			rotation->m128_f32[1] = -m_camera.m_rotation.m128_f32[0] / (PI / 2);
			rotation->m128_f32[2] = -m_camera.m_rotation.m128_f32[1] / PI;
			rotation->m128_f32[3] = -m_camera.m_rotation.m128_f32[0] / (PI / 2);

			float* roll = (float*)((int)pCamera + 0x1D8);
			*roll = m_camera.m_rotation.m128_f32[2];

			XMVECTOR* rotationQuat = (XMVECTOR*)((int)pCamera + 0xC);
			XMVECTOR rot = XMQuaternionRotationRollPitchYaw(-m_camera.m_rotation.m128_f32[0], -m_camera.m_rotation.m128_f32[1], m_camera.m_rotation.m128_f32[2]);
			rotationQuat->m128_f32[0] = rot.m128_f32[0];
			rotationQuat->m128_f32[1] = rot.m128_f32[1];
			rotationQuat->m128_f32[2] = rot.m128_f32[2];
			rotationQuat->m128_f32[3] = rot.m128_f32[3];
		}
	}
	else
	{
		// Zero out the controller state if freecam is not active. This is to prevent Menu from using it.
		// The structure is a bit dirty in the way the controller gets exposed from the Camera. It should live
		// externally instead. Then we wouldn't need this hack.
		memset(&m_state, 0, sizeof(m_state));
	}
}

void Camera::CameraHook(void* camera)
{
	pCamera = camera;
	if (m_enabled)
	{
		XMVECTOR* cameraPos = (XMVECTOR*)((int)camera + 0x1C);
		cameraPos->m128_f32[0] = m_camera.m_position.m128_f32[0];
		cameraPos->m128_f32[1] = m_camera.m_position.m128_f32[1];
		cameraPos->m128_f32[2] = m_camera.m_position.m128_f32[2];

		XMVECTOR* rotation = (XMVECTOR*)((int)camera + 0x1FC);
		rotation->m128_f32[0] = -m_camera.m_rotation.m128_f32[1] / PI;
		rotation->m128_f32[1] = -m_camera.m_rotation.m128_f32[0] / (PI / 2);
		rotation->m128_f32[2] = -m_camera.m_rotation.m128_f32[1] / PI;
		rotation->m128_f32[3] = -m_camera.m_rotation.m128_f32[0] / (PI / 2);

		float* roll = (float*)((int)camera + 0x1D8);
		*roll = m_camera.m_rotation.m128_f32[2];

		XMVECTOR* rotationQuat = (XMVECTOR*)((int)pCamera + 0xC);
		XMVECTOR rot = XMQuaternionRotationRollPitchYaw(-m_camera.m_rotation.m128_f32[0], -m_camera.m_rotation.m128_f32[1], m_camera.m_rotation.m128_f32[2]);
		rotationQuat->m128_f32[0] = rot.m128_f32[0];
		rotationQuat->m128_f32[1] = rot.m128_f32[1];
		rotationQuat->m128_f32[2] = rot.m128_f32[2];
		rotationQuat->m128_f32[3] = rot.m128_f32[3];
	}
}

void Camera::CameraHook2(void* coordinates)
{
	pCameraCoordinates = coordinates;
}

void Camera::CameraHook3()
{
	if (m_enabled && pCamera)
	{
		XMVECTOR* cameraPos = (XMVECTOR*)((int)pCamera + 0x1C);
		cameraPos->m128_f32[0] = m_camera.m_position.m128_f32[0];
		cameraPos->m128_f32[1] = m_camera.m_position.m128_f32[1];
		cameraPos->m128_f32[2] = m_camera.m_position.m128_f32[2];

		XMVECTOR* rotationQuat = (XMVECTOR*)((int)pCamera + 0xC);
		XMVECTOR rot = XMQuaternionRotationRollPitchYaw(-m_camera.m_rotation.m128_f32[0], -m_camera.m_rotation.m128_f32[1], m_camera.m_rotation.m128_f32[2]);
		rotationQuat->m128_f32[0] = rot.m128_f32[0];
		rotationQuat->m128_f32[1] = rot.m128_f32[1];
		rotationQuat->m128_f32[2] = rot.m128_f32[2];
		rotationQuat->m128_f32[3] = rot.m128_f32[3];
	}
}

void Camera::postProcessHook(void* postProcess)
{
	pPostProcess = postProcess;
	if (m_postEnabled)
	{

	}
}

void Camera::updateController()
{
	//if (*Offsets::AI_Focus)
	{
		if (findController())
		{
			{
				double LX = m_state.Gamepad.sThumbLX;
				double LY = m_state.Gamepad.sThumbLY;

				//determine how far the controller is pushed
				double magnitude = sqrt(LX*LX + LY*LY);

				//determine the direction the controller is pushed
				double normalizedLX = LX / magnitude;
				double normalizedLY = LY / magnitude;

				double normalizedMagnitude = 0;

				//check if the controller is outside a circular dead zone
				if (magnitude > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
				{
					//clip the magnitude at its expected maximum value
					if (magnitude > 32767) magnitude = 32767;

					//adjust magnitude relative to the end of the dead zone
					magnitude -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

					//optionally normalize the magnitude with respect to its expected range
					//giving a magnitude value of 0.0 to 1.0
					normalizedMagnitude = magnitude / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
				}
				else //if the controller is in the deadzone zero out the magnitude
				{
					magnitude = 0.0;
					normalizedMagnitude = 0.0;
				}
				if (magnitude > 0.0) {
					normalizedMagnitude *= normalizedMagnitude;
					// Determine the direction the controller is pushed
					double normalizedLX = LX / magnitude;
					double normalizedLY = LY / magnitude;
					m_controllerState.dX = normalizedLX * normalizedMagnitude;
					m_controllerState.dZ = -(normalizedLY * normalizedMagnitude);
				}
				else
				{
					m_controllerState.dX = 0.0;
					m_controllerState.dZ = 0.0;
				}
			}

		{
			// Right thumb
			double RX = m_state.Gamepad.sThumbRX;
			double RY = m_state.Gamepad.sThumbRY;

			// Determine how far the controller is pushed
			double magnitude = sqrt(RX*RX + RY*RY);

			double normalizedMagnitude = 0;

			// Check if the controller is outside a circular dead zone
			if (magnitude > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			{
				// Clip the magnitude at its expected maximum value
				if (magnitude > 32767) magnitude = 32767;

				// Adjust magnitude relative to the end of the dead zone
				magnitude -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;

				// Optionally normalize the magnitude with respect to its expected range
				// Giving a magnitude value of 0.0 to 1.0
				normalizedMagnitude = magnitude / (32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
			}
			else {
				// If the controller is in the deadzone zero out the magnitude
				magnitude = 0.0;
				normalizedMagnitude = 0.0;
			}

			if (magnitude > 0.0) {
				normalizedMagnitude *= normalizedMagnitude;
				// Determine the direction the controller is pushed
				double normalizedRX = RX / magnitude;
				double normalizedRY = RY / magnitude;
				m_controllerState.dYaw = -(normalizedRX * normalizedMagnitude);
				m_controllerState.dPitch = -(normalizedRY * normalizedMagnitude);
			}
			else {
				m_controllerState.dYaw = 0.0;
				m_controllerState.dPitch = 0.0;
			}
		}

		{
			// Triggers
			int delta = -(int)(m_state.Gamepad.bLeftTrigger) + (int)(m_state.Gamepad.bRightTrigger);
			if (delta < XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2 && delta >(-XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2)) {
				delta = 0;
			}
			else if (delta < 0) {
				delta += XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2;
			}
			else {
				delta -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2;
			}
			m_controllerState.dY = ((double)delta) / (255.0 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2);
		}

			if ((m_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0)
				m_controllerState.dRoll += 1;
			if ((m_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0)
				m_controllerState.dRoll += -1;

		}

		{
			if (GetAsyncKeyState(0x57) & 0x8000)
				m_controllerState.dZ += -1;
			if (GetAsyncKeyState(0x53) & 0x8000)
				m_controllerState.dZ += 1;
			if (GetAsyncKeyState(0x41) & 0x8000)
				m_controllerState.dX += -1;
			if (GetAsyncKeyState(0x44) & 0x8000)
				m_controllerState.dX += 1;
			if (GetAsyncKeyState(VK_SPACE) & 0x8000)
				m_controllerState.dY += 1;
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
				m_controllerState.dY += -1;
			if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000)
				m_controllerState.dRoll += 1;
			if (GetAsyncKeyState(VK_NUMPAD3) & 0x8000)
				m_controllerState.dRoll -= 1;
			if (*(bool*)(Offsets::AI_FreezeGame))
			{
				if (GetAsyncKeyState(VK_NUMPAD8) & 0x8000)
					m_controllerState.dPitch -= 1;
				if (GetAsyncKeyState(VK_NUMPAD5) & 0x8000)
					m_controllerState.dPitch += 1;
				if (GetAsyncKeyState(VK_NUMPAD4) & 0x8000)
					m_controllerState.dYaw += 1;
				if (GetAsyncKeyState(VK_NUMPAD6) & 0x8000)
					m_controllerState.dYaw -= 1;
			}


			if ((m_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0 || GetAsyncKeyState(VK_LSHIFT) & 0x8000)
				m_controllerState.dBoost = m_camera.m_boostMultiplier;
			else
				m_controllerState.dBoost = 1;

			m_controllerState.dPitch += m_mouse->dY * 80;
			m_controllerState.dYaw += -m_mouse->dX * 80;
		}

	}

	for (int i = 99; i > 0; i -= 1)
	{
		m_averageMovement[i] = m_averageMovement[i - 1];
		m_averageRotation[i] = m_averageRotation[i - 1];
		m_averageBoost[i] = m_averageBoost[i - 1];
	}

	XMVECTOR averageMove = XMVectorSet(m_controllerState.dX, m_controllerState.dY, m_controllerState.dZ, 0);
	m_averageMovement[0] = averageMove;
	m_averageRotation[0] = XMFLOAT3(m_controllerState.dPitch, m_controllerState.dYaw, m_controllerState.dRoll);
	m_averageBoost[0] = m_controllerState.dBoost;
}

bool Camera::findController()
{
	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
	{
		ZeroMemory(&m_state, sizeof(XINPUT_STATE));
		DWORD dwResult = XInputGetState(i, &m_state);
		if (dwResult == ERROR_SUCCESS)
		{
			m_controllerID = i;
			return true;
		}
	}
	return false;
}

bool Camera::inputDisabled(void* input)
{
	pInput = input;
	m_mouse = *(Mouse**)((int)input - 0x48);
	return false;
}

void Camera::rotationHook(void* rotation)
{
	pRotation = rotation;
}

void CameraReplayController::put(const CameraSnapshot& s)
{
	m_snapshots.push_back(s);
}

void CameraReplayController::get(CameraSnapshot *const s) const
{
	if (m_playbackPosition < m_snapshots.size())
	{
		*s = m_snapshots[m_playbackPosition];
	}
}

bool CameraReplayController::advance()
{
	++m_playbackPosition;
	return m_playing = m_playbackPosition < m_snapshots.size();
}

void CameraReplayController::startRecording()
{
	m_snapshots.clear();
	m_playing = false;
	m_recording = true;
}

void CameraReplayController::stopRecording()
{
	m_recording = false;
}

void CameraReplayController::startPlayback()
{
	m_playbackPosition = 0;
	m_playing = true;
	m_recording = false;
}

void CameraReplayController::stopPlayback()
{
	m_playing = false;
}
