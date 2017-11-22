#pragma once
#include "Common.h"
#include "Skill.h"

ref struct Query;

ref struct Armor : public AdvancedSearchOptions
{
	enum class ArmorType { HEAD = 0, BODY, ARMS, WAIST, LEGS, NumArmorTypes };
	System::String^ name;
	unsigned hr, elder_star, num_slots, defence, max_defence, rarity, difficulty, index, family, ping_index;
	int ice_res, water_res, fire_res, thunder_res, dragon_res;
	Gender gender;
	HunterType type;
	bool torso_inc, no_skills, is_piercing, is_event, jap_only;
	Ability^ danger;
	List_t< AbilityPair^ > abilities;
	List_t< MaterialComponent^ > components;

	bool IsBetterThan( Armor^ other, List_t< Ability^ >^ rel_abilities );
	int GetSkillAt( Ability^ ability );

	bool MatchesQuery( Query^ query, List_t< Ability^ >^ danger_skills, const unsigned max_slots );

	bool ContainsAnyAbility( List_t< Ability^ >% to_search );

	static void Load( System::String^ filename, ArmorType armor_type );
	static void LoadLanguage( System::String^ filename, ArmorType armor_type );
	static array< List_t< Armor^ >^ >^ static_armors;
	static Map_t< System::String^, Armor^ > static_armor_map;
	static Armor^ FindArmor( System::String^ name );
};

ref struct Family
{
	static unsigned count = 0;
	static Map_t< System::String^, unsigned > map;
	static List_t< List_t< Armor^ >^ > families;
};

ref struct Charm
{
	List_t< AbilityPair^ > abilities;
	unsigned num_slots, remove_ability2_when_ability1_reduced_to;
	bool custom, optimal, hacked;

	Charm() : num_slots( 0 ), custom( false ), optimal( false ), hacked( false ), remove_ability2_when_ability1_reduced_to( 0 ) {}
	Charm( Charm^ other );
	Charm( const unsigned num_slots );
	Charm( System::String^ name );

	bool StrictlyBetterThan( Charm^ other );
	bool BasicallyTheSameAs( Charm^ other );
	bool operator == ( Charm^ other );

	System::String^ GetName();
	unsigned GetHash();

	static unsigned HashFunction( const unsigned num_slots, Ability^ a1, const int p1, Ability^ a2, const int p2 );
	static void AddToOptimalList( List_t< Charm^ >% lst, Charm^ new_charm );
};

int CompareCharms( Charm^ a, Charm^ b );
int CompareCharmsAlphabetically( Charm^ a, Charm^ b );
