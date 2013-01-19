#include "dragon-package.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"
#include "card.h"
#include "structs.h"

//Lingfeng

DragonFengwangCard::DragonFengwangCard(){

}

bool DragonFengwangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void DragonFengwangCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    QString choice;
    bool usedJia = false, usedJian = false;
    foreach(ServerPlayer *player, room->getAllPlayers()){
        if(player->getMark("@wind_jia") > 0){
            usedJia = true;
        }
        if(player->getMark("@wind_jian") > 0){
            usedJian = true;
        }
    }
    if(!usedJia && !usedJian){
        choice = room->askForChoice(source, "dragonfengwang", "shunfeng+nifeng");
    }else if(usedJia){
        choice = "shunfeng";
    }else{
        choice = "nifeng";
    }

    if (choice == "shunfeng") {
        targets[0]->gainMark("@wind_jian");
        targets[0]->acquireSkill("dragonfengwang-feng");
        room->setPlayerFlag(source, "used_wind_jian");
    } else {
        targets[0]->gainMark("@wind_jia");
        targets[0]->acquireSkill("dragonfengwang-feng");
        room->setPlayerFlag(source, "used_wind_jia");
    }
}

class DragonFengwangViewAsSkill: public OneCardViewAsSkill{

public:
    DragonFengwangViewAsSkill():OneCardViewAsSkill("dragonfengwang"){

    }

    virtual bool viewFilter(const Card* to_select) const{
        return true;
    }

    virtual const Card *viewAs(const Card *originalCard) const{

        DragonFengwangCard *fengwang_card = new DragonFengwangCard();
        fengwang_card->addSubcard(originalCard->getId());
        fengwang_card->setSkillName(objectName());
        return fengwang_card;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        bool usedJia = false, usedJian = false;
        if(player->hasFlag("used_wind_jia")){
            usedJia = true;
        }
        if(player->hasFlag("used_wind_jian")){
            usedJian = true;
        }

        if (usedJia && usedJian){
            return false;
        }else{
            return true;
        }
    }
};

class DragonFengwang: public DistanceSkill{
public:
    DragonFengwang():DistanceSkill("dragonfengwang-feng"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if(from->getMark("@wind_jian") > 0)
            correct --;
        if(to->getMark("@wind_jia") > 0)
            correct ++;
        return correct;
    }
};

class DragonFengwangClear: public TriggerSkill{
public:
    DragonFengwangClear():TriggerSkill("#dragonfengwang-clear"){
        events << Death << EventPhaseStart;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->getGeneralName().contains("dragon_lingfeng");
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
        if(triggerEvent == Death){
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *player, players){
                player->loseAllMarks("@wind_jian");
                player->loseAllMarks("@wind_jia");
                player->loseSkill("dragonfengwang-feng");
            }
        }
        else if(triggerEvent == EventPhaseStart && player->getPhase() == Player::RoundStart){
            foreach(ServerPlayer *player, room->getAllPlayers()){
                if(player->getMark("@wind_jian") > 0)
                    player->loseMark("@wind_jian");
                if(player->getMark("@wind_jia") > 0)
                    player->loseMark("@wind_jia");
                player->loseSkill("dragonfengwang-feng");
            }
        }

        return false;
    }
};

class DragonShensu: public TriggerSkill{
public:
    DragonShensu():TriggerSkill("dragon_shensu"){
        events << CardResponsed;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *lingfeng, QVariant &data) const{
        if (lingfeng == NULL) return false;
        CardStar card_star = data.value<ResponsedStruct>().m_card;
        if(!card_star->isKindOf("Jink"))
            return false;

        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName(objectName());
        CardUseStruct use;
        use.card = slash;
        use.from = lingfeng;
        use.to << room->getCurrent();

        room->useCard(use);

        return false;
    }
};


// Chen

class DragonAnXue: public TriggerSkill{
public:
    DragonAnXue():TriggerSkill("dragonanxue"){
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player == NULL) return false;
        const Card *card = data.value<CardEffectStruct>().card;
        if(card->isKindOf("Slash") || card->isKindOf("Duel")){
            if(card->isBlack()) {
                LogMessage log;
                log.type = "#dragonanxue";
                log.from = player;
                log.arg = objectName();
                player->getRoom()->sendLog(log);

                room->setEmotion(player, "armor/renwang_shield");

                return true;
            }
        }
        return false;
    }
};

class DragonSanYuViewAsSkill:public ZeroCardViewAsSkill{
public:
    DragonSanYuViewAsSkill():ZeroCardViewAsSkill("dragonsanyu"){

    }

    virtual const Card *viewAs() const{
        DragonSanYuCard *dismantlement = new DragonSanYuCard();
        dismantlement->setSkillName(objectName());
        return dismantlement;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@dragonsanyu";
    }
};

class DragonSanYu: public PhaseChangeSkill{
public:
    DragonSanYu():PhaseChangeSkill("dragonsanyu"){
        view_as_skill = new DragonSanYuViewAsSkill;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Discard){
            Room *room = target->getRoom();

            int x = target->getHandcardNum() - target->getHp();
            if(x > 0) {
                room->askForDiscard(target, "dragonsanyu", x, x);
                target->gainMark("@yumao", x);
                room->askForUseCard(target, "@@dragonsanyu", "@dragonsanyu");
                target->loseAllMarks("@yumao");

                LogMessage log;
                log.type = "#dragonsanyu";
                log.from = target;
                log.arg = QString::number(x);
                log.arg2 = objectName();
                room->sendLog(log);
            }
        }
        return false;
    }
};

DragonSanYuCard::DragonSanYuCard()
{

}

bool DragonSanYuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    int sanyu_number = Self->getMark("@yumao");

    if(targets.length() >= sanyu_number) {
        return false;
    }

    if(to_select->getMark("@sanyu_target") > 0)
        return false;

    if(to_select->isAllNude())
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void DragonSanYuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    foreach(ServerPlayer *target, targets){
        target->gainMark("@sanyu_target");

        int card_id = room->askForCardChosen(source, target, "hej", objectName());
        CardMoveReason reason(CardMoveReason::S_REASON_DISMANTLE, target->objectName());
        reason.m_playerId = source->objectName();
        reason.m_targetId = target->objectName();
        room->moveCardTo(Sanguosha->getCard(card_id), NULL, NULL, Player::DiscardPile, reason);

        LogMessage log;
        log.type = "$Dismantlement";
        log.from = target;
        log.card_str = QString::number(card_id);
        room->sendLog(log);
    }
}

class DragonTianFa: public PhaseChangeSkill{
public:
    DragonTianFa():PhaseChangeSkill("dragontianfa") {
        frequency = Limited;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Discard){
            bool canUse = false;
            Room *room = target->getRoom();
            foreach(ServerPlayer *player, room->getAllPlayers()){
                if(player->getMark("@sanyu_target") > 0)
                    canUse = true;
            }
            if(target->getMark("@tianfa") <= 0)
                canUse = false;

            if (canUse && room->askForSkillInvoke(target, objectName())){
                foreach(ServerPlayer *player, room->getAllPlayers()){
                    if(player->getMark("@sanyu_target") > 0){
                        DamageStruct damage;
                        damage.damage = 1;
                        damage.from = target;
                        damage.to = player;
                        damage.nature = DamageStruct::Fire;

                        target->loseMark("@tianfa");
                        room->damage(damage);
                    }
                }
            }
            foreach(ServerPlayer *player, room->getAllPlayers()){
                if(player->getMark("@sanyu_target") > 0)
                    player->loseMark("@sanyu_target");
            }
        }
        return false;
    }
};

// Bingya
class DragonPoMo: public OneCardViewAsSkill{
public:
    DragonPoMo():OneCardViewAsSkill("dragonpomo"){

    }

    virtual bool viewFilter(const Card* to_select) const{
        return to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{

        DragonPoMoCard *pomo_card = new DragonPoMoCard();
        pomo_card->addSubcard(originalCard->getId());
        pomo_card->setSkillName(objectName());
        return pomo_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("DragonPoMoCard");
    }
};

DragonPoMoCard::DragonPoMoCard(){
    once = true;
}

bool DragonPoMoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if(to_select == Self)
        return false;

    return targets.isEmpty();
}

void DragonPoMoCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *first = targets[0];
    ServerPlayer *second = source;
    bool target_used_slash = false;

    room->setEmotion(first, "duel");
    room->setEmotion(second, "duel");

    forever{
        if (!first->isAlive())
            break;

        const Card *slash = room->askForCard(first,
                                             "slash",
                                             "duel-slash:" + second->objectName(),
                                             QVariant(),
                                             CardResponsed,
                                             second);
        if(slash == NULL)
            break;

        target_used_slash = true;

        qSwap(first, second);
    }

    DamageStruct damage;
    damage.card = this;
    if(second->isAlive())
        damage.from = second;
    else
        damage.from = NULL;
    damage.to = first;

    room->damage(damage);

    if(!target_used_slash){
        QString choice;
        if(targets[0]->isAlive()){
            choice = room->askForChoice(source, "dragonpomo", "zhuapai+qipai");
        }else{
            choice = "zhuapai";
        }

        if(choice == "zhuapai"){
            source->drawCards(2);
        }else{
            int card_id = room->askForCardChosen(source, targets[0], "he", "dragonpomo");
            room->throwCard(Sanguosha->getCard(card_id), targets[0], source);

            if(!targets[0]->isNude()){
                int card_id = room->askForCardChosen(source, targets[0], "he", "dragonpomo");
                room->throwCard(Sanguosha->getCard(card_id), targets[0], source);
            }
        }
    }
}

//Siling

class DragonBuXi: public TriggerSkill{
public:
    DragonBuXi():TriggerSkill("dragonbuxi"){
        events << EventPhaseStart << FinishJudge;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(triggerEvent == EventPhaseStart && player->getPhase() == Player::Start){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(spade|club):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = player;
            judge.time_consuming = true;

            room->judge(judge);
            if(!judge.isBad()){
                RecoverStruct recover;
                recover.who = player;
                room->recover(player, recover);
            }
        }else if(triggerEvent == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason == objectName()){
                if(judge->card->isBlack()){
                    player->obtainCard(judge->card);
                }
            }
        }

        return false;
    }
};

//Yinyue

class DragonZhiLiao: public TriggerSkill{
public:
    DragonZhiLiao():TriggerSkill("dragonzhiliao"){
        events << CardFinished;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *yinyue, QVariant &data) const{
        switch(event){
        case CardFinished: {
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->isKindOf("Peach")){
                ServerPlayer *target = use.to.isEmpty() ? yinyue : use.to[0];

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade|club):([A])");
                judge.good = false;
                judge.reason = objectName();
                judge.who = target;
                judge.play_animation = true;

                room->judge(judge);

                if(judge.isGood()) {
                    LogMessage log;
                    log.type = "#DragonZhiliaoExtraRecover";
                    log.from = yinyue;
                    log.to << target;
                    log.arg = objectName();
                    room->sendLog(log);

                    RecoverStruct recover;
                    recover.card = use.card;
                    recover.who = target;
                    room->recover(target, recover);
                }
                else{
                    room->broadcastSkillInvoke(objectName());

                    LogMessage log;
                    log.type = "#DragonZhiliaoDead";
                    log.from = yinyue;
                    log.to << target;
                    log.arg = objectName();
                    room->sendLog(log);

                    room->killPlayer(target);
                }
            }
            break;
        }
        default:
            break;
        }

        return false;
    }
};

class DragonJieJingPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return !player->hasEquip(card) && (card->getSuit() == Card::Spade || card->getSuit() == Card::Club);
    }

    virtual bool willThrow() const{
        return false;
    }
};

class DragonJieJingClient: public TriggerSkill{
public:
    DragonJieJingClient():TriggerSkill("dragonjiejing-client"){
        events << EventPhaseStart;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(triggerEvent == EventPhaseStart){
            if(player->getPhase() == Player::Start){
                int extraCards = player->getMaxHp() - player->getHandcardNum();
                if(extraCards > 0){
                    QList<ServerPlayer *> players = room->getAllPlayers();
                    const Card *card = NULL;
                    bool canGo = false;
                    foreach(ServerPlayer *player, players){
                        if(player->getMark("jiejingOwner") > 0 && !player->isKongcheng()){
                            card = room->askForCard(player, ".jiejing", "@dragonjiejing", data, CardDiscarded);
                            if (card != NULL && card->isBlack()){
                                if (player->hasInnateSkill("dragonjiejing"))
                                    room->broadcastSkillInvoke("dragonjiejing");
                                canGo = true;
                            }
                        }
                    }

                    if(canGo){
                        if(player->getMark("jiejingOwner") > 0){ //self
                            extraCards = player->getMaxHp() - player->getHandcardNum();
                        }
                        player->drawCards(extraCards, false, "dragonjiejing_draw");
                        player->skip(Player::Judge);
                        player->skip(Player::Draw);
                        player->skip(Player::Play);
                        player->skip(Player::Discard);
                    }
                }
            }
        }
        return false;
    }
};

class DragonJieJingStart: public GameStartSkill{
public:
    DragonJieJingStart(): GameStartSkill("dragonjiejing"){

    }

    virtual int getPriority() const{
        return -1;
    }

    virtual void onGameStart(ServerPlayer *yinyue) const{
        yinyue->getRoom()->setPlayerMark(yinyue, "jiejingOwner", 1);
        QList<ServerPlayer *> players = yinyue->getRoom()->getAllPlayers();
        foreach(ServerPlayer *player, players){
            player->acquireSkill("dragonjiejing-client");
        }
    }
};

class DragonJieJingClear: public TriggerSkill{
public:
    DragonJieJingClear():TriggerSkill("#dragonjiejing-clear"){
        events << Death;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->getMark("jiejingOwner") > 0;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
        if(triggerEvent == Death){
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *player, players){
                player->loseSkill("dragonjiejing-client");
            }
        }

        return false;
    }
};

DragonPackage::DragonPackage()
    :Package("Dragon")
{
    General *lingfeng = new General(this, "dragon_lingfeng", "god", 3);
    lingfeng->addSkill(new DragonShensu);
    lingfeng->addSkill(new DragonFengwangViewAsSkill);
    lingfeng->addSkill(new DragonFengwangClear);
    related_skills.insertMulti("dragonfengwang", "#dragonfengwang-clear");
    related_skills.insertMulti("dragonfengwang", "dragonfengwang-feng");

    General *chen = new General(this, "dragon_chen", "god", 3);
    chen->addSkill(new DragonAnXue);
    chen->addSkill(new DragonSanYu);
    chen->addSkill(new DragonTianFa);
    chen->addSkill(new MarkAssignSkill("@tianfa", 1));

    General *bingya = new General(this, "dragon_bingya", "god", 4);
    bingya->addSkill(new DragonPoMo);

    General *siling = new General(this, "dragon_siling", "god", 4);
    siling->addSkill(new DragonBuXi);

    General *yinyue = new General(this, "dragon_yinyue", "god", 3);
    yinyue->addSkill(new DragonZhiLiao);
    yinyue->addSkill(new DragonJieJingStart);
    yinyue->addSkill(new DragonJieJingClear);
    patterns.insert(".jiejing", new DragonJieJingPattern);
    related_skills.insertMulti("dragonjiejing", "dragonjiejing-client");
    related_skills.insertMulti("dragonjiejing", "#dragonjiejing-clear");

    //
    addMetaObject<DragonFengwangCard>();
    addMetaObject<DragonSanYuCard>();
    addMetaObject<DragonPoMoCard>();

    skills << new DragonFengwang;
    skills << new DragonJieJingClient;
}

ADD_PACKAGE(Dragon)
