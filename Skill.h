#pragma once
#include "Common.h"
#include <cliext/map>

ref struct Skill;
ref struct Query;
ref struct SkillTag;

ref struct Ability
{
	typedef System::Collections::Generic::Dictionary< int, Skill^ > SkillMap_t;
	System::String^ name;
	List_t< SkillTag^ > tags;
	bool relevant, auto_guard, related;
	unsigned static_index, ping_index;
	int order;
	SkillMap_t skills;

	Skill^ GetSkill( const int amount );

	static Ability^ torso_inc;
	static List_t< Ability^ > static_abilities, ordered_abilities; //for sorting alphabetically
	static Map_t< System::String^, Ability^ > static_ability_map;
	static Map_t< System::String^, Ability^ > charm_ability_map; //Japanese-only, for importing from MasaxMH
	static Ability^ FindAbility( System::String^ name );
	static Ability^ FindCharmAbility( System::String^ name );
	static void UpdateOrdering();
};

ref struct Skill
{
	System::String^ name;
	int points_required, order, static_index, ping_index;
	Ability^ ability;

	static void Load( System::String^ filename );
	static void LoadLanguage( System::String^ filename );
	static List_t< Skill^ > static_skills, ordered_skills; //for sorting alphabetically
	static Map_t< System::String^, Skill^ > static_skill_map;
	static Skill^ FindSkill( System::String^ name );
	static void UpdateOrdering();
};

ref struct AbilityPair
{
	Ability^ ability;
	int amount;

	AbilityPair( Ability^ ab, const int am )
		: ability( ab ), amount( am ) {}
};

void FindRelatedSkills( List_t< System::Windows::Forms::ComboBox^ >% skills, List_t< Map_t< unsigned, unsigned >^ >% index_maps );