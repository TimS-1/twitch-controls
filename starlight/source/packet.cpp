#include "fl/packet.h"
#include "al/LiveActor/LiveActor.h"
#include "al/util.hpp"
#include "game/GameData/GameDataFunction.h"
#include "game/StageScene/StageScene.h"
#include "layouts.hpp"
#include "rs/util.hpp"
#include "sead/math/seadVector.h"
#include "util.h"
#include <string.h>

namespace smo {
u32 OutPacketLog::calcLen()
{
    return strlen(message) + 1;
}

void OutPacketLog::construct(u8* dst)
{
    *dst = type;
    strcpy((char*)dst + 1, message);
}

// u32 OutPacketDemoToggle::calcLen()
// {
//     return 1; //1 byte for bool
// }

// void OutPacketDemoToggle::construct(u8* dst)
// {
//     *dst = toggleState;
// }

void InPacketPing::parse(const u8* data, u32 len)
{
    // Nothing to parse
}

void InPacketPing::on(Server& server)
{
    smo::Layouts& layouts = smo::getLayouts();
    layouts.pingFrames = 0;
    amy::log("Ping");
}

void InPacketEvent::parse(const u8* data, u32 len)
{
    eventID = data[0];
}

void InPacketEvent::on(Server& server)
{
    amy::RedeemInfo::state& ri = amy::getRedeemInfo();

    ri.rejectionID++;
    if (ri.rejectionID >= 10)
        ri.rejectionID = 1;

    if (!amy::getRedeemInfo().isTransition) {
        amy::updateRedeemStatus();
    } else {
        amy::sendPacketStateNotice(true);
        return;
    }

    amy::log("Event Redeem Claimed! EventID: %i - Rejected: %s", eventID, !amy::getRedeemInfo().isRedeemsValid ? "true" : "false");

    StageScene* stageScene = amy::getGlobalStageScene();
    al::PlayerHolder* pHolder = al::getScenePlayerHolder(stageScene);
    PlayerActorHakoniwa* player = al::tryGetPlayerActor(pHolder, 0);
    al::LiveActor* curHack = player->getPlayerHackKeeper()->currentHackActor;

    if (!ri.isRedeemsValid)
        amy::sendPacketStateNotice(true);
    else {
        // Send an update about successful packet
        amy::sendPacketStateNotice(false);

        switch (eventID) {
        case 1: { // PrevScene - See Myself Out
            stageScene->mHolder->returnPrevStage();
            break;
        }
        case 2: { // GravFlip - Change Gravity
            amy::RedeemInfo::gravityState& grav = amy::getGravityState();
            al::setGravity(player, amy::getRandomGravity());
            if (grav.timer <= 0)
                grav.comboLength = 0;
            if (grav.comboLength >= 10)
                grav.comboLength--;
            grav.timer += (grav.addLength - (grav.comboLength * grav.comboDec)) * 60;
            grav.comboLength++;
            break;
        }
        case 3: { // Fling - Up we go
            al::LiveActor* targetActor = player;
            if (player->getPlayerHackKeeper()->getCurrentHackName() != nullptr)
                targetActor = curHack;
            al::setTransY(targetActor, al::getTrans(targetActor)->y + 250.f);
            al::setVelocityZero(targetActor);
            al::addVelocity(targetActor, { 0.f, 80.f, 0.f });
            break;
        }
        case 4: {
            GameDataFunction::disableCapByPlacement(player);
            break;
        }
        case 5: {
            ChangeStageInfo stageInfo(stageScene->mHolder, "start", amy::getRandomHomeStage(), false, -1, ChangeStageInfo::SubScenarioType::UNK);
            stageScene->mHolder->changeNextStage(&stageInfo, 0);
            break;
        }
        case 6: {
            amy::getCoinTickState().speed /= 2;
            amy::getCoinTickState().timer = amy::getCoinTickState().speed;
            break;
        }
        case 7: {
            amy::RedeemInfo::windState& wind = amy::getWindState();
            wind.vect = amy::getRandomGravity();
            if (wind.timer <= 0)
                wind.comboLength = 0;
            if (wind.comboLength >= 10)
                wind.comboLength--;
            wind.timer += (wind.addLength - (wind.comboLength * wind.comboDec)) * 60;
            wind.comboLength++;
            break;
        }
        case 8: {
            amy::RedeemInfo::hotFloorState& hot = amy::getHotFloorState();
            if (hot.timer <= 0)
                hot.comboLength = 0;
            if (hot.comboLength >= 10)
                hot.comboLength--;
            hot.timer += (hot.addLength - (hot.comboLength * hot.comboDec)) * 60;
            hot.comboLength++;
            break;
        }
        case 9: {
            amy::RedeemInfo::stickInverState& stick = amy::getStickInverState();
            if (stick.timer <= 0)
                stick.comboLength = 0;
            if (stick.comboLength >= 10)
                stick.comboLength--;
            stick.timer += (stick.addLength - (stick.comboLength * stick.comboDec)) * 60;
            stick.comboLength++;
            break;
        }
        case 10: {
            amy::RedeemInfo::waterAreaState& water = amy::getWaterAreaState();
            if (water.timer <= 0)
                water.comboLength = 0;
            if (water.comboLength >= 10)
                water.comboLength--;
            water.timer += (water.addLength - (water.comboLength * water.comboDec)) * 60;
            water.comboLength++;
            break;
        }
        case 11: {
            if (rs::isPlayer2D(player)) {
                al::setTransY(player, al::getTrans(player)->y + 2000.f);
                break;
            }
            amy::RedeemInfo::dancePartyState& party = amy::getDancePartyState();
            party.timer += party.addLength * 60;

            if (party.enableFrame == false)
                party.enableFrame = true;

            break;
        }
        case 12: {
            amy::getShineWarpState().isWarp = true;
            break;
        }
        case 13: {
            ChangeStageInfo stageInfo(stageScene->mHolder, "start", "MoonWorldHomeStage", false, -1, ChangeStageInfo::SubScenarioType::UNK);
            stageScene->mHolder->changeNextStage(&stageInfo, 0);
            break;
        }
        default: {
            amy::log("Invalid EventID sent? EventID: %i", eventID);
            break;
        }
        }
    }
}

void InPacketResize::parse(const u8* data, u32 len)
{
    scaleVector = *(sead::Vector3f*)&data[0];
}

void InPacketResize::on(Server& server)
{
    amy::RedeemInfo::state& ri = amy::getRedeemInfo();

    ri.rejectionID++;
    if (ri.rejectionID >= 10)
        ri.rejectionID = 1;

    if (!amy::getRedeemInfo().isTransition) {
        amy::updateRedeemStatus();
    } else {
        amy::sendPacketStateNotice(true);
        return;
    }

    amy::log("Resize Redeem Claimed! Rejected: %s", !amy::getRedeemInfo().isRedeemsValid ? "true" : "false");

    if (!amy::getRedeemInfo().isRedeemsValid)
        amy::sendPacketStateNotice(true);
    else {
        amy::sendPacketStateNotice(false);
        al::PlayerHolder* pHolder = al::getScenePlayerHolder(amy::getGlobalStageScene());
        PlayerActorHakoniwa* player = al::tryGetPlayerActor(pHolder, 0);
        al::LiveActor* curHack = player->getPlayerHackKeeper()->currentHackActor;
        al::setScale(player, scaleVector);

        if (player->getPlayerHackKeeper()->getCurrentHackName() != nullptr) {
            sead::Vector3f* scale = al::getScale(curHack);
            scale->x = scaleVector.x;
            scale->y = scaleVector.y;
            scale->z = scaleVector.z;
        }
    }
}

void InPacketPosRandomize::parse(const u8* data, u32 len)
{
    posVector = *(sead::Vector3f*)&data[0];
}

void InPacketPosRandomize::on(Server& server)
{
    amy::RedeemInfo::state& ri = amy::getRedeemInfo();

    ri.rejectionID++;
    if (ri.rejectionID >= 10)
        ri.rejectionID = 1;

    if (!amy::getRedeemInfo().isTransition) {
        amy::updateRedeemStatus();
    } else {
        amy::sendPacketStateNotice(true);
        return;
    }

    amy::log("PosRandomize Redeem Claimed! Rejected: %s", !amy::getRedeemInfo().isRedeemsValid ? "true" : "false");

    if (!amy::getRedeemInfo().isRedeemsValid)
        amy::sendPacketStateNotice(true);
    else {
        amy::sendPacketStateNotice(false);
        al::PlayerHolder* pHolder = al::getScenePlayerHolder(amy::getGlobalStageScene());
        PlayerActorHakoniwa* player = al::tryGetPlayerActor(pHolder, 0);
        al::LiveActor* curHack = player->getPlayerHackKeeper()->currentHackActor;

        if (rs::isPlayer2D(player)) {
            amy::log("Rejected PosRandomize because of 2D Area");
            return;
        }

        if (player->getPlayerHackKeeper()->getCurrentHackName() != nullptr) {
            sead::Vector3f* trans = al::getTrans(curHack);
            trans->x += posVector.x;
            trans->y += posVector.y;
            trans->z += posVector.z;
        } else {
            player->startDemoPuppetable();
            sead::Vector3f* trans = al::getTrans(player);
            trans->x += posVector.x;
            trans->y += posVector.y;
            trans->z += posVector.z;
            player->endDemoPuppetable();
        }
    }
}
}