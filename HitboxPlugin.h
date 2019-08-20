#pragma once
#include <memory>
#pragma comment( lib, "bakkesmod.lib" )
#include "bakkesmod/plugin/bakkesmodplugin.h"

/*
Colors the prediction line can have
*/
struct LineColor
{
	unsigned char r, g, b, a; //rgba can be a value of 0-255
};

/*Predicted point in 3d space*/
struct PredictedPoint
{
	/*Location of the predicted ball*/
	Vector location;
	/*States whether it as its highest point or bounces*/
	bool isApex = false;
	Vector apexLocation = { 0,0,0 };
	Vector velocity;
	Vector angVel;
};
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
class HitboxPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	std::shared_ptr<bool> hitboxOn;
	std::shared_ptr<int> hitboxType;
	std::shared_ptr<float> caleb_value1;
	std::shared_ptr<float> caleb_value2;
	std::shared_ptr<float> caleb_value3;
	LineColor colors[2] = { {0, 255, 0, 240}, {75, 0, 130, 240} };
	std::vector<Vector> hitbox;
	void onHitBallWithCaller(CarWrapper caller, void* params, std::string eventName);
	const int hitsMax = 19;
	Vector hits[20] = { Vector{0,0,0} };
	int hitsIndex = 0;
public:
	HitboxPlugin();
	~HitboxPlugin();
	virtual void onLoad();
	virtual void onUnload();
	
	void OnFreeplayLoad(std::string eventName);
	void OnFreeplayDestroy(std::string eventName);
	void OnVehicleInput();
	void OnHitboxOnValueChanged(std::string oldValue, CVarWrapper cvar);
	void OnHitboxTypeChanged(std::string oldValue, CVarWrapper cvar);
	void Render(CanvasWrapper canvas);
};

// utility function
Vector Rotate(Vector aVec, double roll, double yaw, double pitch);
