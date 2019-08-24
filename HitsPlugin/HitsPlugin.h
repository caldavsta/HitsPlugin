#pragma once
#pragma comment( lib, "bakkesmod.lib" )
#include "bakkesmod\plugin\bakkesmodplugin.h"
#include <ios>
#include <iomanip>
#include <sstream>
constexpr auto HITS_ARRAY_SIZE = 20;
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

struct Color { int r, g, b, a; };




namespace hp
{
	inline string ColorToString(Color color)
	{
		return std::to_string(color.r) + " " +
			std::to_string(color.g) + " " +
			std::to_string(color.b) + " " +
			std::to_string(color.a);
	}
	inline const string BoolToString(bool b)
	{
		return b ? "true" : "false";
	}
	inline Color getHitColorForValueInRange(float value, float valueMax)
	{
		// I have tweaked these values for what I feel is a nice color gradient
		float redLow = 60.0f;
		float redHigh = 255.0f;
		float greenLow = 30.0f;
		float greenHigh = 255.0f;
		Color result = { 0,0,0,200 };
		float scale = value / valueMax;
		if (value < valueMax / 2.0f)
		{
			result.r = static_cast<int>(redLow + (redHigh - redLow) * scale);
			result.g = greenHigh;
		}
		else
		{
			result.r = redHigh;
			result.g = static_cast<int>(greenHigh - (greenHigh - greenLow) * scale);
		}
		return result;
	}

	enum STR2INT_ERROR { SUCCESS, OVERFLOWS, UNDERFLOWS, INCONVERTIBLE };

	STR2INT_ERROR str2int(int& i, char const* s, int base = 0)
	{
		char* end;
		long  l;
		errno = 0;
		l = strtol(s, &end, base);
		if ((errno == ERANGE && l == LONG_MAX) || l > INT_MAX) {
			return OVERFLOWS;
		}
		if ((errno == ERANGE && l == LONG_MIN) || l < INT_MIN) {
			return UNDERFLOWS;
		}
		if (*s == '\0' || *end != '\0') {
			return INCONVERTIBLE;
		}
		i = l;
		return SUCCESS;
	}

	float UupsToMph(float uups)
	{
		return uups * 60.0f * 60.0f / 160934.4f;

	}

	float UupsToKph(float uups)
	{

		return uups * 60.0f * 60.0f / 100000.0f;
	}

	std::string speedAsString(float speed, bool metric)
	{
		std::stringstream stream;
		if (metric)
		{
			stream << std::fixed << std::setprecision(0) << UupsToKph(speed);
			std::string s = stream.str();

			return s + " km/h";
		}
		else
		{
			stream << std::fixed << std::setprecision(0) << UupsToMph(speed);
			std::string s = stream.str();

			return s + " mph";
		}
	}


}
struct Hit
{
	Vector location;
	float hitPowerAmbiguous;
};
/*
 * The HitsPlugin class
 */
class HitsPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
private:

	//whether hits are shown in-game
	std::shared_ptr<bool> hitsShow;
	std::shared_ptr<bool> hitsShowAlways;
	std::shared_ptr<int> hitsShowAmount;
	int millisecondsDisappearDelay = 1000;
	Vector2 hitRenderSizeMin = { 10, 10 };
	Vector2 hitRenderSizeMax = { 15, 15 };
	Vector2 textRenderOffset = { 250, 0 };

	const float textScale = 2.0f;
	Vector textOffsetFromCar = { -100,0,0 };
	void onHitBallWithCaller(CarWrapper caller, void* params, std::string eventName);
	//Vector myGoodRotate(Vector vectorToRotate, Rotator rotation);
	Hit hits[HITS_ARRAY_SIZE] = { Hit{{0,0,0}, 0.0f} };
	int hitsIndex = 0;
public:
	virtual void onLoad();
	void setNumberOfHits(int numberOfHits);
	void SetupAndStartDrawing();
	void StopDrawing();
	//void OnFreeplayLoad(std::string eventName);
	//void OnFreeplayDestroy(std::string eventName);
	//void OnHitboxOnValueChanged(std::string oldValue, CVarWrapper cvar);
	void Render(CanvasWrapper canvas);
	Vector getVectorToStore(Vector carLocation, Rotator carRotation, Vector hitLocation);
	Vector getRecalledVectorForCurrentCar(Vector carLocation, Rotator carRotation, Vector recalledVector);
	//Vector MyRotate(Vector aVec, double roll, double yaw, double pitch);
	virtual void onUnload();
	HitsPlugin();

	~HitsPlugin();
	void listCvars();
};

