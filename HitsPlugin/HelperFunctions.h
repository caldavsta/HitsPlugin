#pragma once
#include <bakkesmod/wrappers/wrapperstructs.h>
#include <vector>

/*
 * These are borrowed from Halfwaydead's Scienceplugin
 */
namespace sp
{
	Vector quatToFwd(Quat quat);
	Vector quatToRight(Quat quat);
	Vector quatToUp(Quat quat);

	Vector rotateVectorWithQuat(Vector v, Quat q);

	Rotator quatToRot(Quat q);
	Quat rotToQuat(Rotator rot);
}

