
UntypedValue* Context::Create(std::string const& name)
{
    RazorfenKraulCharmValueContext factory;
    auto i = factory.creators.find(name);
    return i == factory.creators.end() ? nullptr : i->second(ai);
}
int main()
{
    Player bot;
    PlayerbotAI ai(&bot);
    RazorfenKraulCharmActionContext actions;
    RazorfenKraulCharmTriggerContext triggers;
    std::vector<TriggerNode*> nodes;
    std::vector<Multiplier*> multipliers;
    RazorfenKraul::InitCharmTriggers(nodes);
    RazorfenKraul::InitCharmMultipliers(&ai, multipliers);
    sharedCharm.active = true;
    for (auto* node : nodes)
    {
        std::unique_ptr<Trigger> trigger(triggers.creators.at(node->name)(&ai));
        std::unique_ptr<Action> action(actions.creators.at(node->actions[0].name)(&ai));
        assert(action->getName() == node->actions[0].name);
        assert(!trigger->IsActive() && !action->Execute({}));
        assert(sharedCharm.scans == 0 && sharedCharm.executions == 0);
        sPlayerbotAIConfig.EnableRazorfenKraulMechanics = true;
        assert(trigger->IsActive());
        int scans = sharedCharm.scans;
        assert(trigger->IsActive() && sharedCharm.scans == scans);
        assert(action->Execute({}));
        sharedCharm.active = false;
        assert(!action->Execute({}));
        sharedCharm.active = true;
        bot.map = 0;
        assert(!trigger->IsActive() && !action->Execute({}));
        bot.map = 47;
        sPlayerbotAIConfig.EnableRazorfenKraulMechanics = false;
        sharedCharm.scans = sharedCharm.executions = 0;
    }
    Action spell(&ai, "class spell");
    for (auto* multiplier : multipliers)
    {
        assert(multiplier->GetValue(&spell) == 1);
        sPlayerbotAIConfig.EnableRazorfenKraulMechanics = true;
        assert(multiplier->GetValue(&spell) == 0);
        bot.map = 0;
        assert(multiplier->GetValue(&spell) == 1);
        bot.map = 47;
        sPlayerbotAIConfig.EnableRazorfenKraulMechanics = false;
        delete multiplier;
    }
    for (auto* node : nodes)
        delete node;
    std::cout << "RazorfenKraul shared-charm adapter scenarios passed\n";
}
