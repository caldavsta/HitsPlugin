#include "HitsPlugin.h"
BAKKESMOD_PLUGIN(HitsPlugin, "HitsPlugin ", "0.1", PLUGINTYPE_FREEPLAY);

HitsPlugin::HitsPlugin()
{

}

void HitsPlugin::onLoad()
{
	
}

void HitsPlugin::onUnload()
{
	cvarManager->backupCfg("./bakkesmod/cfg/config.cfg");
}