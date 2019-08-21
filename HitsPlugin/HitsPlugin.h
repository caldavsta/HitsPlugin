#pragma once
#pragma comment( lib, "bakkesmod.lib" )
#include "bakkesmod\plugin\bakkesmodplugin.h"
/*
 * Params for ball collision
 */
struct HitBallParams
{
	void* ptrs[3];
	Vector Velocity;
	Vector OtherVelocity;
	Vector Location;
	Vector Normal;
	Vector NormalForce;
	Vector FrictionForce;
	Vector NormalVelocity;
	Vector FrictionVelocity;
	int NumCollisions;
	int NumContacts;
};
/*
 * The HitsPlugin class
 */
class HitsPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
private:

	//whether hits are shown in-game
	std::shared_ptr<bool> hitsShow;
	// whether to
	int hitsMax = 10;
	void onHitBallWithCaller(CarWrapper caller, void* params, std::string eventName);
	Vector myGoodRotate(Vector vectorToRotate, Rotator rotation);
	Vector hits[20] = { Vector{0,0,0} };
	int hitsIndex = 0;
public:
	virtual void onLoad();
	void OnFreeplayLoad(std::string eventName);
	void OnFreeplayDestroy(std::string eventName);
	void OnHitboxOnValueChanged(std::string oldValue, CVarWrapper cvar);
	void Render(CanvasWrapper canvas);
	Vector getVectorToStore(Vector carLocation, Rotator carRotation, Vector hitLocation);
	Vector getRecalledVectorForCurrentCar(Vector carLocation, Rotator carRotation, Vector recalledVector);
	Vector MyRotate(Vector aVec, double roll, double yaw, double pitch);
	virtual void onUnload();
	HitsPlugin();

	~HitsPlugin();
};

