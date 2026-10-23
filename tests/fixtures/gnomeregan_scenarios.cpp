/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

UntypedValue* Context::Create(std::string const& name)
{
    GnomereganValueContext creators;
    auto i = creators.creators.find(name);
    return i == creators.creators.end() ? nullptr : i->second(ai);
}
int main()
{
    Player bot;
    bot.map = 90;
    bot.ranged = true;
    PlayerbotAI ai(&bot);
    playerAIs[&bot] = &ai;
    Creature runner(2);
    runner.map = 90;
    runner.entry = 6206;
    runner.motion.type = ASSISTANCE_MOTION_TYPE;
    runner.reaction = REACT_PASSIVE;
    Creature alert(3);
    alert.map = 90;
    alert.entry = 7849;
    ai.units[runner.guid] = &runner;
    ai.units[alert.guid] = &alert;
    ai.context.GetValue<GuidVector>("attackers")->Set({runner.guid, alert.guid});
    assert(!Gnomeregan::FindFocus(&ai));
    sPlayerbotAIConfig.EnableGnomereganMechanics = true;
    assert(Gnomeregan::FindFocus(&ai) == alert.guid);
    alert.cc = true;
    assert(Gnomeregan::FindFocus(&ai) == runner.guid);
    GnomereganFocusAction focus(&ai);
    assert(focus.Execute({}));
    runner.markedCc = true;
    assert(!focus.Execute({}));
    runner.markedCc = false;
    bot.tank = true;
    assert(!Gnomeregan::FindFocus(&ai));
    bot.tank = false;
    Group group;
    bot.group = &group;
    group.skull = alert.guid;
    assert(Gnomeregan::HasExplicitTarget(&ai));
    alert.alive = false;
    assert(!Gnomeregan::HasExplicitTarget(&ai));
    GnomereganFocusMultiplier multiplier(&ai);
    AttackMyTargetAction manual(&ai);
    AttackDuelOpponentAction duel(&ai);
    RcLeaveCharmedTargetAction cleanup(&ai);
    assert(multiplier.GetValue(&manual) == 1 && multiplier.GetValue(&duel) == 1 && multiplier.GetValue(&cleanup) == 1);
    Creature mine(4);
    mine.map = 90;
    mine.entry = 8035;
    mine.x = 0;
    ai.units[mine.guid] = &mine;
    ai.context.GetValue<GuidVector>("possible targets no los")->Set({mine.guid});
    spellManager.spells[4043].Effects[0].radius = 10;
    auto hazards = Gnomeregan::CollectHazards(&ai);
    assert(hazards.size() == 1);
    Position from{2, 0, 0}, out{7, 0, 0}, bad{1, 0, 0};
    assert(Gnomeregan::SafeProgress(hazards, from, out));
    assert(!Gnomeregan::SafeProgress(hazards, from, bad));
    assert(!Gnomeregan::SafeProgress(hazards, Position{-20, 0, 0}, Position{20, 0, 0}, false));
    assert(Gnomeregan::SafeProgress(hazards, Position{-20, 20, 0}, Position{20, 20, 0}, false));
    bot.x = 2;
    GnomereganEscapeAction escape(&ai);
    assert(escape.Execute({}));
    assert(bot.castStops == 1);
    assert(escape.Execute({}));
    assert(ai.moves == 1);
    GnomereganMovementMultiplier movement(&ai);
    MovementAction approach(&ai);
    assert(movement.GetValue(&approach) == 0.0f);
    assert(movement.GetValue(&escape) == 1.0f);
    assert(movement.GetValue(&cleanup) == 1.0f);
    CastSpellAction healing(&ai, "heal");
    assert(movement.GetValue(&healing) == 1.0f);
    bot.StopMoving();
    ai.canMove = false;
    clockNow += 600;
    assert(!escape.Execute({}));
    assert(bot.castStops == 1);
    ai.canMove = true;
    ai.pathRejected = true;
    assert(!escape.Execute({}));
    assert(bot.castStops == 1);
    ai.pathRejected = false;
    clockNow += 600;
    bot.movementCast = true;
    assert(escape.Execute({}));
    assert(bot.deferredMove);
    bot.MovementUpdate();
    assert(!bot.movespline->Finalized());
    // A safe healer may approach an ally while the hostile target remains in the mine.
    bot.x = 20;
    ai.context.GetValue<Unit*>("current target")->Set(&mine);
    Player ally(9);
    ally.map = 90;
    ally.x = 25;
    ai.context.GetValue<Unit*>("party member to heal")->Set(&ally);
    ai.context.GetValue<Unit*>("party member to resurrect")->Set(&ally);
    clockNow += 600;
    ReachPartyMemberToHealAction reachHeal(&ai);
    ReachPartyMemberToResurrectAction reachResurrect(&ai);
    TankFaceAction tankFace(&ai);
    SetBehindTargetAction behind(&ai);
    assert(movement.GetValue(&reachHeal) == 1.0f);
    assert(movement.GetValue(&reachResurrect) == 1.0f);
    assert(movement.GetValue(&approach) == 0.0f);
    assert(movement.GetValue(&tankFace) == 0.0f);
    assert(movement.GetValue(&behind) == 0.0f);
    bot.x = 2;
    clockNow += 600;
    assert(movement.GetValue(&reachHeal) == 0.0f);
    assert(movement.GetValue(&escape) == 1.0f);
    auto overlap = hazards;
    overlap.push_back({Position{6, 0, 0}, 13});
    assert(!Gnomeregan::SafeProgress(overlap, Position{2, 0, 0}, Position{7, 0, 0}));
    assert(Gnomeregan::SafeProgress(overlap, Position{2, 0, 0}, Position{2, 5, 0}));
    mine.alive = false;
    assert(Gnomeregan::CollectHazards(&ai).empty());
    GameObject button, face;
    button.guid = 11;
    button.map = 90;
    button.entry = 142214;
    face.guid = 12;
    face.map = 90;
    face.entry = 142211;
    face.state = GO_STATE_ACTIVE;
    ai.objects[button.guid] = &button;
    ai.objects[face.guid] = &face;
    ai.context.GetValue<GuidVector>("nearest game objects no los")->Set({button.guid, face.guid});
    assert(Gnomeregan::FindButton(&ai) == button.guid);
    face.state = GO_STATE_READY;
    assert(!Gnomeregan::GetButton(&ai, button.guid));
    face.state = GO_STATE_ACTIVE;
    bot.heal = true;
    assert(!Gnomeregan::FindButton(&ai));
    bot.heal = false;
    for (auto const [buttonEntry, faceEntry] : std::vector<std::pair<uint32, uint32>>{{142214, 142211},
                                                                                      {142215, 142210},
                                                                                      {142216, 142209},
                                                                                      {142217, 142208},
                                                                                      {142218, 142213},
                                                                                      {142219, 142212}})
    {
        button.entry = buttonEntry;
        face.entry = faceEntry;
        assert(Gnomeregan::GetButton(&ai, button.guid) == &button);
    }
    alert.cc = true;
    mine.alive = false;
    for (uint32 entry : {6206u, 6207u, 6211u, 6391u, 6392u, 6407u, 9676u})
    {
        runner.entry = entry;
        assert(Gnomeregan::FindFocus(&ai) == runner.guid);
    }
    sPlayerbotAIConfig.EnableGnomereganMechanics = false;
    assert(!Gnomeregan::GetButton(&ai, button.guid));
    assert(Gnomeregan::CollectHazards(&ai).empty());
    std::cout << "Gnomeregan source scenarios passed\n";
}
