UntypedValue* Context::Create(std::string const& name)
{
    WailingCavernsValueContext factory;
    auto i = factory.creators.find(name);
    return i == factory.creators.end() ? nullptr : i->second(ai);
}

int main()
{
    Player bot;
    PlayerbotAI ai(&bot);
    Group group;
    bot.group = &group;
    Creature skum(10), druid(11), other(12);
    skum.entry = 3674;
    druid.entry = 3840;
    other.entry = 3637;
    for (Creature* enemy : {&skum, &druid, &other})
    {
        enemy->x = 3;
        enemy->motion.type = FLEEING_MOTION_TYPE;
        ai.units[enemy->guid] = enemy;
    }
    ai.context.GetValue<GuidVector>("attackers")->Set({druid.guid, other.guid, skum.guid, ObjectGuid(99)});
    WailingCavernsStrategy strategy(&ai);
    std::vector<TriggerNode*> triggers;
    std::vector<Multiplier*> multipliers;
    strategy.InitTriggers(triggers);
    strategy.InitMultipliers(multipliers);
    assert(triggers.empty() && multipliers.empty() && !WailingCaverns::FindFocus(&ai));
    sPlayerbotAIConfig.EnableWailingCavernsMechanics = true;
    strategy.InitTriggers(triggers);
    strategy.InitMultipliers(multipliers);
    assert(triggers.size() == 1 && multipliers.size() == 1);
    WailingCavernsActionContext actions;
    WailingCavernsTriggerContext triggerContext;
    assert(actions.creators.contains(triggers[0]->actions[0].name));
    assert(triggerContext.creators.contains(triggers[0]->name));
    assert(WailingCaverns::FindFocus(&ai) == skum.guid);
    WailingCavernsFocusAction focus(&ai);
    AttackAction ordinary(&ai, "attack anything");
    MeleeAction melee(&ai, "melee");
    Action heal(&ai, "heal");
    assert(multipliers[0]->GetValue(&ordinary) == 0);
    assert(multipliers[0]->GetValue(&melee) == 1 && multipliers[0]->GetValue(&heal) == 1);
    assert(focus.isUseful() && focus.Execute({}));
    assert(!focus.isUseful());
    skum.markedCc = true;
    assert(!focus.isUseful() && !focus.Execute({}));
    assert(multipliers[0]->GetValue(&ordinary) == 1);
    assert(!ai.context.GetValue<ObjectGuid>("wailing caverns focus")->Get());
    clockNow += 500;
    assert(WailingCaverns::GetFocus(&ai) == &druid);
    for (bool* invalid : {&druid.cc, &druid.markedCc})
    {
        *invalid = true;
        assert(!focus.Execute({}));
        assert(multipliers[0]->GetValue(&ordinary) == 1);
        *invalid = false;
        clockNow += 500;
    }
    druid.x = 13;
    assert(!focus.Execute({}));
    bot.ranged = true;
    clockNow += 500;
    assert(focus.Execute({}));
    druid.x = 31;
    assert(!focus.Execute({}));
    druid.x = 3;
    druid.los = false;
    assert(!focus.Execute({}) && multipliers[0]->GetValue(&ordinary) == 1);
    druid.los = true;
    for (bool* required : {&druid.alive, &druid.world, &druid.combat, &druid.valid})
    {
        *required = false;
        assert(!focus.Execute({}) && !WailingCaverns::FindFocus(&ai));
        *required = true;
    }
    druid.reaction = REACT_PASSIVE;
    assert(!focus.Execute({}));
    druid.motion.type = ASSISTANCE_MOTION_TYPE;
    clockNow += 500;
    assert(WailingCaverns::FindFocus(&ai) == druid.guid);
    assert(focus.Execute({}));
    druid.markedCc = true;
    assert(!focus.Execute({}) && multipliers[0]->GetValue(&ordinary) == 1);
    druid.markedCc = false;
    clockNow += 500;
    assert(focus.Execute({}));
    druid.motion.type = IDLE_MOTION_TYPE;
    assert(!focus.Execute({}));
    druid.reaction = REACT_AGGRESSIVE;
    druid.motion.type = IDLE_MOTION_TYPE;
    assert(!WailingCaverns::FindFocus(&ai));
    druid.motion.type = TIMED_FLEEING_MOTION_TYPE;
    assert(WailingCaverns::FindFocus(&ai) == druid.guid);
    group.skull = other.guid;
    assert(!focus.Execute({}) && !WailingCaverns::FindFocus(&ai));
    group.skull = {};
    ai.context.GetValue<GuidVector>("prioritized targets")->Set({other.guid});
    assert(!focus.Execute({}) && !WailingCaverns::FindFocus(&ai));
    ai.context.GetValue<GuidVector>("prioritized targets")->Set({});
    for (bool* role : {&bot.tank, &bot.heal})
    {
        *role = true;
        assert(!focus.Execute({}) && !WailingCaverns::FindFocus(&ai));
        *role = false;
    }
    bot.map = 1;
    assert(!focus.Execute({}) && !WailingCaverns::FindFocus(&ai));
    bot.map = WailingCaverns::MAP_ID;
    ai.units.erase(druid.guid);
    assert(!focus.Execute({}));
    sPlayerbotAIConfig.EnableWailingCavernsMechanics = false;
    assert(!focus.Execute({}) && multipliers[0]->GetValue(&ordinary) == 1);
    for (auto* item : triggers)
        delete item;
    for (auto* item : multipliers)
        delete item;
    std::cout << "Wailing Caverns source scenarios passed\n";
}
