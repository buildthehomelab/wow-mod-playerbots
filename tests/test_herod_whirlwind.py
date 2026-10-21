# Focused execution checks with minimal engine doubles; not a core build or game simulation.
from pathlib import Path
import subprocess,tempfile
r=Path(__file__).resolve().parents[1];d=r/'src/Ai/Dungeon/SM'
def source(n):
 return '\n'.join(l for l in (d/n).read_text().splitlines() if not l.startswith('#include') and not l.startswith('#endif') and not l.startswith('#ifndef') and not l.startswith('#define'))
stub=r'''
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
using uint32=uint32_t;
struct Unit { bool alive=true,aura=false,cast=false; uint32 map=189; float distance=0; bool IsAlive(){return alive;} uint32 GetMapId(){return map;} bool HasAura(uint32){return aura;} bool FindCurrentSpellBySpellId(uint32){return cast;} };
struct Player:Unit {int stops=0; void CastStop(){++stops;} float GetExactDist2d(Unit* u){return u->distance;} };
struct LastMovement {bool waiting=true; int clears=0; void clear(){waiting=false;++clears;}};
struct PlayerbotAI {Player* bot; Unit* target=nullptr; LastMovement movement;};
struct Event{};
struct Action {Player* bot; PlayerbotAI* botAI; Action(PlayerbotAI* a,std::string=""):bot(a->bot),botAI(a){} virtual ~Action(){} virtual bool isUseful(){return true;} virtual bool Execute(Event){return true;} };
struct MovementAction:Action {using Action::Action;float moved=0;bool MoveAway(Unit*,float d){if(botAI->movement.waiting)return false;moved=d;return true;} };
struct CastSpellAction:Action{using Action::Action;};
struct CastReachTargetSpellAction:Action{using Action::Action;};
struct Trigger:Action{using Action::Action;virtual bool IsActive(){return false;}};
struct Multiplier:Action{using Action::Action;virtual float GetValue(Action*){return 1;}};
#define AI_VALUE(type,name) (botAI->movement)
#define AI_VALUE2(type,name,qualifier) (botAI->target)
'''
test=r'''
int main(){
 Player bot;PlayerbotAI ai{&bot,nullptr,{}};Unit herod;ai.target=&herod;
 HerodWhirlwindTrigger trigger(&ai);HerodWhirlwindEscapeAction escape(&ai);HerodWhirlwindMultiplier multiplier(&ai);
 MovementAction chase(&ai);CastReachTargetSpellAction charge(&ai);CastSpellAction heal(&ai);Action other(&ai);
 assert(!trigger.IsActive());assert(!escape.Execute({}));assert(multiplier.GetValue(&chase)==1);
 herod.aura=true;herod.distance=3;
 assert(trigger.IsActive());assert(escape.isUseful());assert(escape.Execute({}));assert(escape.moved==9);assert(ai.movement.clears==1);assert(bot.stops==1);
 assert(multiplier.GetValue(&escape)==1);assert(multiplier.GetValue(&chase)==0);assert(multiplier.GetValue(&charge)==0);assert(multiplier.GetValue(&heal)==0);assert(multiplier.GetValue(&other)==1);
 herod.distance=12;assert(!trigger.IsActive());assert(!escape.isUseful());assert(!escape.Execute({}));assert(multiplier.GetValue(&chase)==0);assert(multiplier.GetValue(&heal)==1);
 herod.aura=false;herod.cast=true;herod.distance=4;assert(trigger.IsActive());
 herod.cast=false;assert(!trigger.IsActive());assert(multiplier.GetValue(&chase)==1);assert(multiplier.GetValue(&heal)==1);
 herod.aura=true;herod.alive=false;assert(!trigger.IsActive());assert(!escape.Execute({}));assert(multiplier.GetValue(&chase)==1);
 herod.alive=true;herod.map=1;assert(!trigger.IsActive());assert(multiplier.GetValue(&chase)==1);
 ai.target=nullptr;assert(!trigger.IsActive());assert(!escape.isUseful());assert(!escape.Execute({}));assert(multiplier.GetValue(&chase)==1);
 std::cout<<"Herod reaction boundary/state checks passed\n";
}
'''
with tempfile.TemporaryDirectory() as temp:
 p=Path(temp);cpp=p/'test.cpp';cpp.write_text(stub+'\n'+ '\n'.join(source(n) for n in ['SMShared.h','SMActions.h','SMTriggers.h','SMMultipliers.h','SMActions.cpp','SMTriggers.cpp','SMMultipliers.cpp'])+test)
 subprocess.run(['clang++','-std=c++20','-Wall','-Wextra','-Werror',str(cpp),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
