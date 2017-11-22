#include "stdafx.h"
#include "Common.h"
#include "Armor.h"
#include "Decoration.h"
#include "CharmDatabase.h"
#include "LoadedData.h"

using namespace System;
using namespace Windows::Forms;

/*#using <mscorlib.dll>
using namespace System::Runtime::InteropServices;

[DllImport("user32.dll", SetLastError = true)]
extern IntPtr PostMessage( IntPtr hWnd, int Msg, IntPtr wParam, IntPtr lParam );

[DllImport("Kernel32.dll")]
unsigned GetLastError();*/

enum PingTypeID
{
	ArmorID = 0,
	DecorationID,
	MaterialID,
	SkillTreeID, //aka Ability
	NoID
};

String^ GetPingTypeString( const PingTypeID id )
{
	array< String^ >^ ids = { L"Amr", L"Jew", L"Itm", L"Skl", L"???" };
	return ids[ unsigned( id ) ];
}

ref struct IDInfo
{
	PingTypeID type_id;
	unsigned index;
};

namespace Utility
{
	void AddAbilitiesToMap( List_t< AbilityPair^ >% _list, Map_t< Ability^, int >% _map, const int mult )
	{
		for each( AbilityPair^ ap in _list )
		{
			if( _map.ContainsKey( ap->ability ) )
				_map[ ap->ability ] += ap->amount * mult;
			else _map.Add( ap->ability, ap->amount * mult );
		}
	}

	String^ RemoveQuotes( String^ in_out )
	{
		if( in_out == L"" ) return in_out;
		in_out = in_out->Trim();
		if( in_out[ 0 ] == L'\"' ) in_out = in_out->Substring( 1 );
		if( in_out[ in_out->Length - 1 ] == L'\"' ) in_out = in_out->Substring( 0, in_out->Length - 1 );
		return in_out;
	}

	void SplitString( List_t< String^ >^ vec, String^ str, const wchar_t c )
	{
		str = str->Trim();
		int last_non_delim = 0;
		for( int i = 0; i < str->Length; ++i )
		{
			if( str[ i ] == c )
			{
				String^ substr = str->Substring( last_non_delim, i - last_non_delim );
				RemoveQuotes( substr );
				vec->Add( substr );
				last_non_delim = i + 1;
			}
		}
		str = str->Substring( last_non_delim, str->Length - last_non_delim );
		RemoveQuotes( str );
		vec->Add( str );
	}

	bool ContainsString( List_t< String^ >% vec, String^ item )
	{
		for( int i = 0; i < vec.Count; ++i )
			if( vec[ i ] == item ) return true;
		return false;
	}

	void FindLevelReqs( unsigned% available, unsigned% required, String^ input )
	{
		if( input->Length > 0 && input[ 0 ] == L'\"' ) input = Utility::RemoveQuotes( input );
		const int exclamation_point = input->LastIndexOf( L'!' );
		if( exclamation_point == -1 )
		{
			required = 0;
			available = Convert::ToUInt32( input );
			return;
		}
		required = Convert::ToUInt32( input->Substring( 0, exclamation_point ) );
		if( exclamation_point < input->Length - 1 )
			available = Convert::ToUInt32( input->Substring( exclamation_point + 1 ) );
	}

	String^ SlotString( const unsigned slots )
	{
		return slots == 0 ? L"---" :
			   slots == 1 ? L"O--" :
			   slots == 2 ? L"OO-" : L"OOO";
	}

	unsigned CountChars( String^ str, const wchar_t c )
	{
		unsigned total = 0;
		for( int i = 0; i < str->Length; ++i )
			if( str[ i ] == c ) ++total;
		return total;
	}

	void StartDexProcess( String^ path, String^ filename )
	{
		Diagnostics::ProcessStartInfo^ start_info = gcnew Diagnostics::ProcessStartInfo( filename );
		start_info->WorkingDirectory = path;
		start_info->UseShellExecute = true;
		Diagnostics::Process::Start( start_info );
	}

	void FillDexLines( array< String^ >^ lines, const unsigned i, const PingTypeID type_id, const unsigned id )
	{
		lines[ i + 0 ] = L"[ASStoDex]";
		lines[ i + 1 ] = L"IsASStoDex=1";
		lines[ i + 2 ] = L"ASStoDex_Dex=" + GetPingTypeString( type_id );
		lines[ i + 3 ] = L"ASStoDex_ID=" + Convert::ToString( id );
	}

	void CopyTextToClipBoardAndPing( String^ data, IDInfo^ info )
	{
		Clipboard::SetText( data );
		if( info == nullptr || info->type_id == NoID )
			return;

		//Notify Ping's app if it is running
		/*array< Diagnostics::Process^ >^ processes = Diagnostics::Process::GetProcesses();
		for each( Diagnostics::Process^ process in processes )
		{
			const unsigned WM_USER = 0x400;
			if( process != nullptr && ( process->ProcessName->Contains( L"MH3G Dex" ) || process->ProcessName->Contains( L"MH3U Dex" ) ) )
			{
				PostMessage( process->MainWindowHandle, WM_USER + 24, IntPtr( int( info->type_id ) ), IntPtr( int( info->index ) ) );
				return;
			}
		}*/

		//Ping's Dex not currently running. Try to find it and start it.
		array< String^ >^ files = IO::Directory::GetFiles( L"..", L"MH3G Dex*.exe", IO::SearchOption::AllDirectories );
		if( files == nullptr || files->Length == 0 )
		{
			files = IO::Directory::GetFiles( L"..", L"MH3U Dex*.exe", IO::SearchOption::AllDirectories );
			if( files == nullptr || files->Length == 0 )
				return;
		}

		//first find config file and set it to load ASS data on startup
		const int last_slash = files[0]->LastIndexOf( L'\\' );
		if( last_slash == -1 )
			return; //should never happen

		String^ path = files[ 0 ]->Substring( 0, last_slash );
		String^ config_file = path + L"\\Config.ini";

		if( !IO::File::Exists( config_file ) )
		{
			array< String^ >^ lines = 
			{
				L"[ASStoDex]",
				L"IsASStoDex=1",
				L"ASStoDex_Dex=" + GetPingTypeString( info->type_id ),
				L"ASStoDex_ID=" + Convert::ToString( info->index )
			};
			IO::File::WriteAllLines( config_file, lines );
			StartDexProcess( path, files[ 0 ] );
			return;
		}

		array< String^ >^ lines;
		try
		{
			lines = IO::File::ReadAllLines( config_file );
		}
		catch( Exception^ e )
		{
			MessageBox::Show( e->ToString(), L"Error", MessageBoxButtons::OK, MessageBoxIcon::Error );
			return;
		}
		for( int i = 0; i < lines->Length; ++i )
		{
			if( lines[ i ] == L"[ASStoDex]" && i+3 < lines->Length )
			{
				if( i+3 >= lines->Length )
					lines->Resize( lines, i + 4 );
				
				FillDexLines( lines, i, info->type_id, info->index );

				try
				{
					//write data to config file
					IO::File::WriteAllLines( config_file, lines );

					StartDexProcess( path, files[ 0 ] );
				}
				catch( Exception^ e )
				{
					MessageBox::Show( e->ToString(), L"Error", MessageBoxButtons::OK, MessageBoxIcon::Error );
				}
				return;
			}
		}
		
		lines->Resize( lines, lines->Length + 4 );
		FillDexLines( lines, lines->Length - 4, info->type_id, info->index );

		try
		{
			//write data to config file
			IO::File::WriteAllLines( config_file, lines );

			StartDexProcess( path, files[ 0 ] );
		}
		catch( Exception^ e )
		{
			MessageBox::Show( e->ToString(), L"Error", MessageBoxButtons::OK, MessageBoxIcon::Error );
		}
	}

	void ContextMenuClicked( Object^ obj, EventArgs^ args )
	{
		ToolStripMenuItem^ item = safe_cast< ToolStripMenuItem^ >( obj );
		const int space = item->Text->IndexOf( L' ' );
		String^ material = item->Text->Substring( space + 1 );
		CopyTextToClipBoardAndPing( material, safe_cast< IDInfo^ >( item->Tag ) );
	}

	void ContextMenuClicked2( System::Object^ obj, EventArgs^ args )
	{
		ToolStripMenuItem^ item = safe_cast< ToolStripMenuItem^ >( obj );
		CopyTextToClipBoardAndPing( item->Text, safe_cast< IDInfo^ >( item->Tag ) );
	}

	void AddMenuItem( ToolStripItemCollection^ list, String^ name, const bool context2, const PingTypeID type_id, const unsigned index )
	{
		ToolStripMenuItem^ item;
		if( context2 )
			item = gcnew ToolStripMenuItem( name, nullptr, gcnew EventHandler( ContextMenuClicked2 ) );
		else
			item = gcnew ToolStripMenuItem( name, nullptr, gcnew EventHandler( ContextMenuClicked ) );

		IDInfo^ info = gcnew IDInfo;
		info->type_id = type_id;
		info->index = index;
		item->Tag = info;

		list->Add( item );
	}

	void UpdateContextMenu( ContextMenuStrip^ strip, List_t< AbilityPair^ >% abilities )
	{
		for each( AbilityPair^ apair in abilities )
		{
			if( apair->amount != 0 && apair->ability != Ability::torso_inc )
			{
				AddMenuItem( strip->Items, Convert::ToString( apair->amount ) + L" " + apair->ability->name, false, SkillTreeID, apair->ability->ping_index );
				ToolStripMenuItem^ mi = safe_cast< ToolStripMenuItem^ >( strip->Items[ strip->Items->Count - 1 ] );
				AddMenuItem( mi->DropDownItems, apair->ability->name, true, SkillTreeID, apair->ability->ping_index );
				mi->DropDownItems->Add( L"-" );
				Ability::SkillMap_t::Enumerator e = apair->ability->skills.GetEnumerator();
				while( e.MoveNext() )
				{
					AddMenuItem( mi->DropDownItems, e.Current.Key + L": " + e.Current.Value->name, false, SkillTreeID, e.Current.Value->ability->ping_index );
					if( e.Current.Key < 0 )
						mi->DropDownItems[ mi->DropDownItems->Count - 1 ]->ForeColor = Drawing::Color::Red;
				}
			}
			else
				strip->Items->Add( apair->ability->name, nullptr, gcnew EventHandler( ContextMenuClicked2 ) );
		}
	}

	void UpdateContextMenu( ContextMenuStrip^ strip, List_t< MaterialComponent^ >^ components )
	{
		strip->Items->Add( L"-" );
		for each( MaterialComponent^ part in components )
		{
			AddMenuItem( strip->Items, Convert::ToString( part->amount ) + L"x " + part->material->name, false, MaterialID, part->material->ping_index );
		}
	}

	void UpdateContextMenu( ContextMenuStrip^ strip, Decoration^ decoration )
	{
		AddMenuItem( strip->Items, decoration->name, true, DecorationID, decoration->ping_index );
		strip->Items->Add( L"-" );
		strip->Items->Add( SlotString( decoration->slots_required ) );
		strip->Items->Add( L"-" );
		UpdateContextMenu( strip, decoration->abilities );
		UpdateContextMenu( strip, %decoration->components );
		if( decoration->components2.Count > 0 )
			UpdateContextMenu( strip, %decoration->components2 );
	}

	void UpdateContextMenu( ContextMenuStrip^ strip, Armor^ armor )
	{
		AddMenuItem( strip->Items, armor->name, true, ArmorID, armor->ping_index );
		strip->Items->Add( L"-" );
		strip->Items->Add( SlotString( armor->num_slots ) );
		strip->Items->Add( L"-" );
		strip->Items->Add( ColonString( Defence ) + Convert::ToString( armor->defence ) + RunString( To ) + Convert::ToString( armor->max_defence ) );
		strip->Items->Add( ColonString( DragonRes ) + Convert::ToString( armor->dragon_res ) );
		strip->Items->Add( ColonString( FireRes ) + Convert::ToString( armor->fire_res ) );
		strip->Items->Add( ColonString( IceRes ) + Convert::ToString( armor->ice_res ) );
		strip->Items->Add( ColonString( ThunderRes ) + Convert::ToString( armor->thunder_res ) );
		strip->Items->Add( ColonString( WaterRes ) + Convert::ToString( armor->water_res ) );
		strip->Items->Add( L"-" );
		UpdateContextMenu( strip, armor->abilities );
		UpdateContextMenu( strip, %armor->components );
	}

	ref class ToolHost : public ToolStripControlHost
	{
		RichTextBox rtb;
		String^ rtf;
	public:
		ToolHost( const unsigned table_number, const unsigned mystery, const unsigned shining, const unsigned timeworn, const unsigned weathered )
			: ToolStripControlHost( %rtb )
		{
			using namespace System::Drawing;

			rtb.SelectionColor = Color::Black;
			rtb.AppendText( StartString( Table ) + Convert::ToString( table_number ) + L": " );
			rtb.SelectionColor = Color::Gray;
			rtb.AppendText( Convert::ToString( mystery ) );
			rtb.SelectionColor = rtb.DefaultForeColor;
			rtb.AppendText( L", " );
			rtb.SelectionColor = Color::DarkGoldenrod;
			rtb.AppendText( Convert::ToString( shining ) );
			rtb.SelectionColor = rtb.DefaultForeColor;
			rtb.AppendText( L", " );
			rtb.SelectionColor = Color::Red;
			rtb.AppendText( Convert::ToString( timeworn ) );
			rtb.AppendText( L", " );
			rtb.SelectionColor = Color::Blue;
			rtb.AppendText( Convert::ToString( weathered ) );
			rtb.BorderStyle = System::Windows::Forms::BorderStyle::None;
			rtb.ReadOnly = true;
			rtf = rtb.Rtf;
		}

		virtual void OnPaint( PaintEventArgs^ args ) override
		{
			rtb.Rtf = rtf;
			ToolStripControlHost::OnPaint( args );
		}
	};

	template< bool check_ability1, bool check_ability2 >
	void FindCharm2( CharmLocationDatum^ result, Ability^ ability1, const unsigned points1, Ability^ ability2, const int points2, const unsigned num_slots )
	{
		Charm test;
		test.num_slots = num_slots;
		test.abilities.Add( gcnew AbilityPair( ability1, points1 ) );
		test.abilities.Add( gcnew AbilityPair( ability2, points2 ) );
		const unsigned my_hash = test.GetHash();

		unsigned num_found = 0;
		array< unsigned >^ num_for_table = gcnew array< unsigned >( 17 );

		Map_t< unsigned, unsigned >::Enumerator iter = CharmDatabase::hash_to_table->GetEnumerator();
		while( iter.MoveNext() )
		{
			//unhash
			const unsigned charm_hash = iter.Current.Key;
			const unsigned charm_slots = charm_hash & 0x3;
			const unsigned charm_ability1_index = ( charm_hash >> 11 ) & 0x7F;
			const unsigned charm_ability2_index = ( charm_hash >> 18 ) & 0x7F;
			const unsigned charm_points1 = ( charm_hash >> 2 ) & 0xF;
			const int charm_points2 = ( ( charm_hash >> 6 ) & 0x1F ) - 12;

			if( charm_slots < num_slots )
				continue;

			if( check_ability1 )
			{
				if( check_ability2 )
				{
					if( points2 < 0 )
					{
						if( !( charm_ability1_index == ability1->static_index && charm_points1 >= points1 &&
							   ( charm_ability2_index != ability2->static_index || charm_ability2_index == ability2->static_index && charm_points2 >= points2 )
							   ||
							   charm_ability2_index == ability1->static_index && charm_points2 >= (int)points1 &&
							   ( charm_ability1_index != ability2->static_index || charm_ability1_index == ability2->static_index && (int)charm_points1 >= points2 ) ) )
							continue;
					}
					else if( !( charm_ability1_index == ability1->static_index && charm_points1 >= points1 &&
							    charm_ability2_index == ability2->static_index && charm_points2 >= points2
							    ||
							    charm_ability2_index == ability1->static_index && charm_points2 >= (int)points1 &&
							    charm_ability1_index == ability2->static_index && (int)charm_points1 >= points2 ) )
							   continue;
				}
				else
				{
					if( !( charm_ability1_index == ability1->static_index && (int)charm_points1 >= points1 ||
						   charm_ability2_index == ability1->static_index && charm_points2 >= (int)points1 ) )
						   continue;
				}
			}

			const unsigned info = iter.Current.Value;
			for( unsigned table = 1; table <= 17; ++table )
			{
				if( info & ( 1 << table ) )
				{
					for( unsigned charm_type = 0; charm_type < 4; ++charm_type )
					{
						if( info & ( 0x80000000 >> charm_type ) )
						{
							result->table[ table - 1, charm_type ]++;
							if( num_for_table[ table - 1 ]++ == 0 )
							{
								Charm eg;
								eg.num_slots = charm_slots;
								AbilityPair ap1( Ability::static_abilities[ charm_ability1_index ], charm_points1 );
								eg.abilities.Add( %ap1 );
								if( charm_points2 != 0 )
								{
									AbilityPair ap2( Ability::static_abilities[ charm_ability2_index ], charm_points2 );
									eg.abilities.Add( %ap2 );
								}
								result->examples[ table - 1 ] = eg.GetName();

							}
							else result->examples[ table - 1 ] = nullptr;
							if( num_found++ == 0 )
							{
								Assert( result->examples[ table - 1 ] != nullptr, L"Unexpected null string" );
								result->example = result->examples[ table - 1 ];
							}
							else result->example = nullptr;
						}
					}
				}
			}
		}
	}

	CharmLocationDatum^ FindCharm( Ability^ ability1, const unsigned points1, Ability^ ability2, const int points2, const unsigned num_slots )
	{
		CharmLocationDatum^ result = gcnew CharmLocationDatum();
		result->table = gcnew array< unsigned char, 2 >( 17, 4 );
		result->examples = gcnew array< System::String^ >( 17 );
		
		if( ability1 )
		{
			if( ability2 )
				FindCharm2< true, true >( result, ability1, points1, ability2, points2, num_slots );
			else
				FindCharm2< true, false >( result, ability1, points1, ability2, points2, num_slots );
		}
		else FindCharm2< false, false >( result, ability1, points1, ability2, points2, num_slots );
		return result;
	}

	CharmLocationDatum^ UpdateCharmLocationCache( String^ charm )
	{
		unsigned points1 = 0;
		int points2 = 0;
		Ability^ ability1 = nullptr, ^ability2 = nullptr;
		Charm^ c = gcnew Charm( charm );
		if( c->abilities.Count > 0 )
		{
			ability1 = c->abilities[0]->ability;
			points1 = c->abilities[0]->amount;

			if( c->abilities.Count > 1 )
			{
				ability2 = c->abilities[1]->ability;
				points2 = c->abilities[1]->amount;
			}
		}
		
		CharmLocationDatum^ results = FindCharm( ability1, points1, ability2, points2, c->num_slots );
		CharmDatabase::location_cache->Add( charm, results );
		return results;
	}

	void UpdateContextMenu( ContextMenuStrip^ strip, String^ charm, const unsigned table )
	{
		CharmLocationDatum^ results;
		if( CharmDatabase::location_cache->ContainsKey( charm ) )
			results = CharmDatabase::location_cache[ charm ];
		else
		{
			results = UpdateCharmLocationCache( charm );
		}
		
		if( results->example )
			strip->Items->Add( results->example );
		else if( table > 0 && table < 18 && results->examples[ table - 1 ] != nullptr )
			strip->Items->Add( results->examples[ table - 1 ] );
		else
			strip->Items->Add( charm );

		strip->Items->Add( L"-" );
		if( table > 0 && table < 18 )
		{
			if( results->table[ table - 1, 0 ] ||
				results->table[ table - 1, 1 ] ||
				results->table[ table - 1, 2 ] ||
				results->table[ table - 1, 3 ] )
			{
				strip->Items->Add( gcnew ToolHost( table, results->table[ table - 1, 0 ], results->table[ table - 1, 1 ], results->table[ table - 1, 2 ], results->table[ table - 1, 3 ] ) );
			}
			else strip->Items->Add( StaticString( UnknownCharm ) );
		}
		else
		{
			for( int i = 0; i < results->table->GetLength( 0 ); ++i )
			{
				if( results->table[ i, 0 ] ||
					results->table[ i, 1 ] ||
					results->table[ i, 2 ] ||
					results->table[ i, 3 ] )
				{
					strip->Items->Add( gcnew ToolHost( i + 1, results->table[ i, 0 ], results->table[ i, 1 ], results->table[ i, 2 ], results->table[ i, 3 ] ) );
				}
			}
		}
		strip->Invalidate();
	}
}

void Material::LoadMaterials( String^ filename )
{
	static_materials.Clear();
	IO::StreamReader fin( filename );
	while( !fin.EndOfStream )
	{
		String^ line = fin.ReadLine();
		if( line == L"" ) continue;
		Material^ mat = gcnew Material;
		List_t< String^ > split;
		Utility::SplitString( %split, line, L',' );
		mat->name = split[ 1 ];
		mat->ping_index = Convert::ToUInt32( split[ 0 ] );
		mat->event_only = false;
		mat->jap_only = false;
		mat->difficulty = 0;
		if( split.Count >= 3 )
		{
			if( split[ 2 ] == L"Event" )
			{
				mat->event_only = true;
			}
			else if( split[ 2 ] == L"jEvent" )
			{
				mat->event_only = true;
				mat->jap_only = true;
			}
			else if( split[ 2 ] != L"" )
			{
				mat->difficulty = Convert::ToUInt32( split[ 2 ] );
			}
		}
		static_materials.Add( mat );
	}
	fin.Close();
	static_material_map.Clear();
	for each( Material^% mat in static_materials )
	{
		static_material_map.Add( mat->name, mat );
	}
}

Material^ Material::FindMaterial( String^ name )
{
	if( static_material_map.ContainsKey( name ) )
		return static_material_map[ name ];
	return nullptr;
}

void Material::LoadLanguage( String^ filename )
{
	static_material_map.Clear();
	IO::StreamReader fin( filename );
	for( int i = 0; i < static_materials.Count; )
	{
		String^ line = fin.ReadLine();
		if( line == L"" || line[ 0 ] == L'#' )
			continue;
		static_materials[ i ]->name = line;
		static_material_map.Add( line, static_materials[ i ] );
		i++;
	}
}

void StringTable::LoadLanguage( String^ lang )
{
	String^ dir = L"Data/Languages/" + lang + L"/";
	//load strings
	{
		text = gcnew array< String^ >( (int)StringTable::StringIndex::NumStrings );
		IO::StreamReader fin( IO::File::OpenRead( dir + L"strings.txt" ) );
		int i;
		for( i = 0; i < text->Length; )
		{
			String^ line = fin.ReadLine();
			if( !line )
			{
				Windows::Forms::MessageBox::Show( L"Unexpected end of file: not enough lines of text" );
				break;
			}
			if( line == L"" || line[ 0 ] == L'#' )
				continue;
			text[ i++ ] = line;// + L"qq";
		}
		fin.Close();
	}
	//load armor, skills, etc.
	SkillTag::LoadLanguage( dir + L"tags.txt" );
	Material::LoadLanguage( dir + L"components.txt" );

	Armor::static_armor_map.Clear();
	Armor::LoadLanguage( dir + L"head.txt",  Armor::ArmorType::HEAD );
	Armor::LoadLanguage( dir + L"body.txt",  Armor::ArmorType::BODY );
	Armor::LoadLanguage( dir + L"arms.txt",  Armor::ArmorType::ARMS );
	Armor::LoadLanguage( dir + L"waist.txt", Armor::ArmorType::WAIST );
	Armor::LoadLanguage( dir + L"legs.txt",  Armor::ArmorType::LEGS );

	Decoration::LoadLanguage( dir + L"decorations.txt" );

	Skill::LoadLanguage( dir + L"skills.txt" );
	Skill::UpdateOrdering();
	Ability::UpdateOrdering();
}

String^ StripAmpersands( String^ input )
{
	String^ res = L"";
	for each( wchar_t c in input )
	{
		if( c != L'&' )
			res += c;
	}
	return res;
}

void myassert( const bool val, String^ message )
{
	if( !val )
		MessageBox::Show( message );
}

bool ConvertInt( int% i, String^ str, StringTable::StringIndex err )
{
	int temp;
	try
	{
		const int limit = ( str[ 0 ] == L'-' ) ? 3 : 2;
		if( str->Length > limit )
			temp = Convert::ToInt32( str->Substring( 0, limit ) );
		else temp = Convert::ToInt32( str );
	}
	catch( FormatException^ )
	{
		MessageBox::Show( StringTable::text[(int)err ], StaticString( Error ), MessageBoxButtons::OK, MessageBoxIcon::Error );
		return false;
	}
	i = temp;
	return true;
}

bool ConvertUInt( unsigned% i, String^ str, StringTable::StringIndex err )
{
	unsigned temp;
	try
	{
		temp = Convert::ToUInt32( str );
	}
	catch( FormatException^ )
	{
		MessageBox::Show( StringTable::text[(int)err ], StaticString( Error ), MessageBoxButtons::OK, MessageBoxIcon::Error );
		return false;
	}
	i = temp;
	return true;
}
