
UntypedValue* Context::Create(std::string const& name)
{
    RazorfenKraulValueContext focus;
    RazorfenKraulAvoidanceValueContext avoidance;
    RazorfenKraulCharmValueContext charm;
    for (NamedObjectContext<UntypedValue>* factory : {static_cast<NamedObjectContext<UntypedValue>*>(&focus),
                                                      static_cast<NamedObjectContext<UntypedValue>*>(&avoidance),
                                                      static_cast<NamedObjectContext<UntypedValue>*>(&charm)})
    {
        auto i = factory->creators.find(name);
        if (i != factory->creators.end())
            return i->second(ai);
    }
    return nullptr;
}
int main()
{
    Player bot;
    bot.map = 47;
    PlayerbotAI ai(&bot);
    RazorfenKraulStrategy strategy(&ai);
    std::vector<TriggerNode*> triggers;
    std::vector<Multiplier*> multipliers;
    strategy.InitTriggers(triggers);
    strategy.InitMultipliers(multipliers);
    assert(triggers.size() == 5 && multipliers.size() == 5);
    Creature runner(11), kelris(12);
    runner.entry = 4435;
    runner.map = kelris.map = 47;
    runner.x = 3;
    runner.motion.type = FLEEING_MOTION_TYPE;
    ai.units[runner.guid] = &runner;
    ai.units[kelris.guid] = &kelris;
    ai.context.GetValue<GuidVector>("attackers")->Set({runner.guid, kelris.guid});
    SpellInfo sleep;
    sleep.Id = 8259;
    Spell cast{&sleep, {}};
    kelris.currentSpell[CURRENT_GENERIC_SPELL] = &cast;
    RazorfenKraulFocusMultiplier focus(&ai);
    RazorfenKraulHazardMultiplier hazard(&ai);
    RazorfenKraulCharmAction<RcLeaveCharmedTargetAction> cleanup(&ai, "razorfen kraul charm target");
    AttackAction ordinary(&ai, "attack anything");
    assert(!RazorfenKraul::GetFocus(&ai) && !RazorfenKraul::InHazard(&ai));
    sPlayerbotAIConfig.EnableRazorfenKraulMechanics = true;
    assert(RazorfenKraul::GetFocus(&ai) == &runner && RazorfenKraul::InHazard(&ai));
    assert(focus.GetValue(&ordinary) == 0 && hazard.GetValue(&ordinary) == 0);
    assert(focus.GetValue(&cleanup) == 1 && hazard.GetValue(&cleanup) == 1);
    sharedCharm.active = true;
    assert(cleanup.Execute({}) && sharedCharm.executions == 1);
    for (auto* trigger : triggers)
        delete trigger;
    for (auto* multiplier : multipliers)
        delete multiplier;
    std::cout << "RazorfenKraul complete strategy overlap scenarios passed\n";
}
