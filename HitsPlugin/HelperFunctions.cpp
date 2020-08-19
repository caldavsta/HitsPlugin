#define _USE_MATH_DEFINES
#include "HelperFunctions.h"

Vector sp::quatToFwd(Quat quat)
{
	// Vector fwd = Vector(1 - 2 * (quat.X*quat.X + quat.Y*quat.Y), 2 * (quat.Y*quat.Z + quat.X*quat.W), -1 * (2 * (quat.X*quat.Z - quat.Y*quat.W)));

	Vector fwd = rotateVectorWithQuat(Vector(1, 0, 0), quat);
	fwd.normalize();
	return fwd;
}

Vector sp::quatToRight(Quat quat)
{
	// Vector right = Vector(2 * (quat.Y*quat.Z - quat.X*quat.W), 1 - 2 * (quat.X*quat.X + quat.Z*quat.Z), -1 * (2 * (quat.X*quat.Y + quat.Z*quat.W)));

	Vector right = rotateVectorWithQuat(Vector(0, 1, 0), quat);
	right.normalize();
	return right;
}

Vector sp::quatToUp(Quat quat)
{
	// Vector up = Vector(2 * (quat.X*quat.Z + quat.Y*quat.W), 2 * (quat.X*quat.Y - quat.Z*quat.W), -1 * (1 - 2 * (quat.Y*quat.Y + quat.Z*quat.Z)));

	Vector up = rotateVectorWithQuat(Vector(0, 0, 1), quat);
	up.normalize();
	return up;
}

Vector sp::rotateVectorWithQuat(Vector v, Quat q)
{
	Quat p;
	p.W = 0;
	p.X = v.X;
	p.Y = v.Y;
	p.Z = v.Z;

	Quat result = (q * p) * q.conjugate();
	return Vector(result.X, result.Y, result.Z);
}

Rotator sp::quatToRot(Quat q)
{
	Vector fwd = quatToFwd(q);
	Vector up = quatToUp(q);
	Vector right = quatToRight(q);


	// pitch

	float pitch_f = asin(fwd.Z);
	int pitch = (pitch_f / (M_PI / 2)) * 16384;



	// roll

	Vector vert = Vector(0, 0, 1);
	if (up.Z < 0)
	{
		vert = Vector(0, 0, -1);
	}
	Vector hor_right = Vector::cross(fwd, vert);
	hor_right = { -hor_right.X, -hor_right.Y, -hor_right.Z }; // left-handed coordinate system
	hor_right.normalize();
	float roll_cos = Vector::dot(hor_right, right);
	float roll_f = acos(roll_cos);

	float up_f = asin(up.Z);

	if (right.Z >= 0)
	{
		if (up.Z >= 0)
		{
			roll_f = -roll_f;
		}
		else
		{
			roll_f = -M_PI + roll_f;
		}
	}
	else
	{
		if (up.Z >= 0)
		{
			roll_f = roll_f;
		}
		else
		{
			roll_f = M_PI - roll_f;
		}
	}

	int roll = (roll_f / M_PI) * 32768;



	// yaw

	float hor_mag = sqrt(fwd.X * fwd.X + fwd.Y * fwd.Y);
	float hor_y = fwd.Y / hor_mag;
	float fwd_y = asin(hor_y);
	if (fwd_y >= 0)
	{
		if (fwd.X >= 0)
		{
			fwd_y = fwd_y;
		}
		else
		{
			fwd_y = M_PI - fwd_y;
		}
	}
	else
	{
		if (fwd.X >= 0)
		{
			fwd_y = fwd_y;
		}
		else
		{
			fwd_y = -M_PI - fwd_y;
		}
	}

	int yaw = (fwd_y / M_PI) * 32768;

	return Rotator(pitch, yaw, roll);
}

Quat sp::rotToQuat(Rotator rot)
{
	if (rot.Roll > 16384)
	{
		rot.Roll = -32768 - (32768 - rot.Roll);
	}
	if (rot.Yaw < -16384)
	{
		rot.Yaw = 32768 + (32768 + rot.Yaw);
	}

	float DegToRadDiv2 = (M_PI / 180) / 2;
	float uRotToDeg = 182.044449;
	float sinPitch = sin((rot.Pitch / uRotToDeg) * DegToRadDiv2);
	float cosPitch = cos((rot.Pitch / uRotToDeg) * DegToRadDiv2);
	float sinRoll = sin((rot.Roll / uRotToDeg) * DegToRadDiv2);
	float cosRoll = cos((rot.Roll / uRotToDeg) * DegToRadDiv2);
	float sinYaw = sin((rot.Yaw / uRotToDeg) * DegToRadDiv2);
	float cosYaw = cos((rot.Yaw / uRotToDeg) * DegToRadDiv2);

	Quat quat;
	quat.X = (cosRoll * sinPitch * sinYaw) - (sinRoll * cosPitch * cosYaw);
	quat.Y = ((-cosRoll) * sinPitch * cosYaw) - (sinRoll * cosPitch * sinYaw);
	quat.Z = (cosRoll * cosPitch * sinYaw) - (sinRoll * sinPitch * cosYaw);
	quat.W = (cosRoll * cosPitch * cosYaw) + (sinRoll * sinPitch * sinYaw);

	return quat;
}