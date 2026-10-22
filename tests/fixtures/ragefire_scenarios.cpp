UntypedValue* Context::Create(std::string const& name)
{
    RagefireValueContext factory;
    auto i = factory.creators.find(name);
    return i == factory.creators.end() ? nullptr : i->second(ai);
}

int main()
{
    Player bot;
    bot.map = Ragefire::MAP_ID;
    PlayerbotAI ai(&bot);
    Group group;
    bot.group = &group;
    Creature runner(11), caster(12);
    for (Creature* enemy : {&runner, &caster})
    {
        enemy->map = bot.map;
        enemy->x = 3;
        ai.units[enemy->guid] = enemy;
    }
    runner.motion.type = FLEEING_MOTION_TYPE;
    ai.context.GetValue<GuidVector>("attackers")->Set({runner.guid, caster.guid});
    RagefireStrategy strategy(&ai);
    std::vector<TriggerNode*> triggers;
    std::vector<Multiplier*> multipliers;
    strategy.InitTriggers(triggers);
    strategy.InitMultipliers(multipliers);
    assert(triggers.empty() && multipliers.empty());
    assert(!Ragefire::FindFocus(&ai));
    sPlayerbotAIConfig.EnableRagefireMechanics = true;
    strategy.InitTriggers(triggers);
    strategy.InitMultipliers(multipliers);
    assert(triggers.size() == 2 && multipliers.size() == 2);
    RagefireActionContext actions;
    RagefireTriggerContext triggerContext;
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
    assert(Ragefire::FindFocus(&ai) == runner.guid);
    RagefireFocusAction focus(&ai);
    AttackAction ordinaryAttack(&ai, "attack anything");
    assert(multipliers[1]->GetValue(&ordinaryAttack) == 0);
    assert(focus.isUseful() && focus.Execute({}));
    runner.markedCc = true;
    assert(!focus.isUseful() && !focus.Execute({}));
    assert(multipliers[1]->GetValue(&ordinaryAttack) == 1);
    clockNow += 500;
    runner.markedCc = false;
    assert(Ragefire::GetFocus(&ai) == &runner);
    runner.x = 13;
    assert(!focus.Execute({}));
    assert(multipliers[1]->GetValue(&ordinaryAttack) == 1);
    bot.ranged = true;
    assert(focus.Execute({}));
    runner.los = false;
    assert(multipliers[1]->GetValue(&ordinaryAttack) == 1 && !focus.Execute({}));
    runner.los = true;
    runner.cc = true;
    assert(!Ragefire::FindFocus(&ai));
    runner.cc = false;
    group.skull = caster.guid;
    assert(!focus.Execute({}) && !Ragefire::FindFocus(&ai));
    group.skull = {};
    ai.context.GetValue<GuidVector>("prioritized targets")->Set({caster.guid});
    assert(!Ragefire::FindFocus(&ai));
    ai.context.GetValue<GuidVector>("prioritized targets")->Set({});
    for (bool* role : {&bot.tank, &bot.heal})
    {
        *role = true;
        assert(!Ragefire::FindFocus(&ai));
        *role = false;
    }
    runner.motion.type = TIMED_FLEEING_MOTION_TYPE;
    assert(Ragefire::FindFocus(&ai) == runner.guid);
    runner.motion.type = IDLE_MOTION_TYPE;
    assert(!Ragefire::FindFocus(&ai));
    bot.map = 0;
    assert(!Ragefire::FindFocus(&ai) && !Ragefire::CollectHazard(&ai).active);
    bot.map = Ragefire::MAP_ID;

    SpellInfo nova;
    nova.Id = 11970;
    for (auto& effect : nova.Effects)
        effect.radius = 9;
    Spell cast{&nova, {}};
    caster.currentSpell[CURRENT_GENERIC_SPELL] = &cast;
    auto area = Ragefire::CollectHazard(&ai);
    assert(area.active && area.radius == 12);
    caster.x = 12;
    assert(!Ragefire::CollectHazard(&ai).active);
    caster.x = 0;
    nova.Id = 18070;
    assert(!Ragefire::CollectHazard(&ai).active);
    nova.Id = 11970;
    bot.combat = false;
    assert(!Ragefire::CollectHazard(&ai).active);
    bot.combat = true;
    clockNow += 250;
    RagefireEscapeAction escape(&ai);
    MovementAction approach(&ai, "approach");
    CastSpellAction heal(&ai, "heal");
    CastEscapeArtistAction escapeArtist(&ai);
    assert(multipliers[0]->GetValue(&approach) == 0);
    assert(multipliers[0]->GetValue(&heal) == 1);
    CastSpellAction interrupt(&ai, "counterspell");
    assert(multipliers[0]->GetValue(&interrupt) == 1);
    assert(multipliers[1]->GetValue(&focus) == 0);
    assert(triggers.front()->actions.front().score < ACTION_INTERRUPT);
    assert(triggers.front()->actions.front().score < ACTION_EMERGENCY);
    assert(multipliers[0]->GetValue(&escape) == 1);
    ai.canMove = false;
    int beforeRoot = bot.castStops;
    assert(!escape.isUseful() && !escape.Execute({}));
    assert(bot.castStops == beforeRoot);
    assert(multipliers[0]->GetValue(&escapeArtist) == 1 && multipliers[0]->GetValue(&heal) == 1);
    ai.canMove = true;
    int initialMoves = ai.moves;
    for (int step = 0; step < 3; ++step)
    {
        clockNow += 600;
        assert(escape.Execute({}));
        assert(ai.moves == initialMoves + step + 1);
        assert(bot.spline.destination.x == bot.x + 5);
        clockNow += 100;
        assert(escape.Execute({}) && ai.moves == initialMoves + step + 1);
        bot.Relocate(bot.spline.destination.x, bot.spline.destination.y, bot.spline.destination.z);
        bot.spline.done = true;
        bot.moving = false;
    }
    assert(bot.x == 15 && !Ragefire::CollectHazard(&ai).active && !escape.Execute({}));
    bot.x = 0;
    ai.fleeDistance = 0;
    int beforeFailure = bot.castStops;
    clockNow += 600;
    assert(!escape.Execute({}) && !bot.moving && bot.spline.done);
    assert(bot.castStops == beforeFailure);
    ai.fleeDistance = 5;
    ai.moveBlocked = true;
    clockNow += 600;
    assert(!escape.Execute({}));
    int clears = ai.movement.clears;
    clockNow += 100;
    assert(!escape.Execute({}) && ai.movement.clears == clears);
    assert(bot.castStops == beforeFailure && ai.movement.clears == clears);
    ai.moveBlocked = false;
    ai.pathRejected = true;
    clockNow += 600;
    assert(!escape.Execute({}) && bot.castStops == beforeFailure && ai.movement.clears == clears);
    ai.pathRejected = false;
    bot.movementCast = true;
    clockNow += 600;
    assert(escape.Execute({}) && bot.castStops == beforeFailure + 1);
    assert(bot.deferredMove && bot.spline.done && !bot.movementCast);
    bot.MovementUpdate();
    int acceptedMoves = ai.moves;
    clockNow += 100;
    assert(escape.Execute({}) && ai.moves == acceptedMoves && bot.castStops == beforeFailure + 1);
    assert(!bot.spline.done && bot.spline.destination.x == 5);
    caster.currentSpell[CURRENT_GENERIC_SPELL] = nullptr;
    clockNow += 250;
    assert(!escape.Execute({}) && multipliers[0]->GetValue(&heal) == 1);
    sPlayerbotAIConfig.EnableRagefireMechanics = false;
    assert(!Ragefire::InHazard(&ai) && !Ragefire::CollectHazard(&ai).active);
    for (auto* trigger : triggers)
        delete trigger;
    for (auto* multiplier : multipliers)
        delete multiplier;
    std::cout << "Ragefire source execution scenarios passed\n";
}
