UntypedValue* Context::Create(std::string const& name)
{
    BlackfathomAvoidanceValueContext factory;
    auto i = factory.creators.find(name);
    return i == factory.creators.end() ? nullptr : i->second(ai);
}
int main()
{
    Player bot;
    bot.map = 48;
    PlayerbotAI ai(&bot);
    Creature caster(12);
    caster.map = 48;
    ai.units[caster.guid] = &caster;
    ai.context.GetValue<GuidVector>("attackers")->Set({caster.guid});
    sPlayerbotAIConfig.EnableBlackfathomMechanics = true;
    SpellInfo nova;
    nova.Id = 3815;
    for (auto& effect : nova.Effects)
        effect.radius = 9;
    Spell cast{&nova, {}};
    caster.currentSpell[CURRENT_GENERIC_SPELL] = &cast;
    auto area = Blackfathom::CollectHazard(&ai);
    assert(area.active && area.radius == 12);
    caster.x = 12;
    assert(!Blackfathom::CollectHazard(&ai).active);
    caster.x = 0;
    nova.Id = 18070;
    assert(!Blackfathom::CollectHazard(&ai).active);
    nova.Id = 3815;
    bot.combat = false;
    assert(!Blackfathom::CollectHazard(&ai).active);
    bot.combat = true;
    clockNow += 250;
    BlackfathomEscapeAction escape(&ai);
    ai.canMove = false;
    int beforeRoot = bot.castStops;
    assert(!escape.isUseful() && !escape.Execute({}));
    assert(bot.castStops == beforeRoot);
    ai.canMove = true;
    nova.Id = 8399;
    assert(Blackfathom::CollectHazard(&ai).active);
    nova.Id = 3815;
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
    assert(bot.x == 15 && !Blackfathom::CollectHazard(&ai).active && !escape.Execute({}));
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
    caster.currentSpell[CURRENT_GENERIC_SPELL] = &cast;
    bot.map = 0;
    assert(!Blackfathom::CollectHazard(&ai).active);
    bot.map = 48;
    sPlayerbotAIConfig.EnableBlackfathomMechanics = false;
    assert(!Blackfathom::InHazard(&ai) && !Blackfathom::CollectHazard(&ai).active);
    std::cout << "Blackfathom source execution scenarios passed\n";
}
