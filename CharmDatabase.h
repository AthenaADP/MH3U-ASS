#pragma once
#include "Common.h"

ref struct Charm;
ref struct Query;

ref struct TableSlotDatum
{
	array< signed char, 2 >^ max_single;
	array< array< List_t< Charm^ >^, 2 >^ >^ two_skill_data;
};

ref struct CharmLocationDatum
{
	array< unsigned char, 2 >^ table;
	System::String^ example;
	array< System::String^ >^ examples;
};

ref struct CharmDatabase
{
	static List_t< Charm^ > mycharms;
	static array< TableSlotDatum^, 2 >^ min_max;
	static Map_t< unsigned, unsigned >^ hash_to_table;
	static Map_t< System::String^, CharmLocationDatum^ >^ location_cache;
	static array< unsigned char, 2 >^ have_slots;

	static void LoadCustom();
	static void SaveCustom();

	static int DetectCharmTable();
	static void GenerateCharmTable();
	static bool CharmExists( Charm^ charm );
	static bool CharmIsLegal( Charm^ charm );
	static List_t< Charm^ >^ GetCharms( Query^ query, const bool use_two_skill_charms );
};