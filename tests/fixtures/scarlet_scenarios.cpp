UntypedValue* Context::Create(std::string const& name)
{
    VanillaDungeonScarletMonasteryValueContext factory;
    auto entry = factory.creators.find(name);
    return entry == factory.creators.end() ? nullptr : entry->second(ai);
}

int main()
{
    using namespace SMShared;
    Player bot(1), ally(2), outsider(3);
    PlayerbotAI ai(&bot);
    Group group;
    bot.group = &group;
    ally.group = &group;
    GroupReference allyRef{&ally, nullptr}, botRef{&bot, &allyRef};
    group.first = &botRef;
    Creature herod(10), doan(11), thalnos(12), ironspine(13), whitemane(14), healer(15), other(16);
    herod.entry = 3975;
    doan.entry = 6487;
    thalnos.entry = 4543;
    ironspine.entry = 6489;
    whitemane.entry = 3977;
    healer.entry = 4303;
    bot.x = 3;
    other.x = 5;
    healer.x = 6;
    whitemane.x = 8;
    ai.units = {
        {bot.guid, &bot},         {ally.guid, &ally},           {herod.guid, &herod},         {doan.guid, &doan},
        {thalnos.guid, &thalnos}, {ironspine.guid, &ironspine}, {whitemane.guid, &whitemane}, {healer.guid, &healer},
        {other.guid, &other}};
    ThreatReference rh{&herod}, rd{&doan}, rt{&thalnos}, ri{&ironspine};
    bot.threat.refs = {{herod.guid, &rh}, {doan.guid, &rd}, {thalnos.guid, &rt}, {ironspine.guid, &ri}};
    auto* context = &ai.context;
    HerodWhirlwindTrigger trigger(&ai);
    HerodWhirlwindEscapeAction escape(&ai);
    HerodWhirlwindMultiplier multiplier(&ai);
    MovementAction chase(&ai);
    CastReachTargetSpellAction charge(&ai);
    CastSpellAction heal(&ai, "heal");
    Action otherAction(&ai);
    assert(!trigger.IsActive());
    int reads = bot.threat.reads;
    for (int tick = 0; tick < 10; ++tick)
    {
        trigger.IsActive();
        GetEncounterTarget(&ai, DOAN);
        GetEncounterTarget(&ai, THALNOS);
    }
    assert(bot.threat.reads == reads);  // Four encounters share one time-bounded GUID cache.
    herod.auras.insert(SPELL_HEROD_WHIRLWIND);
    assert(trigger.IsActive());
    assert(escape.isUseful());
    assert(escape.Execute({}));
    assert(ai.moves == 1 && ai.movement.clears == 1 && bot.castStops == 1);
    clockNow += 100;
    assert(escape.Execute({}));
    assert(ai.moves == 1 && ai.movement.clears == 1);  // Preserve spline.
    bot.spline.destination.x += 1;
    clockNow += 600;
    assert(escape.Execute({}));
    assert(ai.moves == 2);  // A replacement move is preempted.
    assert(multiplier.GetValue(&escape) == 1 && multiplier.GetValue(&chase) == 0 && multiplier.GetValue(&charge) == 0);
    assert(multiplier.GetValue(&heal) == 0 && multiplier.GetValue(&otherAction) == 1);
    bot.x = 12;
    assert(!trigger.IsActive());
    assert(!escape.isUseful());
    assert(!escape.Execute({}));
    assert(multiplier.GetValue(&chase) == 0 && multiplier.GetValue(&heal) == 1);
    herod.auras.clear();
    assert(multiplier.GetValue(&chase) == 1);
    herod.casts.insert(SPELL_HEROD_WHIRLWIND);
    bot.x = 3;
    assert(trigger.IsActive());
    herod.alive = false;
    assert(!trigger.IsActive());
    herod.alive = true;
    herod.map = 1;
    assert(!trigger.IsActive());
    herod.map = 189;
    herod.casts.clear();
    bot.moving = false;
    bot.spline.done = true;
    ai.moveBlocked = true;
    clockNow += 600;
    herod.auras.insert(SPELL_HEROD_WHIRLWIND);
    assert(!escape.Execute({}));
    int clears = ai.movement.clears;
    clockNow += 100;
    assert(!escape.Execute({}));
    assert(ai.movement.clears == clears);  // Rate-limit blocked paths.
    ai.moveBlocked = false;
    herod.auras.clear();
    for (auto encounter : {DOAN, THALNOS, IRONSPINE})
    {
        Unit* boss = GetEncounterTarget(&ai, encounter);
        assert(boss);
        uint32 spell = encounter == DOAN      ? SPELL_DETONATION
                       : encounter == THALNOS ? SPELL_FIRE_NOVA
                                              : SPELL_POISON_CLOUD;
        boss->casts.insert(spell);
        assert(IsHazardActive(boss, encounter));
        ScarletHazardTrigger hazard(&ai, "hazard", encounter);
        ScarletAvoidanceAction avoid(&ai, "avoid", encounter);
        assert(hazard.IsActive());
        clockNow += 600;
        bot.moving = false;
        bot.spline.done = true;
        assert(avoid.Execute({}));
        int moves = ai.moves;
        clockNow += 100;
        assert(avoid.Execute({}));
        assert(ai.moves == moves);
        boss->casts.clear();
        assert(!hazard.IsActive());
    }
    doan.auras.insert(SPELL_ARCANE_BUBBLE);
    assert(IsDetonationImminent(&doan));
    doan.auras.clear();
    bot.moving = false;
    bot.spline.done = true;
    context->GetValue<GuidVector>("attackers")->Set({other.guid, healer.guid, whitemane.guid});
    context->GetValue<Unit*>("current target")->Set(&other);
    assert(FindFocusTarget(&ai) == whitemane.guid);
    ScarletFocusAction focus(&ai);
    assert(focus.isUseful());
    whitemane.markedCc = true;
    assert(!focus.isUseful());
    assert(!focus.Execute({}));  // Mark after cached choice.
    whitemane.markedCc = false;
    whitemane.los = false;
    assert(!focus.isUseful());
    whitemane.los = true;
    whitemane.x = 50;
    assert(!focus.Execute({}));
    whitemane.x = 8;
    assert(focus.Execute({}));
    assert(bot.victim == &whitemane);
    whitemane.reaction = REACT_PASSIVE;
    assert(!GetFocusTarget(&ai));
    whitemane.reaction = REACT_AGGRESSIVE;
    bot.tank = true;
    assert(!GetFocusTarget(&ai));
    bot.tank = false;
    bot.heal = true;
    assert(!GetFocusTarget(&ai));
    bot.heal = false;
    group.skull = other.guid;
    assert(!GetFocusTarget(&ai));
    group.skull = {};
    context->GetValue<GuidVector>("prioritized targets")->Set({other.guid});
    assert(!GetFocusTarget(&ai));
    context->GetValue<GuidVector>("prioritized targets")->Set({});
    whitemane.valid = false;
    assert(FindFocusTarget(&ai) == healer.guid);
    whitemane.valid = true;
    healer.cc = true;
    whitemane.valid = false;
    assert(!FindFocusTarget(&ai));
    healer.cc = false;
    whitemane.valid = true;

    context->GetValue<uint32>("spell id", "dispel magic")->Set(527);
    bot.spells.insert(527);
    spellManager.spells[527] = {false, false};
    ally.auras.insert(SPELL_DOMINATE_MIND);
    assert(IsDominatedPartyMember(&bot, &ally));
    assert(!IsDominatedPartyMember(&bot, &outsider));
    assert(CanDispelDominate(&ai, &ally, 527));
    ally.dispelEligible = false;
    assert(!CanDispelDominate(&ai, &ally, 527));
    ally.dispelEligible = true;
    clockNow += 600;
    assert(FindDominatedAlly(&ai) == ally.guid);
    ScarletDispelDominateAction dispel(&ai);
    assert(dispel.isUseful());
    assert(dispel.isPossible());
    assert(dispel.Execute({}));
    assert(ai.castId == 527 && ai.castTarget == &ally);
    ai.canCast = false;
    assert(!dispel.isPossible());
    ai.canCast = true;
    ally.x = 100;
    assert(!dispel.isPossible());
    ally.x = 0;
    Pet pet;
    bot.pet = &pet;
    bot.victim = &ally;
    pet.victim = &ally;
    context->GetValue<Unit*>("current target")->Set(&ally);
    ScarletProtectAllyTrigger protectTrigger(&ai);
    ScarletProtectAllyAction protect(&ai);
    assert(protectTrigger.IsActive());
    assert(protect.Execute({}));
    assert(pet.castStops == 1);
    assert(!bot.victim && !pet.victim && !context->GetValue<Unit*>("current target")->Get());
    SpellInfo harmfulInfo{false, false, false}, dispelInfo{false, false, true}, areaInfo{false, true, false};
    Spell harmfulCast{&harmfulInfo, {ally.guid}}, dispelCast{&dispelInfo, {ally.guid}}, areaCast{&areaInfo, {}};
    bot.currentSpell[0] = &harmfulCast;
    assert(IsCastingAtDominatedAlly(&ai, &bot));
    int castStops = bot.castStops;
    assert(protectTrigger.IsActive());
    assert(protect.Execute({}));
    assert(bot.castStops == castStops + 1);
    bot.currentSpell[0] = &dispelCast;
    assert(!IsCastingAtDominatedAlly(&ai, &bot));
    pet.currentSpell[0] = &harmfulCast;
    assert(protectTrigger.IsActive());
    assert(protect.Execute({}));
    assert(pet.castStops == 2);
    pet.currentSpell[0] = nullptr;
    bot.currentSpell[0] = &areaCast;
    assert(IsCastingAtDominatedAlly(&ai, &bot));
    bot.currentSpell[0] = nullptr;
    ScarletCharmSafetyMultiplier safety(&ai);
    CastSpellAction aoe(&ai, "fire nova"), single(&ai, "shadow bolt"), positive(&ai, "heal");
    context->GetValue<uint32>("spell id", "fire nova")->Set(100);
    spellManager.spells[100] = {false, true};
    context->GetValue<uint32>("spell id", "shadow bolt")->Set(101);
    spellManager.spells[101] = {false, false};
    context->GetValue<uint32>("spell id", "heal")->Set(102);
    spellManager.spells[102] = {true, true};
    context->GetValue<Unit*>("current target")->Set(&ally);
    assert(safety.GetValue(&aoe) == 0 && safety.GetValue(&single) == 0 && safety.GetValue(&positive) == 1);
    context->GetValue<Unit*>("current target")->Set(&whitemane);
    assert(safety.GetValue(&single) == 1);
    ally.auras.clear();
    clockNow += 600;
    assert(safety.GetValue(&aoe) == 1);
    assert(!dispel.isUseful());

    DynamicObject ground;
    ground.radius = 5;
    Aura groundAura;
    groundAura.area = &ground;
    context->GetValue<Aura*>("area debuff")->Set(&groundAura);
    bot.x = 3;
    bot.moving = false;
    bot.spline.done = true;
    clockNow += 1200;
    ScarletGroundDamageTrigger groundTrigger(&ai);
    ScarletGroundEscapeAction groundEscape(&ai);
    ScarletGroundDamageMultiplier groundMultiplier(&ai);
    assert(groundEscape.getName() == "scarlet ground damage");
    assert(groundTrigger.IsActive());
    assert(groundEscape.isUseful());
    assert(groundEscape.Execute({}));
    int groundMoves = ai.moves;
    clockNow += 100;
    assert(groundEscape.Execute({}));
    assert(ai.moves == groundMoves);
    assert(groundMultiplier.GetValue(&groundEscape) == 1 && groundMultiplier.GetValue(&heal) == 0 &&
           groundMultiplier.GetValue(&chase) == 0);
    groundAura.removed = true;
    assert(!IsInGroundDamage(&ai));
    groundAura.removed = false;
    sPlayerbotAIConfig.AoeAvoidSpellWhitelist.insert(8422);
    assert(!IsInGroundDamage(&ai));
    sPlayerbotAIConfig.AoeAvoidSpellWhitelist.clear();
    ground.radius = 100;
    assert(!IsInGroundDamage(&ai));
    ground.radius = 5;
    bot.x = 8;
    clockNow += 600;
    assert(!groundTrigger.IsActive());
    assert(groundMultiplier.GetValue(&chase) == 1);
    context->GetValue<Aura*>("area debuff")->Set(nullptr);
    VanillaDungeonScarletMonasteryStrategy strategy(&ai);
    auto checkConfig = [&](bool legacy, bool full, size_t triggerCount, size_t multiplierCount)
    {
        sPlayerbotAIConfig = {legacy, full};
        std::vector<TriggerNode*> triggers;
        std::vector<Multiplier*> multipliers;
        strategy.InitTriggers(triggers);
        strategy.InitMultipliers(multipliers);
        assert(triggers.size() == triggerCount && multipliers.size() == multiplierCount);
        VanillaDungeonScarletMonasteryActionContext actions;
        VanillaDungeonScarletMonasteryTriggerContext triggerFactory;
        for (auto t : triggers)
        {
            assert(triggerFactory.creators.contains(t->name));
            for (auto const& a : t->actions)
                assert(actions.creators.contains(a.name));
            delete t;
        }
        for (auto m : multipliers)
            delete m;
    };
    checkConfig(false, false, 1, 1);
    checkConfig(true, false, 2, 2);
    checkConfig(false, true, 8, 7);
    checkConfig(true, true, 8, 7);
    ally.auras.insert(SPELL_DOMINATE_MIND);
    clockNow += 600;
    GuidSet exclusions;
    strategy.AppendTargetExclusions(exclusions, TargetValueExclusionType::TankTarget);
    assert(exclusions.contains(ally.guid));
    ally.auras.clear();
    exclusions.clear();
    strategy.AppendTargetExclusions(exclusions, TargetValueExclusionType::Aoe);
    assert(exclusions.empty());
    bot.map = 1;
    assert(!GetEncounterTarget(&ai, HEROD));
    assert(!GetFocusTarget(&ai));
    assert(!FindDominatedAlly(&ai));
    std::cout << "Scarlet Monastery: cache, escape, focus, charm, protection, gates and creator checks passed\n";
}
