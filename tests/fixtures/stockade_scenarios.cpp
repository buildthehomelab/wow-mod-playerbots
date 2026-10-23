/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

UntypedValue* Context::Create(std::string const& name)
{
    StockadeValueContext factory;
    auto i = factory.creators.find(name);
    return i == factory.creators.end() ? nullptr : i->second(ai);
}

int main()
{
    Player bot;
    PlayerbotAI ai(&bot);
    Group group;
    bot.group = &group;
    Creature ward(10), captive(11), other(12);
    ward.entry = 1663;
    captive.entry = 1707;
    other.entry = 1717;
    for (Creature* enemy : {&ward, &captive, &other})
    {
        enemy->x = 3;
        enemy->motion.type = FLEEING_MOTION_TYPE;
        ai.units[enemy->guid] = enemy;
    }
    ai.context.GetValue<GuidVector>("attackers")->Set({captive.guid, other.guid, ward.guid, ObjectGuid(99)});
    StockadeStrategy strategy(&ai);
    std::vector<TriggerNode*> triggers;
    std::vector<Multiplier*> multipliers;
    strategy.InitTriggers(triggers);
    strategy.InitMultipliers(multipliers);
    assert(triggers.empty() && multipliers.empty() && !Stockade::FindFocus(&ai));
    sPlayerbotAIConfig.EnableStockadeMechanics = true;
    strategy.InitTriggers(triggers);
    strategy.InitMultipliers(multipliers);
    assert(triggers.size() == 1 && multipliers.size() == 1);
    StockadeActionContext actions;
    StockadeTriggerContext triggerContext;
    assert(actions.creators.contains(triggers[0]->actions[0].name));
    assert(triggerContext.creators.contains(triggers[0]->name));
    assert(Stockade::FindFocus(&ai) == ward.guid);
    for (uint32 entry : {1663u, 1666u, 1706u, 1707u, 1708u, 1711u, 1715u, 1716u, 1720u})
    {
        ward.entry = entry;
        assert(Stockade::CanFocus(&ai, &ward));
    }
    for (uint32 entry : {1696u, 1717u, 37063u, 0u})
    {
        ward.entry = entry;
        assert(!Stockade::CanFocus(&ai, &ward));
    }
    ward.entry = 1663;
    StockadeFocusAction focus(&ai);
    AttackAction ordinary(&ai, "attack anything");
    MeleeAction melee(&ai, "melee");
    Action heal(&ai, "heal");
    AttackMyTargetAction manualAttack(&ai, "attack my target");
    AttackDuelOpponentAction duelAttack(&ai, "attack duel opponent");
    // The command must survive before Execute records its prioritized target.
    assert(ai.context.GetValue<GuidVector>("prioritized targets")->Get().empty());
    assert(Stockade::GetFocus(&ai) == &ward);
    assert(multipliers[0]->GetValue(&manualAttack) == 1);
    assert(multipliers[0]->GetValue(&duelAttack) == 1);
    assert(multipliers[0]->GetValue(&ordinary) == 0);
    assert(multipliers[0]->GetValue(&melee) == 1 && multipliers[0]->GetValue(&heal) == 1);
    assert(focus.isUseful() && focus.Execute({}));
    assert(!focus.isUseful());
    ward.markedCc = true;
    assert(!focus.isUseful() && !focus.Execute({}));
    assert(multipliers[0]->GetValue(&ordinary) == 1);
    assert(!ai.context.GetValue<ObjectGuid>("stockade focus")->Get());
    clockNow += 500;
    assert(Stockade::GetFocus(&ai) == &captive);
    for (bool* invalid : {&captive.cc, &captive.markedCc})
    {
        *invalid = true;
        assert(!focus.Execute({}));
        assert(multipliers[0]->GetValue(&ordinary) == 1);
        *invalid = false;
        clockNow += 500;
    }
    captive.x = 13;
    assert(!focus.Execute({}));
    bot.ranged = true;
    clockNow += 500;
    assert(focus.Execute({}));
    captive.x = 31;
    assert(!focus.Execute({}));
    captive.x = 3;
    captive.los = false;
    assert(!focus.Execute({}) && multipliers[0]->GetValue(&ordinary) == 1);
    captive.los = true;
    for (bool* required : {&captive.alive, &captive.world, &captive.combat, &captive.valid})
    {
        *required = false;
        assert(!focus.Execute({}) && !Stockade::FindFocus(&ai));
        *required = true;
    }
    captive.reaction = REACT_PASSIVE;
    assert(!focus.Execute({}));
    captive.motion.type = ASSISTANCE_MOTION_TYPE;
    clockNow += 500;
    assert(Stockade::FindFocus(&ai) == captive.guid);
    assert(focus.Execute({}));
    captive.markedCc = true;
    assert(!focus.Execute({}) && multipliers[0]->GetValue(&ordinary) == 1);
    captive.markedCc = false;
    clockNow += 500;
    assert(focus.Execute({}));
    captive.motion.type = IDLE_MOTION_TYPE;
    assert(!focus.Execute({}));
    captive.reaction = REACT_AGGRESSIVE;
    captive.motion.type = IDLE_MOTION_TYPE;
    assert(!Stockade::FindFocus(&ai));
    captive.motion.type = TIMED_FLEEING_MOTION_TYPE;
    assert(Stockade::FindFocus(&ai) == captive.guid);
    group.skull = other.guid;
    assert(!focus.Execute({}) && !Stockade::FindFocus(&ai));
    group.skull = {};
    ai.context.GetValue<GuidVector>("prioritized targets")->Set({other.guid});
    assert(!focus.Execute({}) && !Stockade::FindFocus(&ai));
    other.alive = false;
    clockNow += 500;
    assert(Stockade::FindFocus(&ai) == captive.guid);
    assert(Stockade::GetFocus(&ai) == &captive && focus.Execute({}));
    other.alive = true;
    for (bool* required : {&other.world, &other.valid, &other.los})
    {
        *required = false;
        clockNow += 500;
        assert(Stockade::FindFocus(&ai) == captive.guid);
        assert(Stockade::GetFocus(&ai) == &captive && focus.Execute({}));
        *required = true;
    }
    ai.context.GetValue<GuidVector>("prioritized targets")->Set({ObjectGuid(999)});
    clockNow += 500;
    assert(Stockade::GetFocus(&ai) == &captive && focus.Execute({}));
    ai.context.GetValue<GuidVector>("prioritized targets")->Set({other.guid});
    assert(!focus.Execute({}));
    other.alive = false;
    group.skull = other.guid;
    clockNow += 500;
    assert(Stockade::GetFocus(&ai) == &captive && focus.Execute({}));
    other.alive = true;
    group.skull = {};
    ai.context.GetValue<GuidVector>("prioritized targets")->Set({});
    for (bool* role : {&bot.tank, &bot.heal})
    {
        *role = true;
        assert(!focus.Execute({}) && !Stockade::FindFocus(&ai));
        *role = false;
    }
    bot.map = 1;
    assert(!focus.Execute({}) && !Stockade::FindFocus(&ai));
    bot.map = Stockade::MAP_ID;
    ai.units.erase(captive.guid);
    assert(!focus.Execute({}));
    sPlayerbotAIConfig.EnableStockadeMechanics = false;
    assert(!focus.Execute({}) && multipliers[0]->GetValue(&ordinary) == 1);
    for (auto* item : triggers)
        delete item;
    for (auto* item : multipliers)
        delete item;
    std::cout << "Stockade source scenarios passed\n";
}
