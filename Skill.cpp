#include "stdafx.h"
#include <fstream>
#include "Skill.h"
#include "Solution.h"
#include "LoadedData.h"

using namespace System;

int CompareAbilities( Ability^ a, Ability^ b )
{
	return String::Compare( a->name, b->name );
}

int CompareSkills( Skill^ a, Skill^ b )
{
	return String::Compare( a->name, b->name );
}

Skill^ Ability::GetSkill( const int amount )
{
	if( amount == 0 ) return nullptr;
	int best = 0;
	SkillMap_t::Enumerator iter = skills.GetEnumerator();
	if( amount > 0 )
	{
		while( iter.MoveNext() )
			if( iter.Current.Key <= amount && iter.Current.Key > best )
				best = iter.Current.Key;
	}
	else
	{
		while( iter.MoveNext() )
			if( iter.Current.Key >= amount && iter.Current.Key < best )
				best = iter.Current.Key;
	}
	if( best == 0 ) return nullptr;
	Assert( skills.ContainsKey( best ), L"Skill dictionary doesn't contain this skill?" );
	return skills[ best ];
}
/*
bool Ability::MatchesQuery( Query^ query )
{
	SkillMap_t::Enumerator iter = skills.GetEnumerator();
	while( iter.MoveNext() )
	{
		if( iter.Current.Key > 0 && Utility::Contains( %query->skills, iter.Current.Value ) )
			return true;
	}
	return false;
}*/

Ability^ Ability::FindAbility( System::String^ name )
{
	if( name && static_ability_map.ContainsKey( name ) )
		return static_ability_map[ name ];
	return nullptr;
}

Ability^ Ability::FindCharmAbility( System::String^ name )
{
	if( name && charm_ability_map.ContainsKey( name ) )
		return charm_ability_map[ name ];
	return nullptr;
}

void Ability::UpdateOrdering()
{
	ordered_abilities.Clear();
	ordered_abilities.AddRange( %static_abilities );
	ordered_abilities.Sort( gcnew Comparison< Ability^ >( CompareAbilities ) );
	for( int i = 0; i < ordered_abilities.Count; ++i )
		ordered_abilities[ i ]->order = i;
}

bool ContainsString( List_t< String^ >% vec, String^ item )
{
	for( int i = 0; i < vec.Count; ++i )
		if( vec[ i ] == item ) return true;
	return false;
}

int FindSkillIndex( Skill^ skill, List_t< Skill^ >^ vec )
{
	for( int i = 0; i < vec->Count; ++i )
		if( vec[ i ] == skill )
			return i;
	return -1;
}

void Skill::Load( String^ filename )
{
	IO::StreamReader fin( filename );

	Ability::static_abilities.Clear();
	Ability::static_ability_map.Clear();
	static_skills.Clear();
	static_skill_map.Clear();
	ordered_skills.Clear();
	Ability::static_abilities.Capacity = 128;
	static_skills.Capacity = 256;

	//skill,ability,points,type012
	String^ temp;
	while( !fin.EndOfStream && temp != L"" )
	{
		temp = fin.ReadLine();
		if( temp == L"" ) break;
		else if( temp[ 0 ] == L'#' ) continue;
		List_t< String^ > split;
		Utility::SplitString( %split, temp, L',' );
		Skill^ skill = gcnew Skill;
		skill->ping_index = Convert::ToUInt32( split[ 0 ] );
		skill->name = split[ 2 ];
		if( split[ 4 ] == L"" )
		{
			Assert( !Ability::torso_inc, L"Multiple Torso Inc skills in data file" );
			Ability::torso_inc = gcnew Ability;
			Ability::torso_inc->name = split[ 2 ];
			Ability::torso_inc->ping_index = 1;
			Ability::torso_inc->static_index = Ability::static_abilities.Count;
			Ability::static_abilities.Add( Ability::torso_inc );
			Ability::static_ability_map[ Ability::torso_inc->name ] = Ability::torso_inc;
			continue;
		}
		skill->points_required = Convert::ToInt32( split[ 4 ] );
		skill->ability = Ability::FindAbility( split[ 3 ] );
		if( !skill->ability )
		{
			Ability^ ability = gcnew Ability;
			ability->ping_index = Convert::ToUInt32( split[ 1 ] );
			ability->name = split[ 3 ];
			ability->auto_guard = ability->name == L"自動防御";
			ability->static_index = Ability::static_abilities.Count;
			//ability->order = Ability::static_abilities.Count;
			Ability::static_abilities.Add( ability );
			Ability::static_ability_map[ ability->name ] = ability;
			Ability::charm_ability_map[ ability->name ] = ability;
			skill->ability = ability;
			//if( split.Count != 5 )
			//	throw gcnew Exception( L"Ability " + ability->name + L" has not enough data" );
			for( int i = 6; i < split.Count; ++i )
			{
				if( split[ i ] != L"" )
				{
					SkillTag^ tag = SkillTag::FindTag( split[ i ] );
					if( !tag )
						throw gcnew Exception( L"Skill Tag '" + split[ i ] + L"' does not exist" );
					ability->tags.Add( tag );
				}
			}
			
			/*if( split[ 9 ] != L"" )
				ability->order = Convert::ToUInt32( split[ 9 ] );
			else
				throw gcnew Exception( L"Ability " + ability->name + L" is missing order number" );*/
		}
		skill->ability->skills[ skill->points_required ] = skill;
		skill->static_index = static_skills.Count;
		static_skills.Add( skill );
		static_skill_map[ skill->name ] = skill;
	}
	
	fin.Close();
	static_skills.TrimExcess();
	Ability::static_abilities.TrimExcess();
	Ability::UpdateOrdering();
	Skill::UpdateOrdering();
/*
	for( int i = 0; i < Ability::static_abilities.Count; ++i )
		Ability::ordered_abilities.Add( nullptr );

	for( int i = 0; i < Ability::static_abilities.Count; ++i )
		Ability::ordered_abilities[ Ability::static_abilities[ i ]->order ] = Ability::static_abilities[ i ];

	for( int i = 0; i < Ability::static_abilities.Count; ++i )
		if( Ability::ordered_abilities[ i ] == nullptr )
			throw gcnew Exception( L"Bad ordering of abilities" );

	//construct list of ordered skills
	for( int i = 1; i < Ability::ordered_abilities.Count; ++i )
	{
		Ability^ a = Ability::ordered_abilities[ i ];
		for( int j = 10; j <= 20; j += 5 )
			if( a->skills.ContainsKey( j ) )
				Skill::ordered_skills.Add( a->skills[ j ] );
		for( int j = -10; j >= -20; j -= 5 )
			if( a->skills.ContainsKey( j ) )
				Skill::ordered_skills.Add( a->skills[ j ] );
	}*/
}

Skill^ Skill::FindSkill( System::String^ name )
{
	if( static_skill_map.ContainsKey( name ) )
		return static_skill_map[ name ];
	else return nullptr;
}

void Skill::LoadLanguage( System::String^ filename )
{
	/*IO::StreamWriter testout( L"test.txt" );
	for each( Ability^ a in Ability::static_abilities )
		testout.WriteLine( a->name );
	testout.WriteLine();
	for each( Skill^ a in Skill::static_skills )
		testout.WriteLine( a->name );
	return;*/

	Ability::static_ability_map.Clear();
	IO::StreamReader fin( filename );
	bool ordered = false;
	/*if( fin.Peek() == L'#' )
	{
		String^ comment = fin.ReadLine();
		if( comment == L"#ordered" )
			ordered = true;
	}*/
	for( int i = 0; i < Ability::static_abilities.Count; )
	{
		String^ line = fin.ReadLine();
		if( line == L"" || line[ 0 ] == L'#' )
			continue;
		
		if( ordered )
		{
			//Ability::ordered_abilities[ i ]->name = line;
			//Ability::static_ability_map.Add( line, Ability::ordered_abilities[ i ] );
		}
		else
		{
			Ability::static_abilities[ i ]->name = line;
			Ability::static_ability_map.Add( line, Ability::static_abilities[ i ] );
		}
		i++;
	}
	static_skill_map.Clear();
	for( int i = 0; i < static_skills.Count; )
	{
		String^ line = fin.ReadLine();
		if( line == L"" || line[ 0 ] == L'#' )
			continue;
		
		if( ordered )
		{
			//ordered_skills[ i ]->name = line;
			//static_skill_map.Add( line, ordered_skills[ i ] );
		}
		else
		{
			static_skills[ i ]->name = line;
			static_skill_map.Add( line, static_skills[ i ] );
		}
		i++;
	}
}

void Skill::UpdateOrdering()
{
	ordered_skills.Clear();
	ordered_skills.AddRange( %static_skills );
	ordered_skills.Sort( gcnew Comparison< Skill^ >( CompareSkills ) );
	for( int i = 0; i < ordered_skills.Count; ++i )
		ordered_skills[ i ]->order = i;
}

void FindRelatedSkills( List_t< System::Windows::Forms::ComboBox^ >% skills, List_t< Map_t< unsigned, unsigned >^ >% index_maps )
{
	for each( Ability^ a in Ability::static_abilities )
		a->related = false;

	List_t< Ability^ > selected_abilities;
	array< unsigned >^ relation_count = gcnew array< unsigned >( Ability::static_abilities.Count );
	for( int i = 0; i < skills.Count; ++i )
	{
		if( skills[ i ]->SelectedIndex == -1 )
			continue;

		Ability^ a = Skill::static_skills[ index_maps[ i ][ skills[ i ]->SelectedIndex ] ]->ability ;
		relation_count[ a->static_index ] = 100; //lots because selected skills are related by definition
		selected_abilities.Add( a );
	}
	//static array< List_t< Armor^ >^ >^ static_armors;
	for each( List_t< Armor^ >^ la in Armor::static_armors )
	{
		for each( Armor^ a in la )
		{
			if( a->ContainsAnyAbility( selected_abilities ) )
			{
				for each( AbilityPair^ ap in a->abilities )
					if( ap->amount > 0 )
						relation_count[ ap->ability->static_index ]++;
			}
		}
	}
	
	for( int i = 0; i < relation_count->Length; ++i )
	{
		if( relation_count[ i ] > 4 )
			Ability::static_abilities[ i ]->related = true;
	}
}
