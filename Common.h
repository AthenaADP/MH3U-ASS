#pragma once

enum class HunterType { BLADEMASTER = 1, GUNNER, BOTH_TYPES };
enum class Gender { MALE = 4, FEMALE, BOTH_GENDERS };

#define List_t System::Collections::Generic::List
#define Map_t System::Collections::Generic::Dictionary

ref struct Armor;
ref struct Ability;
ref struct Decoration;
ref struct CharmLocationDatum;
ref struct AbilityPair;

ref struct AdvancedSearchOptions abstract
{
	bool force_enable, force_disable, default_piece;
	unsigned adv_index;
};

namespace Utility
{

	template< class T >
	bool Contains( List_t< T^ >^ cont, const T^ val )
	{
		for( int i = 0; i < cont->Count; ++i )
			if( cont[ i ] == val ) return true;
		return false;
	}

	template< class T >
	int GetIndexOf( List_t< T^ >^ cont, const T^ val )
	{
		for( int i = 0; i < cont->Count; ++i )
			if( cont[ i ] == val ) return i;
		return -1;
	}

	template< class T >
	T^ FindByName( List_t< T^ >^ cont, System::String^ name )
	{
		for each( T^% item in cont )
		{
			if( item->name == name )
				return item;
		}
		return nullptr;
	}

	void AddAbilitiesToMap( List_t< AbilityPair^ >% _list, Map_t< Ability^, int >% _map, const int mult = 1 );

	System::String^ SlotString( const unsigned slots );
	unsigned CountChars( System::String^ str, const wchar_t c );

	bool ContainsString( List_t< System::String^ >% vec, System::String^ item );

	void SplitString( List_t< System::String^ >^ vec, System::String^ str, const wchar_t c );

	void FindLevelReqs( unsigned% available, unsigned% required, System::String^ input );

	System::String^ RemoveQuotes( System::String^ in_out );

	inline unsigned Min( const unsigned a, const unsigned b ) { return a > b ? b : a; }

	CharmLocationDatum^ FindCharm( Ability^ ability1, const unsigned points1, Ability^ ability2, const int points2, const unsigned num_slots );
	CharmLocationDatum^ UpdateCharmLocationCache( System::String^ charm );

	void UpdateContextMenu( System::Windows::Forms::ContextMenuStrip^ strip, Decoration^ decoration );
	void UpdateContextMenu( System::Windows::Forms::ContextMenuStrip^ strip, Armor^ armor );
	void UpdateContextMenu( System::Windows::Forms::ContextMenuStrip^ strip, System::String^ charm, const unsigned table );
}

void myassert( const bool val, System::String^ message );

#ifdef _DEBUG
#define Assert( X, Y ) myassert( !!(X), Y );
#else
#define Assert( X, Y )
#endif

inline int Min( const int a, const int b )
{
	return a > b ? b : a;
}

ref struct Material
{
	System::String^ name;
	bool event_only, jap_only;
	unsigned difficulty, ping_index;

	static List_t< Material^ > static_materials;
	static Material^ FindMaterial( System::String^ name );
	static void LoadMaterials( System::String^ filename );
	static void LoadLanguage( System::String^ filename );
	static Map_t< System::String^, Material^ > static_material_map;
};

ref struct MaterialComponent
{
	Material^ material;
	unsigned amount;
};

ref struct StringTable
{
	static array< System::String^ >^ text;
	static void LoadLanguage( System::String^ dir );
	enum class StringIndex
	{ 
		TranslationCredit = 0,
		File,
		Options,
		Language,
		Help,
		LoadData,
		SaveData,
		SaveCharms,
		Exit,
		ClearSettings,
		AllowBadSkills,
		AllowPiercings,
		AllowEventArmor,
		AllowJapaneseOnlyDLC,
		AllowLowerTierArmor,
		SortSkillsAlphabetically,
		CheckForUpdates,
		About,
		MyCharms,
		Import,
		QuickSearch,
		AdvancedSearch,
		Cancel,
		Default,
		AdvancedNone,
		FilterByNone,
		SortByNone,
		Search,
		SelectNone,
		SelectBest,
		AddNew,
		Delete,
		DeleteAll,
		MoveUp,
		MoveDown,
		Trim,
		Close,
		FindNext,
		HR,
		VillageQuests,
		MaxWeaponSlots,
		Male,
		Female,
		FilterResultsByCharm,
		SortResultsBy,
		All,
		DragonRes,
		FireRes,
		IceRes,
		ThunderRes,
		WaterRes,
		BaseDefence,
		MaxDefence,
		Difficulty,
		Rarity,
		SortSlotsSpare,
		SortFamily,
		SortExtraSkills,
		Charms,
		UseNoCharms,
		UseOnlyMyCharms,
		UseOnlySlottedCharms,
		UseUpToOneSkillCharms,
		UseOnlyTwoSkillCharms,
		Skills,
		SkillFilters,
		Blademaster,
		Gunner,
		SelectArmor,
		ImportCharmsFromSaveData,
		Characters,
		DeleteExistingCharms,
		AreYouSure,
		Sort,
		Slots,
		NoneBrackets,
		SlotSpare,
		SlotsSpare,
		OrAnythingWithSingular,
		OrAnythingWithPlural,
		OrAnythingWithTorsoInc,
		SolutionsFound,
		ShowingFirstSolutionsOnly,
		MasaxMHCharmList,
		MasaxMHCharmListCorrupted,
		Version,
		Find,
		Cheater,
		Error,
		To,
		Defence,
		Table,
		Results,
		CharmTable,
		UnknownTable,
		AnyTable,
		DetectTable,
		FoundNCharmsUniqueToTable,
		FoundNCharmsOnTable,
		FindTableFailed,
		UnknownCharm,
		SorryAboutTheCharms,
		DeleteAllCharms,
		ASSSettings,
		MaxResults,
		PrintDecoNames,
		DefEleAbbrev,
		Jewel,
		NumStrings
	};
};

bool ConvertInt( int% i, System::String^ str, StringTable::StringIndex err );
bool ConvertUInt( unsigned% i, System::String^ str, StringTable::StringIndex err );

System::String^ StripAmpersands( System::String^ input );

#define StaticString( X ) StringTable::text[ (int)StringTable::StringIndex::X ]
#define AddString( X ) L" " + StaticString( X )
#define RunString( X ) L" " + StaticString( X ) + L" "
#define StartString( X ) StaticString( X ) + L" "
#define BasicString( X ) StripAmpersands( StaticString( X ) )
#define ColonString( X ) StaticString( X ) + L": "
#define SpaceListString( X ) L" " + StaticString( X ) + L", "
#define FormatString1( X, Y ) StaticString( X )->Replace( L"%1", Y )
#define FormatString2( X, Y, Z ) StaticString( X )->Replace( L"%1", Convert::ToString( Y ) )->Replace( L"%2", Convert::ToString( Z ) )
#define FormatString7( X, A, B, C, D, E, F, G ) StaticString( X )->Replace( L"%1", Convert::ToString( A ) )->Replace( L"%2", Convert::ToString( B ) )->Replace( L"%3", Convert::ToString( C ) )->Replace( L"%4", Convert::ToString( D ) )->Replace( L"%5", Convert::ToString( E ) )->Replace( L"%6", Convert::ToString( F ) )->Replace( L"%7", Convert::ToString( G ) )
#define FormatStringN( X, Y ) StaticString( X )->Replace( L"%1", Convert::ToString( Y ) )