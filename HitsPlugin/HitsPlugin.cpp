#include "HitsPlugin.h"
BAKKESMOD_PLUGIN(HitsPlugin, "HitsPlugin ", "0.1", PLUGINTYPE_FREEPLAY);

/*
 *	Written by Caleb Stamper 2019
 *	MIT License
 *
 */

HitsPlugin::HitsPlugin()
{

}

HitsPlugin::~HitsPlugin()
{
}

void HitsPlugin::onLoad()
{
	ServerWrapper server_wrapper = gameWrapper->GetGameEventAsServer();

	hitsShow = make_shared<bool>(true);
	cvarManager->registerCvar("hitsplugin_hits_show", "1", "Show Hits", true, true, 0, true, 1).bindTo(hitsShow);
	cvarManager->getCvar("hitsplugin_hits_show").addOnValueChanged(std::bind(&HitsPlugin::OnHitboxOnValueChanged, this, std::placeholders::_1, std::placeholders::_2));
	//ss


	gameWrapper->HookEvent("Function TAGame.GameEvent_Tutorial_TA.OnInit", bind(&HitsPlugin::OnFreeplayLoad, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Tutorial_TA.Destroyed", bind(&HitsPlugin::OnFreeplayDestroy, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_TrainingEditor_TA.StartPlayTest", bind(&HitsPlugin::OnFreeplayLoad, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_TrainingEditor_TA.Destroyed", bind(&HitsPlugin::OnFreeplayDestroy, this, std::placeholders::_1));
	gameWrapper->HookEventWithCaller<CarWrapper>("Function TAGame.Car_TA.OnHitBall",
		std::bind(&HitsPlugin::onHitBallWithCaller,
			this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	// TutorialWrapper.GetSecondsElapsed() - best way of calculating time passing because it’s tied to physics tick
	//Cvars
	// hits - describes available functions

	// hits show - turns on the plugin

	// hits hide - turns off the plugin

	// hits set 2 - limit the amount of hits to show to 2 (min 1, max 1000)

	// hits dim on - scales the alpha of hits based on how long they've been showing

	// hits color - tells the current color

	// hits color on - changes color based on how far back in the list they are

	// hits color red - changes the color that is used for hits to red
	cvarManager->executeCommand("hitsplugin_hits_show 1");
}

void HitsPlugin::onUnload()
{
	cvarManager->backupCfg("./bakkesmod/cfg/config.cfg");
	gameWrapper->UnhookEvent("Function TAGame.Car_TA.OnHitBall");
}

void HitsPlugin::OnFreeplayLoad(std::string eventName)
{
	if (*hitsShow) {
		//clear the hits array
		for (int i = 0; i < hitsMax; i++)
		{
			hits[i] = { Vector{0,0,0} };
		}
		hitsIndex = 0;
		gameWrapper->RegisterDrawable(std::bind(&HitsPlugin::Render, this, std::placeholders::_1));
		cvarManager->log("HitsPlugin started drawing");
	}
}

void HitsPlugin::OnFreeplayDestroy(std::string eventName)
{
	gameWrapper->UnregisterDrawables();
	cvarManager->log("HitsPlugin stopped drawing");
}

void HitsPlugin::OnHitboxOnValueChanged(std::string oldValue, CVarWrapper cvar)
{
	if (cvar.getBoolValue() && gameWrapper->IsInGame()) {
		OnFreeplayLoad("Load");
	}
	else
	{
		OnFreeplayDestroy("Destroy");
	}
}

void HitsPlugin::Render(CanvasWrapper canvas)
{
	if (*hitsShow && gameWrapper->IsInGame())
	{
		ServerWrapper game = gameWrapper->GetGameEventAsServer();

		if (game.IsNull())
			return;
		ArrayWrapper<CarWrapper> cars = game.GetCars();

		if (cars.Count() > 0) {
			CarWrapper car = cars.Get(0);
			if (car.IsNull())
				return;
			Vector carLocation = car.GetLocation();
			Rotator carRotation = car.GetRotation();


			// Setup values for my code
			canvas.SetColor(255, 0, 255, 200);

			// LOOP THROUGH downward AND DRAW MY HITS
			int drawIndex = hitsIndex;
			bool stillDrawing = true;
			Vector2 hitIn2d;
			while (stillDrawing)
			{
				drawIndex -= 1;
				if (drawIndex < 0)
					drawIndex = hitsMax;
				if (drawIndex == hitsIndex)
					stillDrawing = false;

				// if this is actually a hit and not null
				if (hits[drawIndex].X != 0 && hits[drawIndex].Y != 0 && hits[drawIndex].Z != 0)
				{
					// set canvas color to purple
					canvas.SetColor(255, 0, 255, 200);

					// get 2d projection of the hitvector, rotated and translated to its proper location relative to car
					hitIn2d = canvas.Project(getRecalledVectorForCurrentCar(carLocation, carRotation, hits[drawIndex]));
					canvas.SetPosition(hitIn2d);

					if (drawIndex == hitsIndex)
					{
						//set canvas color to red
						canvas.SetColor(255, 0, 0, 200);
					}
					// draw the hit
					canvas.FillBox(Vector2{ 15, 15 });
				}
			}
			// Draw a line from the car to the ball
			// do this to simulate roughly what information get stored into hits but see it live
			BallWrapper ball = game.GetGameBalls().Get(0);
			if (!ball.IsNull()) {
				//Vector vectorFromCarToBall = getVectorToStore(carLocation, carRotation, ball.GetLocation() - carLocation);

				//canvas.DrawLine(canvas.Project(carLocation), canvas.Project(MyRotate(vectorFromCarToBall, dRoll, -dYaw, dPitch) + carLocation));
			}
		}
	}
}

Vector HitsPlugin::getVectorToStore(Vector carLocation, Rotator carRotation, Vector hitLocation)
{
	return myGoodRotate(hitLocation - carLocation, carRotation);
}

Vector HitsPlugin::getRecalledVectorForCurrentCar(Vector carLocation, Rotator carRotation, Vector recalledVector)
{
	Vector result;
	double dPitch = (double)carRotation.Pitch / 32764.0 * 3.14159;
	double dYaw = (double)carRotation.Yaw / 32764.0 * 3.14159;
	double dRoll = (double)carRotation.Roll / 32764.0 * 3.14159;
	result = MyRotate(recalledVector, dRoll, -dYaw, dPitch) + carLocation;
	return result;
}
void HitsPlugin::onHitBallWithCaller(CarWrapper caller, void* params, std::string eventName)
{
	/*
	 * We have a new collision, so prepare the array index
	 * always add one before new hit is recorded
	 */
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

		HitBallParams hbp = *(HitBallParams*)params;
		Vector carLocation = car.GetLocation();
		Rotator carRotation = car.GetRotation();
		Vector hitLocation = hbp.Location;

		//What I'm testing
		//hits[hitsIndex] = getVectorToStore(carLocation, carRotation, hitLocation);

		//What Works (while the car is flat on the ground)
		double dPitch = (double)carRotation.Pitch / 32764.0 * 3.14159;
		double dYaw = (double)carRotation.Yaw / 32764.0 * 3.14159;
		double dRoll = (double)carRotation.Roll / 32764.0 * 3.14159;
		hits[hitsIndex] = MyRotate(hitLocation - carLocation, -dRoll, dYaw, -dPitch);



	}

}

Vector HitsPlugin::myGoodRotate(Vector vectorToRotate, Rotator rotation)
{
	double dPitch = (double)rotation.Pitch / 16340.0 * 3.14159;
	double dYaw = (double)rotation.Yaw / 32764.0 * 3.14159;
	double dRoll = (double)rotation.Roll / 32764.0 * 3.14159;

	Vector result = vectorToRotate;

	float sx = sin(dRoll);
	float cx = cos(dRoll);
	float sy = sin(dPitch);
	float cy = cos(dPitch);
	float sz = sin(dYaw);
	float cz = cos(dYaw);

	// rotate around X = ROLL
	result = Vector(
		result.X,
		result.Y * cx - result.Z * sx,
		result.Y * sx + result.Z * cx
	);
	// rotate around Y = PITCH
	result = Vector(
		result.X * cy + result.Z * sy,
		result.Y,
		-result.X * sy + result.Z * cy
	);
	// rotate around Z = YAW
	result = Vector(
		result.X * cz - result.Y * sz,
		result.X * sz + result.Y * cz,
		result.Z
	);

	return result;
}

Vector HitsPlugin::MyRotate(Vector aVec, double roll, double yaw, double pitch)
{

	float tmp = aVec.Z;
	aVec.Z = aVec.Y;
	aVec.Y = tmp;

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




