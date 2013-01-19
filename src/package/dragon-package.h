#ifndef DRAGONPACKAGE_H
#define DRAGONPACKAGE_H

#include "package.h"
#include "card.h"
#include "standard.h"

class DragonPackage: public Package {
    Q_OBJECT
public:
    DragonPackage();
};

class DragonFengwangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DragonFengwangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class DragonSanYuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DragonSanYuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class DragonPoMoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DragonPoMoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};


#endif // DRAGONPACKAGE_H
