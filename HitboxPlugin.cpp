#define LINMATH_H //Conflicts with linmath.h if we done declare this here

#include "HitboxPlugin.h"
#include "Hitbox.h"
#include "CarManager.h"
#include "bakkesmod/wrappers/GameEvent/ServerWrapper.h"
#include "bakkesmod/wrappers/GameObject/BallWrapper.h"
#include "bakkesmod/wrappers/GameObject/CarWrapper.h"
#include "bakkesmod\wrappers\GameEvent\TutorialWrapper.h"
#include "bakkesmod/wrappers/arraywrapper.h"
#include <sstream>

BAKKESMOD_PLUGIN(HitboxPlugin, "Test hitbox plugin", "1.0", PLUGINTYPE_FREEPLAY | PLUGINTYPE_CUSTOM_TRAINING)

HitboxPlugin::HitboxPlugin()
{

}

HitboxPlugin::~HitboxPlugin()
{
}

void HitboxPlugin::onLoad()
{
	hitboxOn = make_shared<bool>(true);
	cvarManager->registerCvar("cl_soccar_showhitbox", "1", "Show Hitbox", true, true, 0, true, 1).bindTo(hitboxOn);
	cvarManager->getCvar("cl_soccar_showhitbox").addOnValueChanged(std::bind(&HitboxPlugin::OnHitboxOnValueChanged, this, std::placeholders::_1, std::placeholders::_2));

	hitboxType = make_shared<int>(0);
	cvarManager->registerCvar("cl_soccar_sethitboxtype", "0", "Set Hitbox Car Type", true, true, 0, true, 32767, false).bindTo(hitboxType);
	cvarManager->getCvar("cl_soccar_sethitboxtype").addOnValueChanged(std::bind(&HitboxPlugin::OnHitboxTypeChanged, this, std::placeholders::_1, std::placeholders::_2));


	gameWrapper->HookEvent("Function TAGame.GameEvent_Tutorial_TA.OnInit", bind(&HitboxPlugin::OnFreeplayLoad, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Tutorial_TA.Destroyed", bind(&HitboxPlugin::OnFreeplayDestroy, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_TrainingEditor_TA.StartPlayTest", bind(&HitboxPlugin::OnFreeplayLoad, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_TrainingEditor_TA.Destroyed", bind(&HitboxPlugin::OnFreeplayDestroy, this, std::placeholders::_1));

	cvarManager->registerNotifier("cl_soccar_listhitboxtypes", [this](vector<string> params) {
		cvarManager->log(CarManager::getHelpText());
	}, "List all hitbox integer types, use these values as parameters for cl_soccar_sethitboxtype", PERMISSION_ALL);
	
	

	caleb_value1 = make_shared<float>(0);
	cvarManager->registerCvar("caleb_value1", "0").bindTo(caleb_value1);
	caleb_value2 = make_shared<float>(0);
	cvarManager->registerCvar("caleb_value2", "0").bindTo(caleb_value2);
	caleb_value3 = make_shared<float>(0);
	cvarManager->registerCvar("caleb_value3", "0").bindTo(caleb_value3);

	
	gameWrapper->HookEventWithCaller<CarWrapper>("Function TAGame.Car_TA.OnHitBall",
		std::bind(&HitboxPlugin::onHitBallWithCaller,
			this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));


	gameWrapper->HookEvent("Function TAGame.Car_TA.SetVehicleInput", bind(&HitboxPlugin::OnVehicleInput, this));
	cvarManager->executeCommand("cl_settings_refreshplugins");

	cvarManager->executeCommand("cl_soccar_showhitbox 1");
}

void HitboxPlugin::OnFreeplayLoad(std::string eventName)
{
	// get the 8 hitbox points for current car type
	hitbox.clear();  // we'll reinitialize this in Render, for the first few ticks of free play, the car is null
	cvarManager->log(std::string("OnFreeplayLoad") + eventName);
	if (  *hitboxOn ) {
		gameWrapper->RegisterDrawable(std::bind(&HitboxPlugin::Render, this, std::placeholders::_1));
	}	
}

void HitboxPlugin::OnFreeplayDestroy(std::string eventName)
{
	gameWrapper->UnregisterDrawables();
}



void HitboxPlugin::OnHitboxOnValueChanged(std::string oldValue, CVarWrapper cvar)
{
	if (cvar.getBoolValue() && gameWrapper->IsInGame()) {
		OnFreeplayLoad("Load");
	}
	else
	{
		OnFreeplayDestroy("Destroy");
	}
}

void HitboxPlugin::OnHitboxTypeChanged(std::string oldValue, CVarWrapper cvar) {
	hitbox = CarManager::getHitboxPoints(static_cast<CARBODY>(cvar.getIntValue()), *gameWrapper);
}


#include <iostream>     // std::cout
#include <fstream> 

Vector MyRotate(Vector aVec, double roll, double yaw, double pitch)
{
	float tmp = aVec.Z;
	aVec.Z = aVec.Y;
	aVec.Y = tmp;
	// this rotate is kind of messed up, because UE's xyz coordinates didn't match the axes i expected
   /*
   float sx = sin(pitch);
   float cx = cos(pitch);
   float sy = sin(yaw);
   float cy = cos(yaw);
   float sz = sin(roll);
   float cz = cos(roll);
   */
	float sx = sin(roll);
	float cx = cos(roll);
	float sy = sin(yaw);
	float cy = cos(yaw);
	float sz = sin(pitch);
	float cz = cos(pitch);

	aVec = Vector(aVec.X, aVec.Y * cx - aVec.Z * sx, aVec.Y * sx + aVec.Z * cx);  //2  roll?
	aVec = Vector(aVec.X * cz - aVec.Y * sz, aVec.X * sz + aVec.Y * cz, aVec.Z); //1   pitch?
	aVec = Vector(aVec.X * cy + aVec.Z * sy, aVec.Y, -aVec.X * sy + aVec.Z * cy);  //3  yaw?

	// ugly fix to change coordinates to Unreal's axes
	tmp = aVec.Z;
	aVec.Z = aVec.Y;
	aVec.Y = tmp;
	return aVec;
}

 Vector Rotate(Vector aVec, double roll, double yaw, double pitch)
{

	 // this rotate is kind of messed up, because UE's xyz coordinates didn't match the axes i expected
	/*
	float sx = sin(pitch);
	float cx = cos(pitch);
	float sy = sin(yaw);
	float cy = cos(yaw);
	float sz = sin(roll);
	float cz = cos(roll);
	*/
	float sx = sin(roll);
	float cx = cos(roll);
	float sy = sin(yaw);
	float cy = cos(yaw);
	float sz = sin(pitch);
	float cz = cos(pitch);

	aVec = Vector(aVec.X, aVec.Y * cx - aVec.Z * sx, aVec.Y * sx + aVec.Z * cx);  //2  roll?
	aVec = Vector(aVec.X * cz - aVec.Y * sz, aVec.X * sz + aVec.Y * cz, aVec.Z); //1   pitch?
	aVec = Vector(aVec.X * cy + aVec.Z * sy, aVec.Y, -aVec.X * sy + aVec.Z * cy);  //3  yaw?

	// ugly fix to change coordinates to Unreal's axes
	float tmp = aVec.Z;
	aVec.Z = aVec.Y;
	aVec.Y = tmp;
	return aVec;
}


 void HitboxPlugin::OnVehicleInput()
 {
	/*
	 ServerWrapper game = gameWrapper->GetGameEventAsServer();
	 ArrayWrapper<CarWrapper> cars = game.GetCars();
	 if (cars.Count() > 0) {
		 CarWrapper car = cars.Get(0);
		 if (car.IsNull())
			 return;
		 BallWrapper ball = gameWrapper->GetGameEventAsServer().GetBall();
		 if (ball.IsNull())
			 return;
		 auto ballstate = ball.GetRBState();
		 Vector v = car.GetLocation();
		 Rotator r = car.GetRotation();
		 Vector b = ballstate.Location;

		 double dPitch = (double)r.Pitch / 32764.0 * 3.14159;
		 double dYaw = (double)r.Yaw / 32764.0 * 3.14159;
		 double dRoll = (double)r.Roll / 32764.0 * 3.14159;

		 Vector ballDistance = Rotate(v-b, dRoll, -dYaw, dPitch);
		 cvarManager->log(std::to_string(ballDistance.X) + " " +
			 std::to_string(ballDistance.Y) + " " +
			 std::to_string(ballDistance.Z));
	 }
		 */
 }

void HitboxPlugin::Render(CanvasWrapper canvas)
{

	if (*hitboxOn && gameWrapper->IsInGame())
	{
		ServerWrapper game = gameWrapper->GetGameEventAsServer();

		if (game.IsNull())
			return;
		ArrayWrapper<CarWrapper> cars = game.GetCars();

		if (cars.Count() > 0) {
			CarWrapper car = cars.Get(0);
			if (car.IsNull())
				return;
			if (hitbox.size() == 0) { // initialize hitbox 
				hitbox = CarManager::getHitboxPoints(static_cast<CARBODY>(*hitboxType), *gameWrapper);
			}
			canvas.SetColor(255, 255, 0, 200);

			Vector v = car.GetLocation();
			Rotator r = car.GetRotation();

			double dPitch = (double)r.Pitch / 32764.0*3.14159;
			double dYaw = (double)r.Yaw / 32764.0*3.14159;
			double dRoll = (double)r.Roll / 32764.0*3.14159;

			Vector2 carLocation2D = canvas.Project(v);
			Vector2 hitbox2D[8];
			for (int i = 0; i < 8; i++) {
				hitbox2D[i] = canvas.Project(Rotate(hitbox[i], dRoll, -dYaw, dPitch) + v);
			}


			//Setup values for my code
			canvas.SetColor(255, 0, 255, 200);
			double rollModifier = *caleb_value1;
			double pitchModifier = *caleb_value2;
			double yawModifier = *caleb_value3;

			//LOOP THROUGH downward AND DRAW MY HITS
			Vector2 hits2D[20];
			const int colorModifier = 255 / hitsMax;
			int colorValue = 255;
			int drawIndex = hitsIndex;
			bool stillDrawing = true;
			while (stillDrawing)
			{
				drawIndex -= 1;
				if (drawIndex < 0)
					drawIndex = hitsMax;
				if (drawIndex == hitsIndex)
					stillDrawing = false;

				if (hits[drawIndex].X != 0 && hits[drawIndex].Y != 0 && hits[drawIndex].Z != 0)
				{
					hits2D[drawIndex] = canvas.Project(MyRotate(hits[drawIndex], dRoll, -dYaw, dPitch) + v);
					canvas.SetPosition(hits2D[drawIndex]);

					if (drawIndex == hitsIndex)
					{
						canvas.SetColor(255, 0, 0, 200);
					} else
					{
						canvas.SetColor(255, 0, 255, 200);
					}
					canvas.FillBox({ 15, 15 });
				}
			}

			//Draw a line from the car to the ball
			// do this to simulate what information get stored into hits but see it live
			BallWrapper ball = game.GetGameBalls().Get(0);
			if (!ball.IsNull()) {
				//make the version that gets put INTO hits
				Vector vectorToDraw = ball.GetLocation() - car.GetLocation();
				vectorToDraw = MyRotate(vectorToDraw, dRoll, dYaw, dPitch);
				//now do the processes that happen when you pull OUT of hits
				vectorToDraw = MyRotate(vectorToDraw, dRoll, -dYaw, dPitch) + v;
				canvas.DrawLine(carLocation2D, canvas.Project(vectorToDraw));
			}

			//Draw the hitbox around the car
			canvas.SetColor(0, 255, 0, 200);
			canvas.DrawLine(hitbox2D[0], hitbox2D[1]);
			canvas.DrawLine(hitbox2D[1], hitbox2D[2]);
			canvas.DrawLine(hitbox2D[2], hitbox2D[3]);
			canvas.DrawLine(hitbox2D[3], hitbox2D[0]);
			canvas.DrawLine(hitbox2D[4], hitbox2D[5]);
			canvas.DrawLine(hitbox2D[5], hitbox2D[6]);
			canvas.DrawLine(hitbox2D[6], hitbox2D[7]);
			canvas.DrawLine(hitbox2D[7], hitbox2D[4]);
			canvas.DrawLine(hitbox2D[0], hitbox2D[4]);
			canvas.DrawLine(hitbox2D[1], hitbox2D[5]);
			canvas.DrawLine(hitbox2D[2], hitbox2D[6]);
			canvas.DrawLine(hitbox2D[3], hitbox2D[7]);

			canvas.SetPosition(carLocation2D.minus((Vector2 { 10,10 })));
			canvas.FillBox((Vector2 { 20, 20 }));
			return;
		}
	}
}
void HitboxPlugin::onHitBallWithCaller(CarWrapper caller, void* params, std::string eventName)
{
	hitsIndex++;
	if (hitsIndex > hitsMax)
	{
		hitsIndex = 0;
	}
	ServerWrapper game = gameWrapper->GetGameEventAsServer();
	ArrayWrapper<CarWrapper> cars = game.GetCars();
	if (cars.Count() > 0) {
		CarWrapper car = cars.Get(0);
		if (car.IsNull())
			return;

		BallWrapper ball = gameWrapper->GetGameEventAsServer().GetBall();
		if (ball.IsNull())
			return;
		auto ballstate = ball.GetRBState();

		HitBallParams hbp = *(HitBallParams*)params;
		Vector v = car.GetLocation();
		Vector b = hbp.Location;
		Rotator r = car.GetRotation();

		double dPitch = (double)r.Pitch / 32764.0 * 3.14159;
		double dYaw = (double)r.Yaw / 32764.0 * 3.14159;
		double dRoll = (double)r.Roll / 32764.0 * 3.14159;

		hits[hitsIndex] = MyRotate(b - v, dRoll, dYaw, dPitch);

		cvarManager->log(std::to_string(hits[hitsIndex].X) + " " +
			std::to_string(hits[hitsIndex].Y) + " " +
			std::to_string(hits[hitsIndex].Z));



	}

}
void HitboxPlugin::onUnload()
{

	gameWrapper->UnhookEvent("Function TAGame.Car_TA.OnHitBall");
}

