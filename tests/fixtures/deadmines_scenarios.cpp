UntypedValue* Context::Create(std::string const& name)
{
    DeadminesValueContext factory;
    auto i = factory.creators.find(name);
    return i == factory.creators.end() ? nullptr : i->second(ai);
}

int main()
{
    Player bot, healer(2), tank(3);
    bot.map = healer.map = tank.map = Deadmines::MAP_ID;
    healer.heal = true;
    tank.tank = true;
    PlayerbotAI ai(&bot);
    Group group;
    bot.group = healer.group = tank.group = &group;
    Creature guard(10), runner(11), caster(12);
    for (Creature* enemy : {&guard, &runner, &caster})
    {
        enemy->map = bot.map;
        enemy->x = 3;
        ai.units[enemy->guid] = enemy;
    }
    guard.entry = 636;
    guard.victim = &healer;
    runner.motion.type = FLEEING_MOTION_TYPE;
    ai.context.GetValue<GuidVector>("attackers")->Set({runner.guid, guard.guid, caster.guid});
    DeadminesStrategy strategy(&ai);
    std::vector<TriggerNode*> triggers;
    std::vector<Multiplier*> multipliers;
    strategy.InitTriggers(triggers);
    strategy.InitMultipliers(multipliers);
    assert(triggers.empty() && multipliers.empty());
    assert(!Deadmines::FindFocus(&ai));
    sPlayerbotAIConfig.EnableDeadminesMechanics = true;
    strategy.InitTriggers(triggers);
    strategy.InitMultipliers(multipliers);
    assert(triggers.size() == 2 && multipliers.size() == 2);
    DeadminesActionContext actions;
    DeadminesTriggerContext triggerContext;
    for (TriggerNode* trigger : triggers)
    {
        assert(triggerContext.creators.contains(trigger->name));
        for (auto const& action : trigger->actions)
        {
            assert(actions.creators.contains(action.name));
            std::unique_ptr<Action> created(actions.creators[action.name](&ai));
            assert(created->getName() == action.name);
        }
    }
    assert(Deadmines::FindFocus(&ai) == guard.guid);
    DeadminesFocusAction focus(&ai);
    AttackAction ordinaryAttack(&ai, "attack anything");
    assert(multipliers[1]->GetValue(&ordinaryAttack) == 0);
    assert(focus.isUseful() && focus.Execute({}));
    guard.markedCc = true;  // A late CC assignment invalidates the cached candidate.
    assert(!focus.isUseful() && !focus.Execute({}));
    assert(multipliers[1]->GetValue(&ordinaryAttack) == 1);
    clockNow += 500;
    guard.markedCc = false;
    assert(Deadmines::GetFocus(&ai) == &guard);
    guard.x = 13;
    assert(!focus.Execute({}));
    assert(multipliers[1]->GetValue(&ordinaryAttack) == 1);
    bot.ranged = true;
    assert(focus.Execute({}));
    guard.los = false;
    assert(multipliers[1]->GetValue(&ordinaryAttack) == 1);
    assert(!focus.Execute({}));
    guard.los = true;
    guard.victim = &tank;
    assert(Deadmines::FindFocus(&ai) == runner.guid);
    runner.cc = true;
    assert(!Deadmines::FindFocus(&ai));
    runner.cc = false;
    group.skull = caster.guid;
    assert(!focus.Execute({}) && !Deadmines::FindFocus(&ai));
    group.skull = {};
    ai.context.GetValue<GuidVector>("prioritized targets")->Set({caster.guid});
    assert(!Deadmines::FindFocus(&ai));
    ai.context.GetValue<GuidVector>("prioritized targets")->Set({});
    for (bool* role : {&bot.tank, &bot.heal})
    {
        *role = true;
        assert(!Deadmines::FindFocus(&ai));
        *role = false;
    }
    assert(Deadmines::FindFocus(&ai) == runner.guid);
    bot.map = 0;
    assert(!Deadmines::FindFocus(&ai) && !Deadmines::CollectHazard(&ai).active);
    bot.map = Deadmines::MAP_ID;

    SpellInfo splash;
    splash.Id = 6306;
    for (auto& effect : splash.Effects)
        effect.radius = 7;
    Spell cast{&splash, {}};
    caster.currentSpell[CURRENT_GENERIC_SPELL] = &cast;
    auto area = Deadmines::CollectHazard(&ai);
    assert(area.active && area.radius == 10);  // Uses loaded geometry, not a DBC index as yards.
    caster.x = 10;
    assert(!Deadmines::CollectHazard(&ai).active);
    caster.x = 3;
    splash.Id = 5159;
    assert(Deadmines::CollectHazard(&ai).active);
    splash.Id = 5213;  // Gilnid's target DoT is not a ground hazard.
    assert(!Deadmines::CollectHazard(&ai).active);
    caster.currentSpell[CURRENT_GENERIC_SPELL] = nullptr;

    DynamicObject large, floor;
    large.radius = 100;
    Aura ignored, harmful;
    ignored.area = &large;
    harmful.area = &floor;
    AuraEffect first{&ignored}, second{&harmful};
    bot.effects[SPELL_AURA_PERIODIC_DAMAGE] = {&first, &second};
    area = Deadmines::CollectHazard(&ai);
    assert(area.active && area.radius == 5);
    large.radius = 4;
    ignored.info.Id = 100;
    sPlayerbotAIConfig.AoeAvoidSpellWhitelist.insert(100);
    assert(Deadmines::CollectHazard(&ai).radius == 5);
    bot.combat = false;  // A lingering ground area remains dangerous after combat ends.
    assert(Deadmines::CollectHazard(&ai).active);
    bot.combat = true;
    // Both circles are individually actionable and share one enclosing escape objective.
    DynamicObject overlap;
    overlap.x = 4;
    Aura overlapping;
    overlapping.area = &overlap;
    AuraEffect third{&overlapping};
    bot.effects[SPELL_AURA_PERIODIC_DAMAGE].push_back(&third);
    auto combined = Deadmines::CollectHazard(&ai);
    assert(combined.radius == 7 && combined.center.x == 2);
    DeadminesEscapeAction escape(&ai);
    assert(escape.isUseful() && escape.Execute({}));
    assert(ai.moves == 1 && bot.castStops == 1 && ai.movement.clears == 1);
    assert(std::hypot(bot.spline.destination.x - overlap.x, bot.spline.destination.y - overlap.y) > overlap.radius);
    bot.effects[SPELL_AURA_PERIODIC_DAMAGE].pop_back();
    clockNow += 100;
    assert(escape.Execute({}) && ai.moves == 1);  // Keep the existing escape spline.
    floor.radius = std::abs(bot.spline.destination.x) + 1;
    floor.x = bot.spline.destination.x;  // The old destination becomes unsafe.
    assert(!escape.Execute({}));
    clockNow += 500;
    assert(escape.Execute({}) && ai.moves == 2);
    bot.spline.done = true;
    ai.moveBlocked = true;
    clockNow += 500;
    assert(!escape.Execute({}));
    int clears = ai.movement.clears;
    clockNow += 100;
    assert(!escape.Execute({}) && ai.movement.clears == clears);
    MovementAction approach(&ai, "approach");
    CastSpellAction heal(&ai, "heal");
    assert(multipliers[0]->GetValue(&approach) == 0);
    assert(multipliers[0]->GetValue(&heal) == 0);
    assert(multipliers[0]->GetValue(&escape) == 1);
    CastEscapeArtistAction escapeArtist(&ai);
    ai.canMove = false;
    int castStopsBeforeRoot = bot.castStops;
    assert(!escape.isUseful() && !escape.Execute({}));
    assert(bot.castStops == castStopsBeforeRoot);
    assert(multipliers[0]->GetValue(&escapeArtist) == 1);
    assert(multipliers[0]->GetValue(&heal) == 1);
    assert(multipliers[0]->GetValue(&approach) == 0);
    ai.canMove = true;
    assert(escape.isUseful() && multipliers[0]->GetValue(&heal) == 0);
    harmful.expired = true;
    assert(!escape.Execute({}));
    clockNow += 250;
    assert(multipliers[0]->GetValue(&heal) == 1);
    // A wide floor needs several real FleeDistance-limited steps to leave.
    ai.moveBlocked = false;
    bot.moving = false;
    floor.x = floor.y = bot.x = bot.y = 0;
    floor.radius = 12;
    harmful.expired = false;
    DeadminesEscapeAction wideEscape(&ai);
    int initialMoves = ai.moves;
    for (int step = 0; step < 3; ++step)
    {
        clockNow += 600;
        assert(wideEscape.Execute({}));
        assert(ai.moves == initialMoves + step + 1);
        assert(bot.spline.destination.x == bot.x + 5);
        clockNow += 100;
        assert(wideEscape.Execute({}) && ai.moves == initialMoves + step + 1);
        bot.Relocate(bot.spline.destination.x, bot.spline.destination.y, bot.spline.destination.z);
        bot.spline.done = true;
        bot.moving = false;
    }
    assert(bot.x == 15 && !Deadmines::CollectHazard(&ai).active && !wideEscape.Execute({}));
    // Collision/no-progress endpoints must still be stopped.
    bot.x = 0;
    ai.fleeDistance = 0;
    clockNow += 600;
    assert(!wideEscape.Execute({}) && !bot.moving && bot.spline.done);
    ai.fleeDistance = 5;
    sPlayerbotAIConfig.EnableDeadminesMechanics = false;
    harmful.expired = false;
    assert(!Deadmines::InHazard(&ai) && !Deadmines::CollectHazard(&ai).active);
    for (auto* trigger : triggers)
        delete trigger;
    for (auto* multiplier : multipliers)
        delete multiplier;
    std::cout << "Deadmines source execution scenarios passed\n";
}
