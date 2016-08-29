#pragma once

#include "glm/glm.hpp"
#include "common.h"


struct FrameConstants
{
	glm::mat4	currViewMatrix;
	glm::mat4	prevViewProjNoJitter;
	glm::mat4	currViewProjNoJitter;

	glm::dmat4	currInvViewProjNoJitter;

	uint		screenWidth = 0;
	uint		screenHeight = 0;

	uint		taaSampleIdx = 0;
	float		gameTime = 0;
	bool		taaRanThisFrame = false;
};

extern FrameConstants g_frameConstants;
