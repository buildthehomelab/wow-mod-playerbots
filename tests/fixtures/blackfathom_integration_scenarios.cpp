
UntypedValue* Context::Create(std::string const& name)
{
    BlackfathomValueContext focus;
    BlackfathomAvoidanceValueContext avoidance;
    BlackfathomCharmValueContext charm;
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
    bot.map = 48;
    PlayerbotAI ai(&bot);
    BlackfathomStrategy strategy(&ai);
    std::vector<TriggerNode*> triggers;
    std::vector<Multiplier*> multipliers;
    strategy.InitTriggers(triggers);
    strategy.InitMultipliers(multipliers);
    assert(triggers.size() == 5 && multipliers.size() == 5);
    Creature runner(11), kelris(12);
    runner.entry = 4805;
    runner.map = kelris.map = 48;
    runner.x = 3;
    runner.motion.type = FLEEING_MOTION_TYPE;
    ai.units[runner.guid] = &runner;
    ai.units[kelris.guid] = &kelris;
    ai.context.GetValue<GuidVector>("attackers")->Set({runner.guid, kelris.guid});
    SpellInfo sleep;
    sleep.Id = 8399;
    Spell cast{&sleep, {}};
    kelris.currentSpell[CURRENT_GENERIC_SPELL] = &cast;
    BlackfathomFocusMultiplier focus(&ai);
    BlackfathomHazardMultiplier hazard(&ai);
    BlackfathomCharmAction<RcLeaveCharmedTargetAction> cleanup(&ai, "blackfathom charm target");
    AttackAction ordinary(&ai, "attack anything");
    assert(!Blackfathom::GetFocus(&ai) && !Blackfathom::InHazard(&ai));
    sPlayerbotAIConfig.EnableBlackfathomMechanics = true;
    assert(Blackfathom::GetFocus(&ai) == &runner && Blackfathom::InHazard(&ai));
    assert(focus.GetValue(&ordinary) == 0 && hazard.GetValue(&ordinary) == 0);
    assert(focus.GetValue(&cleanup) == 1 && hazard.GetValue(&cleanup) == 1);
    sharedCharm.active = true;
    assert(cleanup.Execute({}) && sharedCharm.executions == 1);
    for (auto* trigger : triggers)
        delete trigger;
    for (auto* multiplier : multipliers)
        delete multiplier;
    std::cout << "Blackfathom complete strategy overlap scenarios passed\n";
}
