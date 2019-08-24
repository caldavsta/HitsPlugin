#include "HitsPlugin.h"
#include "HelperFunctions.h"
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

void HitsPlugin::listCvars()
{
	cvarManager->log("hitsplugin_hits_show " + hp::BoolToString(cvarManager->getCvar("hitsplugin_hits_show").getBoolValue()));
	cvarManager->log("hitsplugin_hits_show_always " + hp::BoolToString(cvarManager->getCvar("hitsplugin_hits_show_always").getBoolValue()));
	cvarManager->log("hitsplugin_hits_amount " + std::to_string(cvarManager->getCvar("hitsplugin_hits_show_always").getIntValue()));
}

void HitsPlugin::onLoad()
{
	ServerWrapper server_wrapper = gameWrapper->GetGameEventAsServer();

	hitsShow = make_shared<bool>(true);
	hitsShowAlways = make_shared<bool>(true);
	hitsShowAmount = make_shared<int>(true);
	cvarManager->registerCvar("hitsplugin_hits_show", "1", "Attempt to enable display of hits right now.", false, true, 0, true, 1).bindTo(hitsShow);
	cvarManager->registerCvar("hitsplugin_hits_show_always", "1", "Automatically enable hits whenever possible (freeplay and training)", false, true, 0, true, 1).bindTo(hitsShowAlways);
	cvarManager->registerCvar("hitsplugin_hits_amount", "5", "", false, true, 1, true, HITS_ARRAY_SIZE -1).bindTo(hitsShowAmount);
	cvarManager->registerNotifier("hits_list_hits", [&gw = this->gameWrapper, this](vector<string> commands) {
		if (!gw->IsInFreeplay())
			return;
		// list all debug info about vectors
		int drawIndex = hitsIndex;
		bool stillListing = true;
		cvarManager->log("HitsPlugin hits currently showing:");
		while (stillListing)
		{
			drawIndex -= 1;
			if (drawIndex < 0)
				drawIndex = *hitsShowAmount;
			if (drawIndex == hitsIndex)
				stillListing = false;

			cvarManager->log(std::to_string(hits[drawIndex].hitPowerAmbiguous))
			;
		}
	}, "Shows the car speedometer, ball speedometer, and boost usage meter.", PERMISSION_FREEPLAY | PERMISSION_PAUSEMENU_CLOSED);
	
	cvarManager->registerNotifier("hits", [&gw = this->gameWrapper, this](vector<string> commands) {
		if (!gameWrapper->IsInFreeplay() && !gameWrapper->IsInCustomTraining())
			return;
		if (commands.size() >=2)
		{
			if (commands.at(1)._Equal("off"))
			{
				cvarManager->getCvar("hitsplugin_hits_show").setValue(false);
				cvarManager->log("Hits OFF");
			}
			// hits show
			if (commands.at(1)._Equal("on"))
			{
				if (commands.size() == 2)
				{
					cvarManager->getCvar("hitsplugin_hits_show").setValue(true);
					cvarManager->log("Hits ON");
				}
				else
				{
					// hits show always
					if (commands.at(2)._Equal("always"))
					{
						cvarManager->getCvar("hitsplugin_hits_show_always").setValue(true);
						cvarManager->log("Hits will show automatically.");
					}
					// hits show never
					if (commands.at(2)._Equal("never"))
					{
						cvarManager->getCvar("hitsplugin_hits_show_always").setValue(false);
						cvarManager->log("Hits will not be shown automatically.");
					}
				}
			}
			if (commands.at(1)._Equal("show"))
			{
				if(commands.size() == 2)
				{
					cvarManager->log("Showing " + std::to_string(cvarManager->getCvar("hitsplugin_hits_amount").getIntValue()) + " hits");
				}
				if (commands.size() == 3)
				{
					int newHitsAmount = 1;
					if (hp::str2int(newHitsAmount, commands.at(2).c_str()) == hp::SUCCESS)
					{
						setNumberOfHits(newHitsAmount);
					} else
					{
						cvarManager->log("Number of hits must be a number.");
					}
				}
			}
			// hits hide

		} else
		{
			listCvars();
		}
		
	}, "Shows the car speedometer, ball speedometer, and boost usage meter.", PERMISSION_FREEPLAY | PERMISSION_PAUSEMENU_CLOSED);

	gameWrapper->HookEventWithCaller<CarWrapper>("Function TAGame.Car_TA.OnHitBall",std::bind(&HitsPlugin::onHitBallWithCaller,this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	//Cvars
	// hits - describes available functions
	// hits show - turns on the plugin
	// hits hide - turns off the plugin
	// hits set 2 - limit the amount of hits to show to 2 (min 1, max 1000)
	// hits dim on - scales the alpha of hits based on how long they've been showing
	// hits color - tells the current color
	// hits color on - changes color based on how far back in the list they are
	// hits color red - changes the color that is used for hits to red
	if (*hitsShowAlways)
		cvarManager->getCvar("hitsplugin_hits_show").setValue(true);

	*hitsShowAmount = cvarManager->getCvar("hitsplugin_hits_amount").getIntValue();

	SetupAndStartDrawing();
	listCvars();

	cvarManager->log("HitsPlugin loaded. Will only show the hits in freeplay and custom training! Created by expendis.");
}

void HitsPlugin::setNumberOfHits(int numberOfHits)
{
	int valueToChangeHitsAmountTo = 1;
	if (numberOfHits >= HITS_ARRAY_SIZE)
	{
		cvarManager->log(std::to_string(numberOfHits) + " is too large. Setting to the max, " + std::to_string((HITS_ARRAY_SIZE - 1)));
		valueToChangeHitsAmountTo = HITS_ARRAY_SIZE - 1;
	} else
	{
		if (numberOfHits <= 0)
		{
			cvarManager->log(std::to_string(numberOfHits) + " is too small. Setting to the min, 1");
		} else
		{
			valueToChangeHitsAmountTo = numberOfHits;
		}
	}
	StopDrawing();
	SetupAndStartDrawing();
	*hitsShowAmount = valueToChangeHitsAmountTo;
	//cvarManager->getCvar("hitsplugin_hits_amount").setValue(valueToChangeHitsAmountTo);
	cvarManager->log("Now showing " + std::to_string(valueToChangeHitsAmountTo) + " hits.");
}

void HitsPlugin::onUnload()
{
	StopDrawing();
	cvarManager->backupCfg("./bakkesmod/cfg/config.cfg");
	gameWrapper->UnhookEvent("Function TAGame.Car_TA.OnHitBall");
}

void HitsPlugin::SetupAndStartDrawing()
{
	//clear the hits array
	for (int i = 0; i < HITS_ARRAY_SIZE; i++)
	{
		hits[i] = { Vector{0,0,0} };
	}
	hitsIndex = 0;
	gameWrapper->RegisterDrawable(std::bind(&HitsPlugin::Render, this, std::placeholders::_1));
	cvarManager->log("HitsPlugin drawing enabled.");
}

void HitsPlugin::StopDrawing()
{
	gameWrapper->UnregisterDrawables();
	cvarManager->log("HitsPlugin stopped drawing");
}

void HitsPlugin::Render(CanvasWrapper canvas)
{
	if (!gameWrapper->IsInFreeplay() && !gameWrapper->IsInCustomTraining())
		return;

	if (!*hitsShow)
		return;

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



		// LOOP THROUGH downward AND DRAW MY HITS
		int drawIndex = hitsIndex;
		bool stillDrawing = true;
		while (stillDrawing)
		{
			if (*hitsShowAmount != 1)
				drawIndex -= 1;

			if (drawIndex < 0)
				drawIndex = *hitsShowAmount;
			if (drawIndex == hitsIndex)
				stillDrawing = false;

			Vector thisHitLocation = hits[drawIndex].location;

			// if this is actually a hit and not null
			if (thisHitLocation.X != 0 && thisHitLocation.Y != 0 && thisHitLocation.Z != 0)
			{
				// set canvas color to purple
				Color hitColor = hp::getHitColorForValueInRange(hits[drawIndex].hitPowerAmbiguous, 2500.0f);
				canvas.SetColor(hitColor.r, hitColor.g, hitColor.b, hitColor.a);

				// get 2d projection of the hitvector, rotated and translated to its proper location relative to car
				Vector vectorToDraw = getRecalledVectorForCurrentCar(carLocation, carRotation, thisHitLocation);

				// Project the location onto the canvas
				Vector2 canvasPosition = canvas.Project(vectorToDraw);

				// Set the position of canvas
				canvas.SetPosition(canvasPosition);

				// draw the hit
				if (drawIndex == hitsIndex)
				{
					// This is the most recent hit
					canvas.FillBox(hitRenderSizeMax);
					canvas.SetColor(255, 255, 255, 255);

					canvas.SetPosition(canvasPosition);
					canvas.DrawBox(hitRenderSizeMax);

					// draw the text
					canvas.SetColor(hitColor.r, hitColor.g, hitColor.b, hitColor.a);

					Vector2 textPosition = canvas.Project(
						sp::rotateVectorWithQuat(textOffsetFromCar, sp::rotToQuat(carRotation))
					+ carLocation);
					canvas.SetPosition(
						Vector2{ textPosition.X// + textRenderOffset.X
						,textPosition.Y// + textRenderOffset.Y
						});
					canvas.DrawString(hp::speedAsString(hits[drawIndex].hitPowerAmbiguous, false), textScale, textScale);
					
				} else
				{
					canvas.FillBox(hitRenderSizeMin);
				}
			}
		}
	}
}

Vector HitsPlugin::getVectorToStore(Vector carLocation, Rotator carRotation, Vector hitLocation)
{
	Vector rotatedVector = hitLocation - carLocation;
	rotatedVector = sp::rotateVectorWithQuat(rotatedVector, sp::rotToQuat({ 0,-carRotation.Yaw,0 }));
	rotatedVector = sp::rotateVectorWithQuat(rotatedVector, sp::rotToQuat({ -carRotation.Pitch,0,0 }));
	rotatedVector = sp::rotateVectorWithQuat(rotatedVector, sp::rotToQuat({ 0,0,-carRotation.Roll }));
	return rotatedVector;
}

Vector HitsPlugin::getRecalledVectorForCurrentCar(Vector carLocation, Rotator carRotation, Vector recalledVector)
{
	Vector result = recalledVector;
	result = sp::rotateVectorWithQuat(result, sp::rotToQuat({ 0,0,carRotation.Roll }));
	result = sp::rotateVectorWithQuat(result, sp::rotToQuat({ carRotation.Pitch,0,0 }));
	result = sp::rotateVectorWithQuat(result, sp::rotToQuat({ 0,carRotation.Yaw,0 }));
	return result + carLocation;
}

void HitsPlugin::onHitBallWithCaller(CarWrapper caller, void* params, std::string eventName)
{
	if (!gameWrapper->IsInFreeplay() && !gameWrapper->IsInCustomTraining())
		return;
	/*
	 * We have a new collision, so prepare the array index
	 * always add one before new hit is recorded
	 */
	hitsIndex++;
	if (hitsIndex > *hitsShowAmount)
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


		float forceToInputForThisHit = 0.0f;
		//BallWrapper ball = gameWrapper->GetGameEventAsServer().GetGameBalls().Get(0);
		//if (!ball.IsNull()) {
		//	forceToInputForThisHit = (ball.GetVelocity() - hbp.NormalVelocity).magnitude();
		//}

		forceToInputForThisHit = (hbp.Velocity - hbp.OtherVelocity).magnitude();

		hits[hitsIndex] = {
			getVectorToStore(carLocation, carRotation, hitLocation),
			forceToInputForThisHit };

		//print the coloro utput to console
		Color hitColor = hp::getHitColorForValueInRange(forceToInputForThisHit, 4200.0f);//4128 is the max I've seen

	}
}

/* UNUSED - but I might want to use later */
/*
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
*/



