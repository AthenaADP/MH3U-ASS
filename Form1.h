#pragma once
#include "LoadedData.h"
#include "frmAbout.h"
#include "frmAdvanced.h"
#include "frmFind.h"
#include "frmImportCharms.h"
#include "CharmDatabase.h"
#include "Version.h"
#include "ManageCharms.h"

namespace MH3GASS 
{
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

#pragma region Comparison Functions
	int Compare( const int x, const int y )
	{
		return x < y ? -1 : y < x ? 1 : 0;
	}

	int ReverseCompare( const int x, const int y )
	{
		return x < y ? 1 : y < x ? -1 : 0;
	}
#define TryReverseCompare( X, Y ) X != Y ? ReverseCompare( X, Y )

#define BasicCompare( Name, Var ) \
	int CompareSolutionBy##Name( Solution^ s1, Solution^ s2 )\
	{\
		return s1->Var != s2->Var ? ReverseCompare( s1->Var, s2->Var ) : ReverseCompare( s1->total_slots_spare, s2->total_slots_spare );\
	}

	BasicCompare( Defence, defence );
	BasicCompare( MaxDefence, max_defence );
	BasicCompare( FireRes, fire_res );
	BasicCompare( IceRes, ice_res );
	BasicCompare( WaterRes, water_res );
	BasicCompare( ThunderRes, thunder_res );
	BasicCompare( DragonRes, dragon_res );
	BasicCompare( Rarity, rarity );

#undef BasicCompare
	
	int CompareSolutionByDifficulty( Solution^ s1, Solution^ s2 )
	{
		return Compare( s1->difficulty, s2->difficulty );
	}

	int CompareSolutionBySlotsSpare( Solution^ s1, Solution^ s2 )
	{
		return ReverseCompare( s1->total_slots_spare, s2->total_slots_spare );
	}
	int CompareSolutionByFamily( Solution^ s1, Solution^ s2 )
	{
		return TryReverseCompare( s1->family_score[0], s2->family_score[0] ) :
			   TryReverseCompare( s1->family_score[1], s2->family_score[1] ) :
			   TryReverseCompare( s1->family_score[2], s2->family_score[2] ) :
			   ReverseCompare( s1->total_slots_spare, s2->total_slots_spare );
	}

	int CompareSolutionsByExtraSkills( Solution^ s1, Solution^ s2 )
	{
		return TryReverseCompare( s1->extra_skills.Count, s2->extra_skills.Count ) :
			   ReverseCompare( s1->extra_skill_score, s2->extra_skill_score );
	}
#pragma endregion

	public ref class Form1 : public System::Windows::Forms::Form
	{
		typedef System::Windows::Forms::DialogResult DialogResult_t;
		const static DialogResult_t OK = DialogResult_t::OK;
		int MAX_LIMIT;
		const static int NumSkills = 6;
		static Threading::Mutex^ progress_mutex = gcnew Threading::Mutex;
		static Threading::Mutex^ results_mutex = gcnew Threading::Mutex;
		static Threading::Mutex^ charm_map_mutex = gcnew Threading::Mutex;
		static Threading::Mutex^ worker_mutex = gcnew Threading::Mutex;
		String^ CFG_FILE;
		String^ endl;
		String^ last_result;
		bool lock_skills, sort_off, can_save, last_search_gunner, updating_language, construction_complete, lock_related, search_cancelled;
		LoadedData^ data;
		Query^ query;
		frmFind^ find_dialog;
		frmAdvanced::Result_t^ blast_options;
		frmAdvanced::Result_t^ glast_options;
		typedef Generic::Dictionary< unsigned, unsigned > IndexMap;
		typedef Generic::Dictionary< String^, List_t< Solution^ >^ > CharmSolutionMap;
		CharmSolutionMap charm_solution_map;
		Generic::Dictionary< String^, Generic::Dictionary< __int64, bool >^ > existing_armor;

		List_t< Solution^ > final_solutions, no_charm_solutions, all_solutions;
		List_t< ComboBox^ > bSkills, gSkills;
		List_t< ComboBox^ > bSkillFilters, gSkillFilters;
		List_t< IndexMap^ > bIndexMaps, gIndexMaps;
		List_t< String^ > languages;
		List_t< Charm^ > charm_box_charms;
		int language, adv_x, adv_y;
#pragma region Members
	private: System::Windows::Forms::MenuStrip^  menuStrip1;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuFile;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuExit;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuHelp;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuAbout;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuLanguage;
	private: System::Windows::Forms::GroupBox^  groupBox6;
	private: System::Windows::Forms::RadioButton^  rdoFemale;
	private: System::Windows::Forms::RadioButton^  rdoMale;
	private: System::Windows::Forms::Button^  btnAdvancedSearch;
	private: System::Windows::Forms::TabControl^  tabHunterType;
	private: System::Windows::Forms::TabPage^  tabBlademaster;
	private: System::Windows::Forms::TabPage^  tabGunner;
	private: System::Windows::Forms::GroupBox^  grpGSkillFilters;
	private: System::Windows::Forms::GroupBox^  grpGSkills;
	private: System::Windows::Forms::NumericUpDown^  nudHR;
	private: System::Windows::Forms::NumericUpDown^  nudWeaponSlots;
	private: System::Windows::Forms::NumericUpDown^  nudElder;
	private: System::Windows::Forms::GroupBox^  groupBox1;
	private: System::Windows::Forms::Label^  lblHR;
	private: System::Windows::Forms::Label^  lblElder;
	private: System::Windows::Forms::Label^  lblSlots;
	private: System::Windows::Forms::Button^  btnCancel;
	private: System::Windows::Forms::GroupBox^  grpBSkills;
	private: System::Windows::Forms::Button^  btnSearch;
	private: System::Windows::Forms::ProgressBar^  progressBar1;
	private: System::Windows::Forms::RichTextBox^  txtSolutions;
	private: System::Windows::Forms::GroupBox^  groupBox4;
	private: System::Windows::Forms::GroupBox^  grpResults;
	private: System::Windows::Forms::GroupBox^  grpBSkillFilters;
	private: System::Windows::Forms::GroupBox^  grpSort;
	private: System::Windows::Forms::ComboBox^  cmbSort;
	private: System::Windows::Forms::ContextMenuStrip^  cmsSolutions;
	private: System::Windows::Forms::GroupBox^  grpCharmFilter;
	private: System::Windows::Forms::ComboBox^  cmbCharms;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuOptions;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuAllowBadSkills;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuAllowPiercings;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuAllowEventArmor;
	private: System::Windows::Forms::Button^  btnCharms;
	private: System::Windows::Forms::GroupBox^  grpCharms;
	private: System::Windows::Forms::ComboBox^  cmbCharmSelect;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuCheckForUpdates;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuClearSettings;
	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator1;
	private: System::Windows::Forms::ContextMenuStrip^  cmsCharms;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuLoadData;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuSaveData;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuSortSkillsAlphabetically;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuPrintDecoNames;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuMaxResults;
	private: System::Windows::Forms::ToolStripTextBox^  mnuNumResults;
	private: System::Windows::Forms::Button^  btnImport;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuAllowLowerTierArmor;
	private: System::Windows::Forms::Label^  lblCharmTable;
	private: System::Windows::Forms::Label^  lblWhatIsCharmTable;
	private: System::Windows::Forms::ComboBox^  cmbCharmTable;

#pragma endregion

		List_t< BackgroundWorker^ >  workers;
		List_t< ThreadSearchData^ > worker_data;
		unsigned finished_workers, total_progress, worker_start_index, num_updates;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuAllowJapaneseOnlyDLC;

		void ClearFilters()
		{
			for each( ComboBox^ box in bSkillFilters )
				box->Items->Clear();
			for each( ComboBox^ box in gSkillFilters )
				box->Items->Clear();
		}
	
		void AddFilters()
		{
			for each( SkillTag^ tag in SkillTag::tags )
			{
				if( !tag->disable_g )
					for each( ComboBox^ box in gSkillFilters )
						box->Items->Add( tag->name );
				if( !tag->disable_b )
					for each( ComboBox^ box in bSkillFilters )
						box->Items->Add( tag->name );
			}
		}

		void InitFilters()
		{
			ClearFilters();
			AddFilters();
			for each( ComboBox^ box in gSkillFilters )
				box->SelectedIndex = 0;
			for each( ComboBox^ box in bSkillFilters )
				box->SelectedIndex = 0;
		}

		void ResetSkill( ComboBox^ box, IndexMap^ map, Skill^ skill )
		{
			if( skill == nullptr ) return;
			IndexMap::Enumerator iter = map->GetEnumerator();
			while( iter.MoveNext() )
			{
				if( Skill::static_skills[ iter.Current.Value ] == skill )
				{
					box->SelectedIndex = iter.Current.Key;
					return;
				}
			}
		}

		void InitSkills2( ComboBox^ box, IndexMap^ map, const int filter, List_t< Ability^ >^ disallowed )
		{
			map->Clear();
			box->SelectedIndex = -1;
			box->Items->Clear();
			if( filter == -1 || !StringTable::text )
				return;
			box->Items->Add( StaticString( NoneBrackets ) );
			List_t< Skill^ >^ the_list = mnuSortSkillsAlphabetically->Checked ? %Skill::ordered_skills : %Skill::static_skills;
			for each( Skill^ skill in the_list )
			{
				if( skill->points_required <= 0 || 
					Utility::Contains( disallowed, skill->ability ) )
					continue;

				if( filter == 0 || filter == 1 && skill->ability->tags.Count == 0 || 
					filter == 2 && skill->ability->related ||
					!!Utility::FindByName( %skill->ability->tags, SkillTag::tags[ filter ]->name ) )
				{
					map[ box->Items->Count ] = skill->static_index;
					box->Items->Add( skill->name );
				}
			}
		}

		void InitSkills( ComboBox^ box, IndexMap^ map, const int filter, List_t< Ability^ >^ disallowed, const bool blade )
		{
			unsigned index = filter;
			if( blade )
			{
				for( int i = 0; i <= filter; ++i )
					index += SkillTag::tags[i]->disable_b;
			}
			else
			{
				for( int i = 0; i <= filter; ++i )
					index += SkillTag::tags[i]->disable_g;
			}
			InitSkills2( box, map, index, disallowed );
		}

		void InitSkills()
		{
			for( unsigned i = 0; i < NumSkills; ++i )
			{
				InitSkills( gSkills[ i ], gIndexMaps[ i ], gSkillFilters[ i ]->SelectedIndex, gcnew List_t< Ability^ >, false );
				InitSkills( bSkills[ i ], bIndexMaps[ i ], bSkillFilters[ i ]->SelectedIndex, gcnew List_t< Ability^ >, true );
			}
		}

		ComboBox^ GetNewComboBox( const unsigned width, const unsigned i )
		{
			ComboBox^ box = gcnew ComboBox;
			box->Location = System::Drawing::Point( 6, 19 + i * 27 );
			box->Size = System::Drawing::Size( width, 21 );
			box->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			box->AutoCompleteSource = AutoCompleteSource::ListItems;
			box->AutoCompleteMode = AutoCompleteMode::Suggest;
			return box;
		}

		void InitializeComboBoxes()
		{
			for( unsigned i = 0; i < NumSkills; ++i )
			{
				gSkillFilters.Add( GetNewComboBox( 134, i ) );
				bSkillFilters.Add( GetNewComboBox( 134, i ) );
				gSkillFilters[ i ]->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSkillFilter_SelectedIndexChanged);
				bSkillFilters[ i ]->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSkillFilter_SelectedIndexChanged);
				grpGSkillFilters->Controls->Add( gSkillFilters[ i ] );
				grpBSkillFilters->Controls->Add( bSkillFilters[ i ] );

				gSkills.Add( GetNewComboBox( 171, i ) );
				bSkills.Add( GetNewComboBox( 171, i ) );
				gSkills[ i ]->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSkill_SelectedIndexChanged);
				bSkills[ i ]->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSkill_SelectedIndexChanged);
				grpGSkills->Controls->Add( gSkills[ i ] );
				grpBSkills->Controls->Add( bSkills[ i ] );

				gIndexMaps.Add( gcnew IndexMap );
				bIndexMaps.Add( gcnew IndexMap );
			}
			charm_solution_map.Clear();
			cmbSort->SelectedIndex = 0;
			cmbCharms->SelectedIndex = 1;
			cmbCharmSelect->SelectedIndex = 3;
			cmbCharmTable->SelectedIndex = 0;
		}

	public:

		Form1(void) : CFG_FILE( L"settings.cfg" ), endl( L"\r\n" ), construction_complete( false ), MAX_LIMIT( 1000 )
		{
			//exitToolStripMenuItem_Click( nullptr, nullptr );
			//return;
			DoubleBuffered = true;
			language = -1;
			sort_off = false;
			updating_language = false;
			can_save = false;
			last_search_gunner = false;
			lock_related = false;
			InitializeComponent();
			InitializeComboBoxes();
			mnuNumResults->KeyPress += gcnew KeyPressEventHandler( this, &Form1::MaxResultsTextBoxKeyPress );
			mnuNumResults->TextChanged += gcnew EventHandler(  this, &Form1::MaxResultsTextChanged );
			can_save = true;
			adv_x = 1031;
			adv_y = 587;

			data = gcnew LoadedData();
			data->ImportTextFiles();
			CharmDatabase::GenerateCharmTable();
			LoadLanguages();

			InitFilters();
			InitSkills();
		
			lock_skills = false;
			btnCancel->Enabled = false;

			LoadConfig();

			CharmDatabase::LoadCustom();

			Text += " v" +  STRINGIZE( VERSION_NO );

			//Text = StaticString( Charms );

			construction_complete = true;
		}

		void LoadLanguages()
		{
			array< String^ >^ langs = IO::Directory::GetDirectories( L"Data/Languages" );
			for each( String^% lang in langs )
			{
				ToolStripMenuItem^ item = gcnew ToolStripMenuItem( lang->Substring( lang->LastIndexOf( L'\\' ) + 1 ) );
				item->Click += gcnew EventHandler( this, &Form1::LanguageSelect_Click );
				mnuLanguage->DropDownItems->Add( item );
			}
		}

		void AddSolution( String^ line, const unsigned version )
		{
			List_t< String^ > split;
			Utility::SplitString( %split, line, L' ' );
			Solution^ solution = gcnew Solution;
			for( unsigned i = 0; i < 5; ++i )
			{
				const int index = Convert::ToInt32( split[ i ] );
				if( index < 0 )
					solution->armors.Add( nullptr );
				else solution->armors.Add( Armor::static_armors[ i ][ index ] );
			}
			unsigned num_decorations = 0;
			if( version >= 4 )
			{
				const unsigned num_torso_decorations = Convert::ToUInt32( split[ 9 ] );
				const unsigned num_non_torso_decorations = Convert::ToUInt32( split[ 10 ] );
				num_decorations = num_torso_decorations + num_non_torso_decorations + 1;
				for( unsigned i = 0; i < num_torso_decorations; ++i )
					solution->body_decorations.Add( Decoration::static_decorations[ Convert::ToUInt32( split[ 11 + i ] ) ] );
				for( unsigned i = 0; i < num_non_torso_decorations; ++i )
					solution->non_body_decorations.Add( Decoration::static_decorations[ Convert::ToUInt32( split[ 11 + num_torso_decorations + i ] ) ] );
				solution->decorations.AddRange( %solution->body_decorations );
				solution->decorations.AddRange( %solution->non_body_decorations );
			}
			else
			{
				num_decorations = Convert::ToUInt32( split[ 9 ] );
				for( unsigned i = 0; i < num_decorations; ++i )
					solution->decorations.Add( Decoration::static_decorations[ Convert::ToUInt32( split[ 10 + i ] ) ] );
			}
			
			const unsigned num_skills = Convert::ToUInt32( split[ 10 + num_decorations ] );
			for( unsigned i = 0; i < num_skills; ++i )
				solution->extra_skills.Add( Skill::static_skills[ Convert::ToUInt32( split[ 11 + num_decorations + i ] ) ] );
			
			const unsigned charm_start = 11 + num_decorations + num_skills;
			if( charm_start < unsigned( split.Count ) )
			{
				solution->charm = gcnew Charm;
				solution->charm->num_slots = Convert::ToUInt32( split[ charm_start ] );
				const unsigned num_abilities = Convert::ToUInt32( split[ charm_start + 1 ] );
				for( unsigned i = 0; i < num_abilities; ++i )
				{
					Ability^ ability = Ability::static_abilities[ Convert::ToUInt32( split[ charm_start + 3 + i * 2 ] ) ];
					const int amount = Convert::ToInt32( split[ charm_start + 2 + i * 2 ] );
					solution->charm->abilities.Add( gcnew AbilityPair( ability, amount ) );
				}
			}

			solution->slots_spare = gcnew array< unsigned >( 4 );
			solution->CalculateData( int( nudHR->Value), int( nudElder->Value ) );
			solution->total_slots_spare = Convert::ToUInt32( split[ 5 ] );

			for( unsigned i = 1; i <= 3; ++i )
				solution->slots_spare[ i ] = Convert::ToUInt32( split[ 5 + i ] );
			if( solution->charm )
				AddSolution( solution->charm->GetName(), solution );
			else
			{
				no_charm_solutions.Add( solution );
				all_solutions.Add( solution );
			}
		}

		void LoadConfig( String^ file )
		{
			can_save = false;
			if( IO::File::Exists( file ) )
			{
				IO::StreamReader fin( file );
				String^ version_string = fin.ReadLine();
				const int version = Convert::ToInt32( version_string );
				
				if( version < 1 || version > 8 )
				{
					fin.Close();
					can_save = true;
					return;
				}
				else
				{
					language = -1;
					const int temp = Convert::ToUInt32( fin.ReadLine() );
					LanguageSelect_Click( mnuLanguage->DropDownItems[ temp ], nullptr );
					if( version >= 3 )
					{
						MAX_LIMIT = Convert::ToInt32( fin.ReadLine() );
						mnuNumResults->Text = L"" + MAX_LIMIT;
					}

					last_search_gunner = Convert::ToBoolean( fin.ReadLine() );
					const int hunter_type = Convert::ToInt32( fin.ReadLine() );
					rdoMale->Checked = Convert::ToBoolean( fin.ReadLine() );
					rdoFemale->Checked = !rdoMale->Checked;
					mnuAllowBadSkills->Checked = Convert::ToBoolean( fin.ReadLine() );
					mnuAllowPiercings->Checked = Convert::ToBoolean( fin.ReadLine() );
					mnuAllowEventArmor->Checked = Convert::ToBoolean( fin.ReadLine() );
					if( version >= 7 )
						mnuAllowJapaneseOnlyDLC->Checked = Convert::ToBoolean( fin.ReadLine() );
					mnuAllowLowerTierArmor->Checked = Convert::ToBoolean( fin.ReadLine() );
					if( version >= 4 )
						mnuPrintDecoNames->Checked = Convert::ToBoolean( fin.ReadLine() );
					if( version >= 5 )
						mnuSortSkillsAlphabetically->Checked = Convert::ToBoolean( fin.ReadLine() );
					cmbSort->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
					cmbCharmSelect->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
					if( version >= 2 )
						cmbCharmTable->SelectedIndex = Convert::ToInt32( fin.ReadLine() );

					nudHR->Value = Convert::ToInt32( fin.ReadLine() );
					nudElder->Value = Convert::ToInt32( fin.ReadLine() );
					nudWeaponSlots->Value = Convert::ToInt32( fin.ReadLine() );

					if( version >= 6 )
					{
						adv_x = Convert::ToInt32( fin.ReadLine() );
						adv_y = Convert::ToInt32( fin.ReadLine() );
					}

					tabHunterType->SuspendLayout();
					for( unsigned i = 0; i < NumSkills; ++i )
					{
						tabHunterType->SelectedIndex = 0;
						bSkillFilters[ i ]->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
						if( bSkillFilters[ i ]->SelectedIndex == 2 ) //related
						{
							bSkillFilters[ i ]->SelectedIndex = 0;
							bSkills[ i ]->SelectedIndex = SearchIndexMap( bIndexMaps[ i ], Convert::ToInt32( fin.ReadLine() ) );
							bSkillFilters[ i ]->SelectedIndex = 2;
						}
						else bSkills[ i ]->SelectedIndex = SearchIndexMap( bIndexMaps[ i ], Convert::ToInt32( fin.ReadLine() ) );
						
						tabHunterType->SelectedIndex = 1;
						gSkillFilters[ i ]->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
						if( gSkillFilters[ i ]->SelectedIndex == 2 ) //related
						{
							gSkillFilters[ i ]->SelectedIndex = 0;
							gSkills[ i ]->SelectedIndex = SearchIndexMap( gIndexMaps[ i ], Convert::ToInt32( fin.ReadLine() ) );
							gSkillFilters[ i ]->SelectedIndex = 2;
						}
						else gSkills[ i ]->SelectedIndex = SearchIndexMap( gIndexMaps[ i ], Convert::ToInt32( fin.ReadLine() ) );
					}
					tabHunterType->SelectedIndex = hunter_type;
					tabHunterType->ResumeLayout();

					FormulateQuery( false, last_search_gunner );

					charm_solution_map.Clear();
					all_solutions.Clear();
					no_charm_solutions.Clear();
					existing_armor.Clear();
					while( !fin.EndOfStream )
						AddSolution( fin.ReadLine(), version );
					last_result = nullptr;
					fin.Close();

					UpdateCharmComboBox( 1 );
				}	
			}
			else if( mnuLanguage->HasDropDownItems )
			{
				language = -1;

				String^ native = Globalization::CultureInfo::InstalledUICulture->Parent->NativeName->ToLower();
				const int spaceloc = native->IndexOf( L" " );
				if( spaceloc > 0 )
					native = native->Substring( 0, spaceloc );

				for each( ToolStripItem^ item in mnuLanguage->DropDownItems )
				{
					if( item->ToString()->ToLower()->IndexOf( native ) >= 0 )
					{
						LanguageSelect_Click( item, nullptr );
						break;
					}
				}

				if( language == -1 )
				{
					for each( ToolStripItem^ item in mnuLanguage->DropDownItems )
					{
						if( item->ToString()->IndexOf( L"English" ) >= 0 )
						{
							LanguageSelect_Click( item, nullptr );
							break;
						}
					}
				}
				if( language == -1 )
					LanguageSelect_Click( mnuLanguage->DropDownItems[ 0 ], nullptr );
			}
			can_save = true;
			SaveConfig();
		}

		void LoadConfig()
		{
			LoadConfig( CFG_FILE );
		}

		void SaveConfig( String^ file )
		{
			if( !can_save ) return;

			IO::StreamWriter fout( file );
			fout.WriteLine( L"8" );
			fout.WriteLine( language );
			fout.WriteLine( MAX_LIMIT );
			fout.WriteLine( last_search_gunner );
			fout.WriteLine( tabHunterType->SelectedIndex );
			fout.WriteLine( rdoMale->Checked );
			fout.WriteLine( mnuAllowBadSkills->Checked );
			fout.WriteLine( mnuAllowPiercings->Checked );
			fout.WriteLine( mnuAllowEventArmor->Checked );
			fout.WriteLine( mnuAllowJapaneseOnlyDLC->Checked );
			fout.WriteLine( mnuAllowLowerTierArmor->Checked );
			fout.WriteLine( mnuPrintDecoNames->Checked );
			fout.WriteLine( mnuSortSkillsAlphabetically->Checked );
			fout.WriteLine( cmbSort->SelectedIndex );
			fout.WriteLine( cmbCharmSelect->SelectedIndex );
			fout.WriteLine( cmbCharmTable->SelectedIndex );
			fout.WriteLine( nudHR->Value );
			fout.WriteLine( nudElder->Value );
			fout.WriteLine( nudWeaponSlots->Value );
			fout.WriteLine( adv_x );
			fout.WriteLine( adv_y );
			for( unsigned i = 0; i < NumSkills; ++i )
			{
				fout.WriteLine( bSkillFilters[ i ]->SelectedIndex );
				if( bSkills[ i ]->SelectedIndex == -1 )
					fout.WriteLine( -1 );
				else fout.WriteLine( bIndexMaps[ i ][ bSkills[ i ]->SelectedIndex ] );

				fout.WriteLine( gSkillFilters[ i ]->SelectedIndex );
				if( gSkills[ i ]->SelectedIndex == -1 )
					fout.WriteLine( -1 );
				else fout.WriteLine( gIndexMaps[ i ][ gSkills[ i ]->SelectedIndex ] );
			}
			
			for each( Solution^ solution in all_solutions )
			{
				for( unsigned i = 0; i < 5; ++i )
				{
					fout.Write( Convert::ToString( Utility::GetIndexOf( Armor::static_armors[ i ], solution->armors[ i ] ) ) );
					fout.Write( L" " );
				}
				fout.Write( Convert::ToString( solution->total_slots_spare ) );
				fout.Write( L" " );
				for( unsigned i = 1; i <= 3; ++i )
				{
					fout.Write( Convert::ToString( solution->slots_spare[ i ] ) );
					fout.Write( L" " );
				}
				fout.Write( Convert::ToString( solution->body_decorations.Count ) );
				fout.Write( L" " );
				fout.Write( Convert::ToString( solution->non_body_decorations.Count ) );
				fout.Write( L" " );
				for each( Decoration^ decoration in solution->body_decorations )
				{
					fout.Write( Convert::ToString( Utility::GetIndexOf( %Decoration::static_decorations, decoration ) ) );
					fout.Write( L" " );
				}
				for each( Decoration^ decoration in solution->non_body_decorations )
				{
					fout.Write( Convert::ToString( Utility::GetIndexOf( %Decoration::static_decorations, decoration ) ) );
					fout.Write( L" " );
				}
				fout.Write( Convert::ToString( solution->extra_skills.Count ) );
				fout.Write( L" " );
				for each( Skill^ skill in solution->extra_skills )
				{
					fout.Write( Convert::ToString( Utility::GetIndexOf( %Skill::static_skills, skill ) ) );
					fout.Write( L" " );
				}
				if( solution->charm )
				{
					fout.Write( Convert::ToString( solution->charm->num_slots ) );
					fout.Write( L" " );
					fout.Write( Convert::ToString( solution->charm->abilities.Count ) );
					fout.Write( L" " );
					for each( AbilityPair^ ap in solution->charm->abilities )
					{
						fout.Write( Convert::ToString( ap->amount ) );
						fout.Write( L" " );
						fout.Write( Convert::ToString( Utility::GetIndexOf( %Ability::static_abilities, ap->ability ) ) );
						fout.Write( L" " );
					}
				}

				fout.WriteLine();
			}
			fout.Close();
		}

		void SaveConfig()
		{
			SaveConfig( CFG_FILE );
		}

		int SearchIndexMap( IndexMap^ imap, int skill_index )
		{
			for each( Generic::KeyValuePair< unsigned, unsigned > kvp in imap )
			{
				if( kvp.Value == skill_index )
					return kvp.Key;
			}
			return -1;
		}

	protected:

		~Form1()
		{
			SaveConfig();
		}

	private: System::ComponentModel::IContainer^  components;


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->nudHR = (gcnew System::Windows::Forms::NumericUpDown());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->lblWhatIsCharmTable = (gcnew System::Windows::Forms::Label());
			this->lblCharmTable = (gcnew System::Windows::Forms::Label());
			this->cmbCharmTable = (gcnew System::Windows::Forms::ComboBox());
			this->nudWeaponSlots = (gcnew System::Windows::Forms::NumericUpDown());
			this->lblElder = (gcnew System::Windows::Forms::Label());
			this->lblSlots = (gcnew System::Windows::Forms::Label());
			this->nudElder = (gcnew System::Windows::Forms::NumericUpDown());
			this->lblHR = (gcnew System::Windows::Forms::Label());
			this->grpBSkills = (gcnew System::Windows::Forms::GroupBox());
			this->btnSearch = (gcnew System::Windows::Forms::Button());
			this->progressBar1 = (gcnew System::Windows::Forms::ProgressBar());
			this->txtSolutions = (gcnew System::Windows::Forms::RichTextBox());
			this->cmsSolutions = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->groupBox4 = (gcnew System::Windows::Forms::GroupBox());
			this->btnAdvancedSearch = (gcnew System::Windows::Forms::Button());
			this->btnCancel = (gcnew System::Windows::Forms::Button());
			this->grpResults = (gcnew System::Windows::Forms::GroupBox());
			this->btnCharms = (gcnew System::Windows::Forms::Button());
			this->grpBSkillFilters = (gcnew System::Windows::Forms::GroupBox());
			this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
			this->mnuFile = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuLoadData = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuSaveData = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuExit = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuOptions = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuClearSettings = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripSeparator1 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->mnuMaxResults = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuNumResults = (gcnew System::Windows::Forms::ToolStripTextBox());
			this->mnuAllowBadSkills = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuAllowPiercings = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuAllowEventArmor = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuAllowJapaneseOnlyDLC = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuAllowLowerTierArmor = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuPrintDecoNames = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuSortSkillsAlphabetically = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuLanguage = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuHelp = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuCheckForUpdates = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuAbout = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->groupBox6 = (gcnew System::Windows::Forms::GroupBox());
			this->rdoFemale = (gcnew System::Windows::Forms::RadioButton());
			this->rdoMale = (gcnew System::Windows::Forms::RadioButton());
			this->tabHunterType = (gcnew System::Windows::Forms::TabControl());
			this->tabBlademaster = (gcnew System::Windows::Forms::TabPage());
			this->tabGunner = (gcnew System::Windows::Forms::TabPage());
			this->grpGSkillFilters = (gcnew System::Windows::Forms::GroupBox());
			this->grpGSkills = (gcnew System::Windows::Forms::GroupBox());
			this->grpSort = (gcnew System::Windows::Forms::GroupBox());
			this->cmbSort = (gcnew System::Windows::Forms::ComboBox());
			this->grpCharmFilter = (gcnew System::Windows::Forms::GroupBox());
			this->cmbCharms = (gcnew System::Windows::Forms::ComboBox());
			this->grpCharms = (gcnew System::Windows::Forms::GroupBox());
			this->btnImport = (gcnew System::Windows::Forms::Button());
			this->cmbCharmSelect = (gcnew System::Windows::Forms::ComboBox());
			this->cmsCharms = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudHR))->BeginInit();
			this->groupBox1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudWeaponSlots))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudElder))->BeginInit();
			this->groupBox4->SuspendLayout();
			this->grpResults->SuspendLayout();
			this->menuStrip1->SuspendLayout();
			this->groupBox6->SuspendLayout();
			this->tabHunterType->SuspendLayout();
			this->tabBlademaster->SuspendLayout();
			this->tabGunner->SuspendLayout();
			this->grpSort->SuspendLayout();
			this->grpCharmFilter->SuspendLayout();
			this->grpCharms->SuspendLayout();
			this->SuspendLayout();
			// 
			// nudHR
			// 
			this->nudHR->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->nudHR->Location = System::Drawing::Point(109, 20);
			this->nudHR->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {8, 0, 0, 0});
			this->nudHR->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 0});
			this->nudHR->Name = L"nudHR";
			this->nudHR->Size = System::Drawing::Size(35, 20);
			this->nudHR->TabIndex = 2;
			this->nudHR->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {8, 0, 0, 0});
			this->nudHR->ValueChanged += gcnew System::EventHandler(this, &Form1::HRChanged);
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->lblWhatIsCharmTable);
			this->groupBox1->Controls->Add(this->lblCharmTable);
			this->groupBox1->Controls->Add(this->cmbCharmTable);
			this->groupBox1->Controls->Add(this->nudWeaponSlots);
			this->groupBox1->Controls->Add(this->nudHR);
			this->groupBox1->Controls->Add(this->lblElder);
			this->groupBox1->Controls->Add(this->lblSlots);
			this->groupBox1->Controls->Add(this->nudElder);
			this->groupBox1->Controls->Add(this->lblHR);
			this->groupBox1->Location = System::Drawing::Point(12, 27);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(154, 140);
			this->groupBox1->TabIndex = 5;
			this->groupBox1->TabStop = false;
			// 
			// lblWhatIsCharmTable
			// 
			this->lblWhatIsCharmTable->AutoSize = true;
			this->lblWhatIsCharmTable->BackColor = System::Drawing::SystemColors::Control;
			this->lblWhatIsCharmTable->Cursor = System::Windows::Forms::Cursors::Hand;
			this->lblWhatIsCharmTable->ForeColor = System::Drawing::SystemColors::HotTrack;
			this->lblWhatIsCharmTable->Location = System::Drawing::Point(75, 111);
			this->lblWhatIsCharmTable->Name = L"lblWhatIsCharmTable";
			this->lblWhatIsCharmTable->Size = System::Drawing::Size(13, 13);
			this->lblWhatIsCharmTable->TabIndex = 9;
			this->lblWhatIsCharmTable->Text = L"\?";
			this->lblWhatIsCharmTable->Click += gcnew System::EventHandler(this, &Form1::lblWhatIsCharmTable_Click);
			// 
			// lblCharmTable
			// 
			this->lblCharmTable->AutoSize = true;
			this->lblCharmTable->Location = System::Drawing::Point(11, 111);
			this->lblCharmTable->Name = L"lblCharmTable";
			this->lblCharmTable->Size = System::Drawing::Size(67, 13);
			this->lblCharmTable->TabIndex = 8;
			this->lblCharmTable->Text = L"Charm Table";
			// 
			// cmbCharmTable
			// 
			this->cmbCharmTable->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->cmbCharmTable->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbCharmTable->FormattingEnabled = true;
			this->cmbCharmTable->Items->AddRange(gcnew cli::array< System::Object^  >(19) {L"\?\?\?", L"   1", L"   2", L"   3", L"   4", 
				L"   5", L"   6", L"   7", L"   8", L"   9", L"  10", L"  11", L"  12", L"  13", L"  14", L"  15", L"  16", L"  17", L"All"});
			this->cmbCharmTable->Location = System::Drawing::Point(101, 108);
			this->cmbCharmTable->Name = L"cmbCharmTable";
			this->cmbCharmTable->Size = System::Drawing::Size(43, 21);
			this->cmbCharmTable->TabIndex = 1;
			this->cmbCharmTable->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbCharmTable_SelectedIndexChanged);
			// 
			// nudWeaponSlots
			// 
			this->nudWeaponSlots->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->nudWeaponSlots->Location = System::Drawing::Point(109, 80);
			this->nudWeaponSlots->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {3, 0, 0, 0});
			this->nudWeaponSlots->Name = L"nudWeaponSlots";
			this->nudWeaponSlots->Size = System::Drawing::Size(35, 20);
			this->nudWeaponSlots->TabIndex = 7;
			// 
			// lblElder
			// 
			this->lblElder->AutoSize = true;
			this->lblElder->Location = System::Drawing::Point(11, 52);
			this->lblElder->Name = L"lblElder";
			this->lblElder->Size = System::Drawing::Size(74, 13);
			this->lblElder->TabIndex = 4;
			this->lblElder->Text = L"Village Quests";
			// 
			// lblSlots
			// 
			this->lblSlots->AutoSize = true;
			this->lblSlots->Location = System::Drawing::Point(11, 82);
			this->lblSlots->Name = L"lblSlots";
			this->lblSlots->Size = System::Drawing::Size(97, 13);
			this->lblSlots->TabIndex = 1;
			this->lblSlots->Text = L"Max Weapon Slots";
			// 
			// nudElder
			// 
			this->nudElder->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->nudElder->Location = System::Drawing::Point(109, 50);
			this->nudElder->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {10, 0, 0, 0});
			this->nudElder->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 0});
			this->nudElder->Name = L"nudElder";
			this->nudElder->Size = System::Drawing::Size(35, 20);
			this->nudElder->TabIndex = 5;
			this->nudElder->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {10, 0, 0, 0});
			this->nudElder->ValueChanged += gcnew System::EventHandler(this, &Form1::DeleteOptions);
			// 
			// lblHR
			// 
			this->lblHR->AutoSize = true;
			this->lblHR->Location = System::Drawing::Point(11, 22);
			this->lblHR->Name = L"lblHR";
			this->lblHR->Size = System::Drawing::Size(75, 13);
			this->lblHR->TabIndex = 6;
			this->lblHR->Text = L"Harbor Quests";
			// 
			// grpBSkills
			// 
			this->grpBSkills->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left));
			this->grpBSkills->Location = System::Drawing::Point(6, 8);
			this->grpBSkills->Name = L"grpBSkills";
			this->grpBSkills->Size = System::Drawing::Size(183, 182);
			this->grpBSkills->TabIndex = 8;
			this->grpBSkills->TabStop = false;
			this->grpBSkills->Text = L"Skills";
			// 
			// btnSearch
			// 
			this->btnSearch->Location = System::Drawing::Point(6, 12);
			this->btnSearch->Name = L"btnSearch";
			this->btnSearch->Size = System::Drawing::Size(107, 27);
			this->btnSearch->TabIndex = 9;
			this->btnSearch->Text = L"&Quick Search";
			this->btnSearch->UseVisualStyleBackColor = true;
			this->btnSearch->Click += gcnew System::EventHandler(this, &Form1::btnSearch_Click);
			// 
			// progressBar1
			// 
			this->progressBar1->Location = System::Drawing::Point(14, 492);
			this->progressBar1->Name = L"progressBar1";
			this->progressBar1->Size = System::Drawing::Size(353, 10);
			this->progressBar1->Step = 1;
			this->progressBar1->TabIndex = 10;
			// 
			// txtSolutions
			// 
			this->txtSolutions->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->txtSolutions->ContextMenuStrip = this->cmsSolutions;
			this->txtSolutions->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->txtSolutions->Location = System::Drawing::Point(6, 16);
			this->txtSolutions->Name = L"txtSolutions";
			this->txtSolutions->ReadOnly = true;
			this->txtSolutions->ScrollBars = System::Windows::Forms::RichTextBoxScrollBars::Vertical;
			this->txtSolutions->Size = System::Drawing::Size(332, 451);
			this->txtSolutions->TabIndex = 11;
			this->txtSolutions->Text = L"";
			this->txtSolutions->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &Form1::KeyDown);
			// 
			// cmsSolutions
			// 
			this->cmsSolutions->Name = L"contextMenuStrip1";
			this->cmsSolutions->Size = System::Drawing::Size(61, 4);
			this->cmsSolutions->Opening += gcnew System::ComponentModel::CancelEventHandler(this, &Form1::contextMenuStrip1_Opening);
			// 
			// groupBox4
			// 
			this->groupBox4->Controls->Add(this->btnAdvancedSearch);
			this->groupBox4->Controls->Add(this->btnCancel);
			this->groupBox4->Controls->Add(this->btnSearch);
			this->groupBox4->Location = System::Drawing::Point(14, 440);
			this->groupBox4->Name = L"groupBox4";
			this->groupBox4->Size = System::Drawing::Size(353, 46);
			this->groupBox4->TabIndex = 14;
			this->groupBox4->TabStop = false;
			// 
			// btnAdvancedSearch
			// 
			this->btnAdvancedSearch->Location = System::Drawing::Point(119, 12);
			this->btnAdvancedSearch->Name = L"btnAdvancedSearch";
			this->btnAdvancedSearch->Size = System::Drawing::Size(137, 27);
			this->btnAdvancedSearch->TabIndex = 11;
			this->btnAdvancedSearch->Text = L"&Advanced Search";
			this->btnAdvancedSearch->UseVisualStyleBackColor = true;
			this->btnAdvancedSearch->Click += gcnew System::EventHandler(this, &Form1::btnAdvancedSearch_Click);
			// 
			// btnCancel
			// 
			this->btnCancel->Location = System::Drawing::Point(262, 12);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(85, 27);
			this->btnCancel->TabIndex = 10;
			this->btnCancel->Text = L"&Cancel";
			this->btnCancel->UseVisualStyleBackColor = true;
			this->btnCancel->Click += gcnew System::EventHandler(this, &Form1::btnCancel_Click);
			// 
			// grpResults
			// 
			this->grpResults->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->grpResults->Controls->Add(this->txtSolutions);
			this->grpResults->Location = System::Drawing::Point(373, 27);
			this->grpResults->Name = L"grpResults";
			this->grpResults->Size = System::Drawing::Size(344, 477);
			this->grpResults->TabIndex = 15;
			this->grpResults->TabStop = false;
			this->grpResults->Text = L"Results";
			// 
			// btnCharms
			// 
			this->btnCharms->Location = System::Drawing::Point(5, 46);
			this->btnCharms->Name = L"btnCharms";
			this->btnCharms->Size = System::Drawing::Size(89, 23);
			this->btnCharms->TabIndex = 25;
			this->btnCharms->Text = L"&My Charms";
			this->btnCharms->UseVisualStyleBackColor = true;
			this->btnCharms->Click += gcnew System::EventHandler(this, &Form1::btnCharms_Click);
			// 
			// grpBSkillFilters
			// 
			this->grpBSkillFilters->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left));
			this->grpBSkillFilters->Location = System::Drawing::Point(195, 8);
			this->grpBSkillFilters->Name = L"grpBSkillFilters";
			this->grpBSkillFilters->Size = System::Drawing::Size(146, 182);
			this->grpBSkillFilters->TabIndex = 9;
			this->grpBSkillFilters->TabStop = false;
			this->grpBSkillFilters->Text = L"Skill Filters";
			// 
			// menuStrip1
			// 
			this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->mnuFile, this->mnuOptions, 
				this->mnuLanguage, this->mnuHelp});
			this->menuStrip1->Location = System::Drawing::Point(0, 0);
			this->menuStrip1->Name = L"menuStrip1";
			this->menuStrip1->Size = System::Drawing::Size(729, 24);
			this->menuStrip1->TabIndex = 16;
			this->menuStrip1->Text = L"menuStrip1";
			// 
			// mnuFile
			// 
			this->mnuFile->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {this->mnuLoadData, 
				this->mnuSaveData, this->mnuExit});
			this->mnuFile->Name = L"mnuFile";
			this->mnuFile->Size = System::Drawing::Size(37, 20);
			this->mnuFile->Text = L"&File";
			// 
			// mnuLoadData
			// 
			this->mnuLoadData->Name = L"mnuLoadData";
			this->mnuLoadData->Size = System::Drawing::Size(100, 22);
			this->mnuLoadData->Text = L"&Load";
			this->mnuLoadData->Click += gcnew System::EventHandler(this, &Form1::mnuLoad_Click);
			// 
			// mnuSaveData
			// 
			this->mnuSaveData->Name = L"mnuSaveData";
			this->mnuSaveData->Size = System::Drawing::Size(100, 22);
			this->mnuSaveData->Text = L"&Save";
			this->mnuSaveData->Click += gcnew System::EventHandler(this, &Form1::mnuSave_Click);
			// 
			// mnuExit
			// 
			this->mnuExit->Name = L"mnuExit";
			this->mnuExit->Size = System::Drawing::Size(100, 22);
			this->mnuExit->Text = L"E&xit";
			this->mnuExit->Click += gcnew System::EventHandler(this, &Form1::exitToolStripMenuItem_Click);
			// 
			// mnuOptions
			// 
			this->mnuOptions->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(10) {this->mnuClearSettings, 
				this->toolStripSeparator1, this->mnuMaxResults, this->mnuAllowBadSkills, this->mnuAllowPiercings, this->mnuAllowEventArmor, this->mnuAllowJapaneseOnlyDLC, 
				this->mnuAllowLowerTierArmor, this->mnuPrintDecoNames, this->mnuSortSkillsAlphabetically});
			this->mnuOptions->Name = L"mnuOptions";
			this->mnuOptions->Size = System::Drawing::Size(61, 20);
			this->mnuOptions->Text = L"&Options";
			// 
			// mnuClearSettings
			// 
			this->mnuClearSettings->Name = L"mnuClearSettings";
			this->mnuClearSettings->Size = System::Drawing::Size(209, 22);
			this->mnuClearSettings->Text = L"&Clear Settings";
			this->mnuClearSettings->Click += gcnew System::EventHandler(this, &Form1::toolStripMenuItem1_Click);
			// 
			// toolStripSeparator1
			// 
			this->toolStripSeparator1->Name = L"toolStripSeparator1";
			this->toolStripSeparator1->Size = System::Drawing::Size(206, 6);
			// 
			// mnuMaxResults
			// 
			this->mnuMaxResults->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->mnuNumResults});
			this->mnuMaxResults->Name = L"mnuMaxResults";
			this->mnuMaxResults->Size = System::Drawing::Size(209, 22);
			this->mnuMaxResults->Text = L"&Max Results";
			// 
			// mnuNumResults
			// 
			this->mnuNumResults->Name = L"mnuNumResults";
			this->mnuNumResults->Size = System::Drawing::Size(100, 23);
			this->mnuNumResults->Text = L"1000";
			// 
			// mnuAllowBadSkills
			// 
			this->mnuAllowBadSkills->CheckOnClick = true;
			this->mnuAllowBadSkills->Name = L"mnuAllowBadSkills";
			this->mnuAllowBadSkills->Size = System::Drawing::Size(209, 22);
			this->mnuAllowBadSkills->Text = L"Allow &Bad Skills";
			this->mnuAllowBadSkills->Click += gcnew System::EventHandler(this, &Form1::OptionsChanged);
			// 
			// mnuAllowPiercings
			// 
			this->mnuAllowPiercings->CheckOnClick = true;
			this->mnuAllowPiercings->Name = L"mnuAllowPiercings";
			this->mnuAllowPiercings->Size = System::Drawing::Size(209, 22);
			this->mnuAllowPiercings->Text = L"Allow &Piercings";
			this->mnuAllowPiercings->Click += gcnew System::EventHandler(this, &Form1::OptionsChanged);
			// 
			// mnuAllowEventArmor
			// 
			this->mnuAllowEventArmor->Checked = true;
			this->mnuAllowEventArmor->CheckOnClick = true;
			this->mnuAllowEventArmor->CheckState = System::Windows::Forms::CheckState::Checked;
			this->mnuAllowEventArmor->Name = L"mnuAllowEventArmor";
			this->mnuAllowEventArmor->Size = System::Drawing::Size(209, 22);
			this->mnuAllowEventArmor->Text = L"Allow &Event Armor";
			this->mnuAllowEventArmor->Click += gcnew System::EventHandler(this, &Form1::OptionsChanged);
			// 
			// mnuAllowJapaneseOnlyDLC
			// 
			this->mnuAllowJapaneseOnlyDLC->CheckOnClick = true;
			this->mnuAllowJapaneseOnlyDLC->Name = L"mnuAllowJapaneseOnlyDLC";
			this->mnuAllowJapaneseOnlyDLC->Size = System::Drawing::Size(209, 22);
			this->mnuAllowJapaneseOnlyDLC->Text = L"Allow &Japanese-Only DLC";
			// 
			// mnuAllowLowerTierArmor
			// 
			this->mnuAllowLowerTierArmor->Checked = true;
			this->mnuAllowLowerTierArmor->CheckOnClick = true;
			this->mnuAllowLowerTierArmor->CheckState = System::Windows::Forms::CheckState::Checked;
			this->mnuAllowLowerTierArmor->Name = L"mnuAllowLowerTierArmor";
			this->mnuAllowLowerTierArmor->Size = System::Drawing::Size(209, 22);
			this->mnuAllowLowerTierArmor->Text = L"Allow &Lower Tier Armor";
			this->mnuAllowLowerTierArmor->Click += gcnew System::EventHandler(this, &Form1::OptionsChanged);
			// 
			// mnuPrintDecoNames
			// 
			this->mnuPrintDecoNames->Checked = true;
			this->mnuPrintDecoNames->CheckOnClick = true;
			this->mnuPrintDecoNames->CheckState = System::Windows::Forms::CheckState::Checked;
			this->mnuPrintDecoNames->Name = L"mnuPrintDecoNames";
			this->mnuPrintDecoNames->Size = System::Drawing::Size(209, 22);
			this->mnuPrintDecoNames->Text = L"Print &Decoration Names";
			this->mnuPrintDecoNames->Click += gcnew System::EventHandler(this, &Form1::OptionsChanged);
			// 
			// mnuSortSkillsAlphabetically
			// 
			this->mnuSortSkillsAlphabetically->Checked = true;
			this->mnuSortSkillsAlphabetically->CheckOnClick = true;
			this->mnuSortSkillsAlphabetically->CheckState = System::Windows::Forms::CheckState::Checked;
			this->mnuSortSkillsAlphabetically->Name = L"mnuSortSkillsAlphabetically";
			this->mnuSortSkillsAlphabetically->Size = System::Drawing::Size(209, 22);
			this->mnuSortSkillsAlphabetically->Text = L"&Sort Skills Alphabetically";
			this->mnuSortSkillsAlphabetically->Click += gcnew System::EventHandler(this, &Form1::OptionsChanged);
			// 
			// mnuLanguage
			// 
			this->mnuLanguage->Name = L"mnuLanguage";
			this->mnuLanguage->Size = System::Drawing::Size(71, 20);
			this->mnuLanguage->Text = L"&Language";
			// 
			// mnuHelp
			// 
			this->mnuHelp->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->mnuCheckForUpdates, 
				this->mnuAbout});
			this->mnuHelp->Name = L"mnuHelp";
			this->mnuHelp->Size = System::Drawing::Size(44, 20);
			this->mnuHelp->Text = L"&Help";
			// 
			// mnuCheckForUpdates
			// 
			this->mnuCheckForUpdates->Name = L"mnuCheckForUpdates";
			this->mnuCheckForUpdates->Size = System::Drawing::Size(171, 22);
			this->mnuCheckForUpdates->Text = L"Check for &Updates";
			this->mnuCheckForUpdates->Click += gcnew System::EventHandler(this, &Form1::UpdateMenuItem_Click);
			// 
			// mnuAbout
			// 
			this->mnuAbout->Name = L"mnuAbout";
			this->mnuAbout->Size = System::Drawing::Size(171, 22);
			this->mnuAbout->Text = L"&About";
			this->mnuAbout->Click += gcnew System::EventHandler(this, &Form1::aboutToolStripMenuItem_Click);
			// 
			// groupBox6
			// 
			this->groupBox6->Controls->Add(this->rdoFemale);
			this->groupBox6->Controls->Add(this->rdoMale);
			this->groupBox6->Location = System::Drawing::Point(12, 166);
			this->groupBox6->Name = L"groupBox6";
			this->groupBox6->Size = System::Drawing::Size(154, 40);
			this->groupBox6->TabIndex = 19;
			this->groupBox6->TabStop = false;
			// 
			// rdoFemale
			// 
			this->rdoFemale->AutoSize = true;
			this->rdoFemale->Checked = true;
			this->rdoFemale->Location = System::Drawing::Point(78, 15);
			this->rdoFemale->Name = L"rdoFemale";
			this->rdoFemale->Size = System::Drawing::Size(59, 17);
			this->rdoFemale->TabIndex = 18;
			this->rdoFemale->TabStop = true;
			this->rdoFemale->Text = L"Female";
			this->rdoFemale->UseVisualStyleBackColor = true;
			this->rdoFemale->CheckedChanged += gcnew System::EventHandler(this, &Form1::DeleteOptions);
			// 
			// rdoMale
			// 
			this->rdoMale->AutoSize = true;
			this->rdoMale->Location = System::Drawing::Point(15, 15);
			this->rdoMale->Name = L"rdoMale";
			this->rdoMale->Size = System::Drawing::Size(48, 17);
			this->rdoMale->TabIndex = 0;
			this->rdoMale->Text = L"Male";
			this->rdoMale->UseVisualStyleBackColor = true;
			this->rdoMale->CheckedChanged += gcnew System::EventHandler(this, &Form1::DeleteOptions);
			// 
			// tabHunterType
			// 
			this->tabHunterType->Controls->Add(this->tabBlademaster);
			this->tabHunterType->Controls->Add(this->tabGunner);
			this->tabHunterType->Location = System::Drawing::Point(12, 212);
			this->tabHunterType->Name = L"tabHunterType";
			this->tabHunterType->SelectedIndex = 0;
			this->tabHunterType->Size = System::Drawing::Size(355, 222);
			this->tabHunterType->TabIndex = 24;
			// 
			// tabBlademaster
			// 
			this->tabBlademaster->BackColor = System::Drawing::SystemColors::Control;
			this->tabBlademaster->Controls->Add(this->grpBSkills);
			this->tabBlademaster->Controls->Add(this->grpBSkillFilters);
			this->tabBlademaster->Location = System::Drawing::Point(4, 22);
			this->tabBlademaster->Name = L"tabBlademaster";
			this->tabBlademaster->Padding = System::Windows::Forms::Padding(3);
			this->tabBlademaster->Size = System::Drawing::Size(347, 196);
			this->tabBlademaster->TabIndex = 0;
			this->tabBlademaster->Text = L"Blademaster";
			// 
			// tabGunner
			// 
			this->tabGunner->BackColor = System::Drawing::SystemColors::Control;
			this->tabGunner->Controls->Add(this->grpGSkillFilters);
			this->tabGunner->Controls->Add(this->grpGSkills);
			this->tabGunner->Location = System::Drawing::Point(4, 22);
			this->tabGunner->Name = L"tabGunner";
			this->tabGunner->Padding = System::Windows::Forms::Padding(3);
			this->tabGunner->Size = System::Drawing::Size(347, 196);
			this->tabGunner->TabIndex = 1;
			this->tabGunner->Text = L"Gunner";
			// 
			// grpGSkillFilters
			// 
			this->grpGSkillFilters->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left));
			this->grpGSkillFilters->Location = System::Drawing::Point(195, 8);
			this->grpGSkillFilters->Name = L"grpGSkillFilters";
			this->grpGSkillFilters->Size = System::Drawing::Size(146, 182);
			this->grpGSkillFilters->TabIndex = 10;
			this->grpGSkillFilters->TabStop = false;
			this->grpGSkillFilters->Text = L"Skill Filters";
			// 
			// grpGSkills
			// 
			this->grpGSkills->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left));
			this->grpGSkills->Location = System::Drawing::Point(6, 8);
			this->grpGSkills->Name = L"grpGSkills";
			this->grpGSkills->Size = System::Drawing::Size(183, 182);
			this->grpGSkills->TabIndex = 9;
			this->grpGSkills->TabStop = false;
			this->grpGSkills->Text = L"Skills";
			// 
			// grpSort
			// 
			this->grpSort->Controls->Add(this->cmbSort);
			this->grpSort->Location = System::Drawing::Point(172, 27);
			this->grpSort->Name = L"grpSort";
			this->grpSort->Size = System::Drawing::Size(193, 45);
			this->grpSort->TabIndex = 0;
			this->grpSort->TabStop = false;
			this->grpSort->Text = L"Sort Results By";
			// 
			// cmbSort
			// 
			this->cmbSort->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->cmbSort->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbSort->FormattingEnabled = true;
			this->cmbSort->Items->AddRange(gcnew cli::array< System::Object^  >(13) {L"None", L"Dragon res", L"Fire res", L"Ice res", 
				L"Thunder res", L"Water res", L"Base defence", L"Max defence", L"Difficulty", L"Rarity", L"Slots spare", L"Family", L"Extra Skills"});
			this->cmbSort->Location = System::Drawing::Point(6, 16);
			this->cmbSort->Name = L"cmbSort";
			this->cmbSort->Size = System::Drawing::Size(181, 21);
			this->cmbSort->TabIndex = 0;
			this->cmbSort->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSort_SelectedIndexChanged);
			// 
			// grpCharmFilter
			// 
			this->grpCharmFilter->Controls->Add(this->cmbCharms);
			this->grpCharmFilter->Location = System::Drawing::Point(172, 78);
			this->grpCharmFilter->Name = L"grpCharmFilter";
			this->grpCharmFilter->Size = System::Drawing::Size(193, 45);
			this->grpCharmFilter->TabIndex = 1;
			this->grpCharmFilter->TabStop = false;
			this->grpCharmFilter->Text = L"Filter Results by Charm";
			// 
			// cmbCharms
			// 
			this->cmbCharms->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->cmbCharms->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbCharms->FormattingEnabled = true;
			this->cmbCharms->Items->AddRange(gcnew cli::array< System::Object^  >(2) {L"None", L"All"});
			this->cmbCharms->Location = System::Drawing::Point(6, 16);
			this->cmbCharms->Name = L"cmbCharms";
			this->cmbCharms->Size = System::Drawing::Size(181, 21);
			this->cmbCharms->TabIndex = 0;
			this->cmbCharms->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbCharms_SelectedIndexChanged);
			// 
			// grpCharms
			// 
			this->grpCharms->Controls->Add(this->btnImport);
			this->grpCharms->Controls->Add(this->cmbCharmSelect);
			this->grpCharms->Controls->Add(this->btnCharms);
			this->grpCharms->Location = System::Drawing::Point(172, 129);
			this->grpCharms->Name = L"grpCharms";
			this->grpCharms->Size = System::Drawing::Size(193, 77);
			this->grpCharms->TabIndex = 25;
			this->grpCharms->TabStop = false;
			this->grpCharms->Text = L"Charms";
			// 
			// btnImport
			// 
			this->btnImport->Location = System::Drawing::Point(99, 46);
			this->btnImport->Name = L"btnImport";
			this->btnImport->Size = System::Drawing::Size(89, 23);
			this->btnImport->TabIndex = 26;
			this->btnImport->Text = L"&Import";
			this->btnImport->UseVisualStyleBackColor = true;
			this->btnImport->Click += gcnew System::EventHandler(this, &Form1::btnImport_Click);
			// 
			// cmbCharmSelect
			// 
			this->cmbCharmSelect->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->cmbCharmSelect->ContextMenuStrip = this->cmsCharms;
			this->cmbCharmSelect->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbCharmSelect->FormattingEnabled = true;
			this->cmbCharmSelect->Items->AddRange(gcnew cli::array< System::Object^  >(5) {L"Use no charms", L"Use my charms", L"Use only slotted charms", 
				L"Use up to one skill charms", L"Use up to two skill charms"});
			this->cmbCharmSelect->Location = System::Drawing::Point(6, 19);
			this->cmbCharmSelect->Name = L"cmbCharmSelect";
			this->cmbCharmSelect->Size = System::Drawing::Size(181, 21);
			this->cmbCharmSelect->TabIndex = 1;
			// 
			// cmsCharms
			// 
			this->cmsCharms->Name = L"cmsCharms";
			this->cmsCharms->Size = System::Drawing::Size(61, 4);
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(729, 509);
			this->Controls->Add(this->grpCharmFilter);
			this->Controls->Add(this->grpSort);
			this->Controls->Add(this->grpResults);
			this->Controls->Add(this->groupBox1);
			this->Controls->Add(this->menuStrip1);
			this->Controls->Add(this->grpCharms);
			this->Controls->Add(this->groupBox6);
			this->Controls->Add(this->tabHunterType);
			this->Controls->Add(this->groupBox4);
			this->Controls->Add(this->progressBar1);
			this->Name = L"Form1";
			this->Text = L"Athena\'s ASS for MH3G and MH3U";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudHR))->EndInit();
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudWeaponSlots))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudElder))->EndInit();
			this->groupBox4->ResumeLayout(false);
			this->grpResults->ResumeLayout(false);
			this->menuStrip1->ResumeLayout(false);
			this->menuStrip1->PerformLayout();
			this->groupBox6->ResumeLayout(false);
			this->groupBox6->PerformLayout();
			this->tabHunterType->ResumeLayout(false);
			this->tabBlademaster->ResumeLayout(false);
			this->tabGunner->ResumeLayout(false);
			this->grpSort->ResumeLayout(false);
			this->grpCharmFilter->ResumeLayout(false);
			this->grpCharms->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

private: 
	System::Void FormulateQuery( const bool danger, const bool use_gunner_skills )
	{
		query = gcnew Query;
		for( int i = 0; i < int( Armor::ArmorType::NumArmorTypes ); ++i )
		{
			query->rel_armor.Add( gcnew List_t< Armor^ >() );
			query->inf_armor.Add( gcnew List_t< Armor^ >() );
		}
		query->weapon_slots_allowed = int( nudWeaponSlots->Value );
		query->elder_star = int( nudElder->Value );
		query->hr = int( nudHR->Value );
		query->charm_table = cmbCharmTable->SelectedIndex;

		query->gender = rdoMale->Checked ? Gender::MALE : Gender::FEMALE;
		query->hunter_type = !use_gunner_skills ? HunterType::BLADEMASTER : HunterType::GUNNER;
		query->include_piercings = mnuAllowPiercings->Checked;
		query->allow_bad = mnuAllowBadSkills->Checked;
		query->allow_event = mnuAllowEventArmor->Checked;
		query->allow_japs = mnuAllowJapaneseOnlyDLC->Checked;
		query->allow_lower_tier = mnuAllowLowerTierArmor->Checked;
		
		if( !use_gunner_skills )
		{
			for( unsigned i = 0; i < NumSkills; ++i )
				if( bSkills[ i ]->SelectedIndex >= 0 )
					query->skills.Add( data->FindSkill( bIndexMaps[ i ][ bSkills[ i ]->SelectedIndex ] ) );
		}
		else
		{
			for( unsigned i = 0; i < NumSkills; ++i )
				if( gSkills[ i ]->SelectedIndex >= 0 )
					query->skills.Add( data->FindSkill( gIndexMaps[ i ][ gSkills[ i ]->SelectedIndex ] ) );
		}
		data->GetRelevantData( query );
	}

	System::Void MaxResultsTextBoxKeyPress( System::Object^ sender, KeyPressEventArgs^ e )
	{
		//ignore all keypresses that aren't numeric digits or control chars
		if( !Char::IsControl( e->KeyChar ) &&
			!Char::IsDigit( e->KeyChar ) )
			e->Handled = true;
	}

	System::Void MaxResultsTextChanged( System::Object^ sender, EventArgs^ e )
	{
		try
		{
			MAX_LIMIT = Convert::ToUInt32( mnuNumResults->Text );
		}
		catch( Exception^ )
		{
			return;
		}
		SaveConfig();
	}

	System::Void OptionsChanged( System::Object^ sender, System::EventArgs^ e )
	{
		SaveConfig();
		if( sender != mnuAllowBadSkills )
			DeleteOptions( sender, e );
		if( sender == mnuPrintDecoNames )
			cmbCharms_SelectedIndexChanged( nullptr, nullptr );
		if( sender == mnuSortSkillsAlphabetically )
		{
			for each( ComboBox^ cb in bSkillFilters )
				cmbSkillFilter_SelectedIndexChanged( cb, nullptr );
			for each( ComboBox^ cb in gSkillFilters )
				cmbSkillFilter_SelectedIndexChanged( cb, nullptr );
		}
	}

	/*void QueueTask( Query^ query, Charm^ ct, Armor^ helm )
	{
		BackgroundWorker^ new_thread = gcnew BackgroundWorker;
		new_thread->WorkerSupportsCancellation = true;
		new_thread->WorkerReportsProgress = true;
		new_thread->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &Form1::backgroundWorker1_DoWork);
		new_thread->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &Form1::backgroundWorker1_RunWorkerCompleted);
		new_thread->ProgressChanged += gcnew System::ComponentModel::ProgressChangedEventHandler(this, &Form1::backgroundWorker1_ProgressChanged);

		ThreadSearchData^ tsd = gcnew ThreadSearchData;
		tsd->charm_template = ct;
		tsd->query = query;
		tsd->helm = helm;
		worker_data.Add( tsd );

		workers.Add( new_thread );
	}*/

	System::Void QueueTask( Query^ query, Charm^ ct )
	{
		/*if( query->rel_armor[ int( Armor::ArmorType::HEAD ) ]->Count == 0 )
		{
			QueueTask( query, ct, nullptr );
		}
		else
		{
			for each( Armor^ helm in query->rel_armor[ int( Armor::ArmorType::HEAD ) ] )
			{
				QueueTask( query, ct, helm );
			}
		}*/
		
		BackgroundWorker^ new_thread = gcnew BackgroundWorker;
		new_thread->WorkerSupportsCancellation = true;
		new_thread->WorkerReportsProgress = true;
		new_thread->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &Form1::backgroundWorker1_DoWork);
		new_thread->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &Form1::backgroundWorker1_RunWorkerCompleted);
		new_thread->ProgressChanged += gcnew System::ComponentModel::ProgressChangedEventHandler(this, &Form1::backgroundWorker1_ProgressChanged);

		ThreadSearchData^ tsd = gcnew ThreadSearchData;
		tsd->charm_template = ct;
		tsd->query = query;
		worker_data.Add( tsd );

		workers.Add( new_thread );
	}

	void EnableSearchControls( const bool enabled )
	{
		btnSearch->Enabled = enabled;
		btnAdvancedSearch->Enabled = enabled;
		btnCancel->Enabled = !enabled;
		mnuMaxResults->Enabled = enabled;
		mnuNumResults->Enabled = enabled;
		cmbSort->Enabled = enabled;
		cmbCharms->Enabled = enabled;
	}

	void StartTasks()
	{
		worker_mutex->WaitOne();

		if( workers.Count > 0 )
		{
			worker_start_index = Math::Max( 1, Math::Min( Environment::ProcessorCount, workers.Count ) );
			//worker_start_index = workers.Count;

			for( unsigned i = 0; i < worker_start_index; ++i )
				workers[ i ]->RunWorkerAsync( worker_data[ i ] );
		}

		worker_mutex->ReleaseMutex();
	}

	System::Void StartSearch()
	{
		progressBar1->Value = 0;
		total_progress = 0;
		num_updates = 0;
		search_cancelled = false;
		if( query->skills.Count > 0 )
		{
			existing_armor.Clear();
			EnableSearchControls( false );
			charm_solution_map.Clear();
			cmbCharms->SelectedIndex = 1;
			txtSolutions->Text = StartString( SolutionsFound ) + L"0";
			final_solutions.Clear();
			all_solutions.Clear();
			no_charm_solutions.Clear();
			workers.Clear();
			worker_data.Clear();
			last_result = nullptr;
			last_search_gunner = tabHunterType->SelectedIndex == 1;
			finished_workers = 0;

			//Check for Auto-Guard first
			if( cmbCharmSelect->SelectedIndex > 1 ) 
			{
				for each( Ability^ ab in query->rel_abilities )
				{
					if( ab->auto_guard )
					{
						for each( Charm^ ch in CharmDatabase::mycharms )
						{
							if( ch->abilities.Count == 1 && ch->abilities[ 0 ]->ability == ab )
							{
								QueueTask( query, ch );
								StartTasks();
								break;
							}
						}
						return;
					}
				}
			}

			switch( cmbCharmSelect->SelectedIndex )
			{
			case 0: // no charms
				{
					QueueTask( query, nullptr );
				}
				break;
			case 1: // my charms only
				{
					List_t< Charm^ > to_search;
					for each( Charm^ ct in CharmDatabase::mycharms )
					{
						Charm::AddToOptimalList( to_search, ct );
					}
					//StartThread( query, nullptr );
					//StartThread( query, to_search[1] );
					for each( Charm^ ct in to_search )
					{
						QueueTask( query, ct );
					}
				}
				break;
			case 2: // slotted charms only
				{
					QueueTask( query, nullptr );
					if( CharmDatabase::have_slots[ cmbCharmTable->SelectedIndex, 1 ] & 0x2 )
						QueueTask( query, gcnew Charm( 1 ) );
					if( ( query->elder_star > 6 || query->hr >= 3 ) && CharmDatabase::have_slots[ cmbCharmTable->SelectedIndex, 2 ] & 0x4 )
						QueueTask( query, gcnew Charm( 2 ) );
					if( ( query->elder_star > 8 || query->hr >= 4 ) && CharmDatabase::have_slots[ cmbCharmTable->SelectedIndex, 3 ] & 0x8 )
						QueueTask( query, gcnew Charm( 3 ) );
				}
				break;
			case 3: // one skill charms
				{
					List_t< Charm^ >^ charms = CharmDatabase::GetCharms( query, false );
					Assert( charms, L"No one-skill charms found" );
					
					for each( Charm^ ct in charms )
						QueueTask( query, ct );
					//StartThread( query, charms[ 2 ] );
					//break;
				}
				break;
			case 4: // two skill charms
				{
					List_t< Charm^ >^ charms = CharmDatabase::GetCharms( query, true );
					Assert( charms, L"No two-skill charms found" );

					for each( Charm^ ct in charms )
						QueueTask( query, ct );
					//StartThread( query, charms[ 0 ] );
				}
				break;
			}
			
			if( workers.Count == 0 )
				QueueTask( query, nullptr );

			StartTasks();
		}
	}

	System::Void btnAdvancedSearch_Click( System::Object^ sender, System::EventArgs^ e )
	{
		//updatedata();
		//return;
		FormulateQuery( true, tabHunterType->SelectedIndex == 1 );
		frmAdvanced advanced_search( query );
		advanced_search.Width = adv_x;
		advanced_search.Height = adv_y;
		if( tabHunterType->SelectedIndex == 0 )
		{
			advanced_search.CheckResult( blast_options );
			advanced_search.ShowDialog( this );
			blast_options = advanced_search.result;
		}
		else if( tabHunterType->SelectedIndex == 1 )
		{
			advanced_search.CheckResult( glast_options );
			advanced_search.ShowDialog( this );
			glast_options = advanced_search.result;
		}
		else return;

		adv_x = advanced_search.Width;
		adv_y = advanced_search.Height;
		SaveConfig();

		if( advanced_search.DialogResult != ::DialogResult::OK ) return;

		for( int p = 0; p < int( Armor::ArmorType::NumArmorTypes ); ++p )
		{
			query->rel_armor[ p ]->Clear();
			for( int i = 0; i < query->inf_armor[ p ]->Count; ++i )
			{
				if( advanced_search.boxes[ p ]->Items[ i ]->Checked )
					query->rel_armor[ p ]->Add( query->inf_armor[ p ][ i ] );
			}
		}
		query->rel_decorations.Clear();
		for( int i = 0; i < query->inf_decorations.Count; ++i )
		{
			if( advanced_search.boxes[ int( Armor::ArmorType::NumArmorTypes ) ]->Items[ i ]->Checked )
				query->rel_decorations.Add( query->inf_decorations[ i ] );
		}

		StartSearch();
	}

	System::Void btnSearch_Click( System::Object^ sender, System::EventArgs^ e )
	{		
		FormulateQuery( false, tabHunterType->SelectedIndex == 1 );
		
		StartSearch();
	}

	System::Void btnCancel_Click( System::Object^ sender, System::EventArgs^ e )
	{
		search_cancelled = true;
		for each( BackgroundWorker^ worker in workers )
			worker->CancelAsync();
		EnableSearchControls( true );
		progressBar1->Value = 0;
	}

	System::Void cmbSkillFilter_SelectedIndexChanged( System::Object^ sender, List_t< ComboBox^ >% skills, List_t< ComboBox^ >% skill_filters, List_t< IndexMap^ >% index_maps )
	{
		List_t< Ability^ > old_skills;
		int index = -1;
		Skill^ selected_skill = nullptr;
		for( unsigned i = 0; i < NumSkills; ++i )
		{
			if( sender == skill_filters[ i ] )
				index = i;
			//Assert( skills[ i ]->SelectedIndex != 0, L"What happen" );
			if( skills[ i ]->SelectedIndex < 1 )
				continue;

			Skill^ skill = Skill::static_skills[ index_maps[ i ][ skills[ i ]->SelectedIndex ] ];
			if( sender == skill_filters[ i ] )
				selected_skill = skill;
			else old_skills.Add( skill->ability );
		}
		if( index == -1 )
			return;

		if( skill_filters[ index ]->SelectedIndex == 2 ) //related
			FindRelatedSkills( skills, index_maps );

		skills[ index ]->BeginUpdate();
		lock_related = true;
		InitSkills( skills[ index ], index_maps[ index ], skill_filters[ index ]->SelectedIndex, %old_skills, %skill_filters == %bSkillFilters );
		ResetSkill( skills[ index ], index_maps[ index ], selected_skill );
		lock_related = false;
		skills[ index ]->EndUpdate();
	}

	System::Void cmbSkillFilter_SelectedIndexChanged( System::Object^ sender, System::EventArgs^ e )
	{
		if( tabHunterType->SelectedIndex == 0 )
		{
			blast_options = nullptr;
			cmbSkillFilter_SelectedIndexChanged( sender, bSkills, bSkillFilters, bIndexMaps );
		}
		else if( tabHunterType->SelectedIndex == 1 )
		{
			glast_options = nullptr;
			cmbSkillFilter_SelectedIndexChanged( sender, gSkills, gSkillFilters, gIndexMaps );
		}
	}

	System::Void cmbSkill_SelectedIndexChanged( System::Object^ sender, List_t< ComboBox^ >% skills, List_t< ComboBox^ >% skill_filters, List_t< IndexMap^ >% index_maps )
	{
		int index = -1;
		for( int i = 0; i < NumSkills; ++i )
			if( sender == skills[ i ] )
			{
				index = i;
				break;
			}
		if( index == -1 )
			return;

		lock_skills = true;

		if( skills[ index ]->SelectedIndex == 0 )
			skills[ index ]->SelectedIndex = -1;

		if( !lock_related )
			FindRelatedSkills( skills, index_maps );
		
		for( int i = 0; i < NumSkills; ++i )
		{
			if( i == index && skill_filters[ i ]->SelectedIndex != 2 )
				continue;

			Skill^ skill = skills[ i ]->SelectedIndex == -1 ? nullptr : Skill::static_skills[ index_maps[ i ][ skills[ i ]->SelectedIndex ] ];
			List_t< Ability^ > old_skills;
			for( int j = 0; j < NumSkills; ++j )
				if( j != i && skills[ j ]->SelectedIndex != -1 )
					old_skills.Add( Skill::static_skills[ index_maps[ j ][ skills[ j ]->SelectedIndex ] ]->ability );

			skills[ i ]->BeginUpdate();
			InitSkills( skills[ i ], index_maps[ i ], skill_filters[ i ]->SelectedIndex, %old_skills, %skill_filters == %bSkillFilters );
			ResetSkill( skills[ i ], index_maps[ i ], skill );
			skills[ i ]->EndUpdate();
		}
		lock_skills = false;
	}

	System::Void cmbSkill_SelectedIndexChanged( System::Object^ sender, System::EventArgs^ e ) 
	{
		if( lock_skills ) return;
		if( tabHunterType->SelectedIndex == 0 )
		{
			blast_options = nullptr;
			cmbSkill_SelectedIndexChanged( sender, bSkills, bSkillFilters, bIndexMaps );
		}
		else if( tabHunterType->SelectedIndex == 1 )
		{
			glast_options = nullptr;
			cmbSkill_SelectedIndexChanged( sender, gSkills, gSkillFilters, gIndexMaps );
		}
	}

	System::Void UpdateResultString( List_t< Solution^ >^ solutions )
	{
		//if( !solutions || solutions->Count == 0 ) return;
		if( !StringTable::text )
			return;
		System::Text::StringBuilder sb( solutions->Count * 1024 );
		int count = 0;

		if( last_result ) sb.Append( last_result );
		
		System::String^ dash = L"-----------------";
		for each( Solution^ solution in solutions )
		{
			if( ++count > MAX_LIMIT )
				break;

			sb.Append( endl );
			for each( Armor^ armor in solution->armors )
			{
				if( armor )
				{
					sb.Append( armor->name );
					if( armor->no_skills )
					{
						if( armor->num_slots == 1 )
							sb.Append( StaticString( OrAnythingWithSingular ) );
						else
							sb.Append( FormatStringN( OrAnythingWithPlural, armor->num_slots ) );
					}
					else if( armor->torso_inc )
						sb.Append( StaticString( OrAnythingWithTorsoInc ) );
					sb.Append( endl );
				}
			}
			if( solution->charm )
			{
				sb.AppendLine( dash );
				sb.AppendLine( solution->charm->GetName() );
			}
			if( solution->decorations.Count > 0 )
			{
				sb.AppendLine( dash );
				Generic::Dictionary< Decoration^, unsigned > deco_dict;
				for each( Decoration^ decoration in solution->decorations )
				{
					if( !deco_dict.ContainsKey( decoration ) )
						deco_dict.Add( decoration, 1 );
					else deco_dict[ decoration ]++;
				}
				Generic::Dictionary< Decoration^, unsigned >::Enumerator iter = deco_dict.GetEnumerator();
				while( iter.MoveNext() )
				{
					sb.Append( Convert::ToString( iter.Current.Value ) )->Append( L"x " );
					if( mnuPrintDecoNames->Checked )
						sb.AppendLine( iter.Current.Key->name );
					else
					{
						sb.Append( iter.Current.Key->abilities[ 0 ]->ability->name )->Append( L" +" );
						sb.Append( iter.Current.Key->abilities[ 0 ]->amount )->Append( L" " )->AppendLine( StaticString( Jewel ) );
					}
				}
			}
			if( solution->total_slots_spare > 0 || cmbSort->SelectedIndex == 10 )
			{
				if( solution->total_slots_spare == 1 )
					sb.AppendLine( StaticString( SlotSpare ) );
				else if( solution->total_slots_spare == 0 )
					sb.AppendLine( FormatString1( SlotsSpare, L"0" ) );
				else if( !solution->slots_spare || ( solution->total_slots_spare <= 3 && solution->slots_spare[ solution->total_slots_spare ] == 1 ) )
				{
					sb.AppendLine( FormatStringN( SlotsSpare, solution->total_slots_spare ) );
				}
				else
				{
					sb.Append( FormatStringN( SlotsSpare, solution->total_slots_spare ) );
					sb.Append( L" (" );
					bool first = true;
					for( unsigned i = 1; i <= 3; ++i )
						for( unsigned j = 0; j < solution->slots_spare[ i ]; ++j )
						{
							sb.Append( ( first ? L"" : L"+" ) + Convert::ToString( i ) );
							first = false;
						}
					sb.AppendLine( L")" );
				}
			}
			if( cmbSort->SelectedIndex > 0 && cmbSort->SelectedIndex < 10 )
			{
				if( cmbSort->SelectedIndex < 8 )
				{
					sb.AppendLine( dash );
					sb.AppendLine( FormatString7( DefEleAbbrev, solution->defence, solution->max_defence, solution->fire_res, solution->water_res, solution->ice_res, solution->thunder_res, solution->dragon_res ) );
				}
				else
				{
					if( cmbSort->SelectedIndex == 8 ) sb.Append( solution->difficulty );
					else if( cmbSort->SelectedIndex == 9 ) sb.Append( solution->rarity );
					sb.Append( L" " )->AppendLine( (String^)cmbSort->SelectedItem );
				}
			}
			if( solution->extra_skills.Count > 0 )
			{
				sb.AppendLine( dash );
				for each( Skill^ skill in solution->extra_skills )
				{
					if( !Utility::Contains( %query->skills, skill ) )
						sb.AppendLine( skill->name );
				}
			}
			if( cmbSort->SelectedIndex == 12 )
			{
				sb.AppendLine( dash );
				AbilityMap::Enumerator e = solution->abilities.GetEnumerator();
				while( e.MoveNext() )
				{
					if( e.Current.Value < 10 &&
						e.Current.Value > 3 )
						sb.Append( L"+" )->Append( e.Current.Value )->Append( L" " )->AppendLine( e.Current.Key->name );
				}
			}
		}

		if( %final_solutions != solutions )
			final_solutions.AddRange( solutions );

		System::Text::StringBuilder sb2;
		sb2.Append( StartString( SolutionsFound ) )->AppendLine( Convert::ToString( final_solutions.Count ) );

		if( solutions->Count > MAX_LIMIT )
		{
			sb2.AppendLine( FormatStringN( ShowingFirstSolutionsOnly, MAX_LIMIT ) );
		}

		sb2.Append( last_result = sb.ToString() );
		
		results_mutex->WaitOne();
		//if( updating_language || language >= 0 && mnuLanguage->DropDownItems[ language ]->ToString() == L"Japanese" )
		{
			//noobery to ensure entire textbox is correct font
			txtSolutions->SuspendLayout();
			txtSolutions->Enabled = false;
			txtSolutions->Text = sb2.ToString();
			txtSolutions->SelectionStart = 0;
			txtSolutions->SelectionLength = txtSolutions->Text->Length;
			txtSolutions->SelectionFont = gcnew Drawing::Font( L"Microsoft Sans Serif", txtSolutions->Font->Size );
			txtSolutions->SelectionLength = 0;
			txtSolutions->Enabled = true;
			txtSolutions->ResumeLayout();
		}
		//else txtSolutions->Text = sb2.ToString();
		
		results_mutex->ReleaseMutex();
	}

#pragma region Worker Thread Stuff

	__int64 HashArmor( List_t< Armor^ >% armors )
	{
		__int64 res = 0;
		//12 bits * 5 armors = 60 bits total for the hash
		for( int i = 0; i < armors.Count; ++i )
		{
			res += armors[ i ] ? ( armors[ i ]->index << ( i * 12 ) ) : 0;
		}
		return res;
	}

	bool SolutionExists( String^ charm, Solution^ sol )
	{
		if( !existing_armor.ContainsKey( charm ) )
		{
			existing_armor.Add( charm, gcnew Generic::Dictionary< __int64, bool > );
			existing_armor[ charm ]->Add( HashArmor( sol->armors ), true );
			return false;
		}
		else
		{
			Generic::Dictionary< __int64, bool >^ set = existing_armor[ charm ];
			const __int64 armor_hash = HashArmor( sol->armors );
			if( set->ContainsKey( armor_hash ) )
				return true;
			else
			{
				set->Add( armor_hash, true );
				return false;
			}
		}
	}

	System::Void AddSolution( String^ charm, Solution^ sol )
	{
		if( SolutionExists( charm, sol ) ) return;
		if( !charm_solution_map.ContainsKey( charm ) )
			charm_solution_map.Add( charm, gcnew List_t< Solution^ > );
		charm_solution_map[ charm ]->Add( sol );
		all_solutions.Add( sol );
	}

	System::Void AddSolutions( List_t< Solution^ >^ solutions )
	{
		charm_map_mutex->WaitOne();
		for each( Solution^ sol in solutions )
		{
			if( sol->charm )
				AddSolution( sol->charm->GetName(), sol );
			else
			{
				all_solutions.Add( sol );
				no_charm_solutions.Add( sol );
			}
		}
		charm_map_mutex->ReleaseMutex();
	}

	System::Void UpdateCharmComboBox()
	{
		UpdateCharmComboBox( -1 );
	}

	System::Void UpdateCharmComboBox( const int new_index )
	{
		Charm^ selected_charm = cmbCharms->SelectedIndex > 1 ? charm_box_charms[ cmbCharms->SelectedIndex - 2 ] : nullptr;
		const int old_index = cmbCharms->SelectedIndex;
		charm_box_charms.Clear();
		List_t< String^ > to_order;
		Map_t< String^, Charm^ > charm_map;
		for each( Solution^ s in all_solutions )
		{
			if( s->charm )
			{
				String^ name = s->charm->GetName();
				if( !charm_map.ContainsKey( name ) )
				{
					to_order.Add( name );
					charm_map.Add( name, s->charm );
				}
			}
		}
		to_order.Sort();
		cmbCharms->BeginUpdate();
		cmbCharms->Items->Clear();
		cmbCharms->Items->Add( BasicString( FilterByNone ) );
		cmbCharms->Items->Add( StaticString( All ) );
		for each( String^% str in to_order )
		{
			charm_box_charms.Add( charm_map[ str ] );
			cmbCharms->Items->Add( str );
		}
		if( new_index == -1 )
		{
			if( old_index == -1 )
				cmbCharms->SelectedIndex = 1;
			else if( old_index < 2 )
				cmbCharms->SelectedIndex = old_index;
			else
			{
				for( int i = 2; i < cmbCharms->Items->Count; ++i )
				{
					if( cmbCharms->Items[ i ]->ToString() == selected_charm->GetName() )
					{
						cmbCharms->SelectedIndex = i;
						break;
					}
				}
			}
		}
		else cmbCharms->SelectedIndex = new_index;
		cmbCharms->EndUpdate();
	}

	System::Void backgroundWorker1_RunWorkerCompleted( Object^ /*sender*/, RunWorkerCompletedEventArgs^ e )
	{
		if( e->Error != nullptr )
		{
			MessageBox::Show( e->Error->Message );
			progressBar1->Value = 0;
			return;
		}
		else if( e->Cancelled )
		{
			progressBar1->Value = 0;
			return;
		}
		else if( e->Result )
		{
			if( !search_cancelled )
			{
				worker_mutex->WaitOne();
				if( worker_start_index < unsigned( workers.Count ) )
				{
					workers[ worker_start_index ]->RunWorkerAsync( worker_data[ worker_start_index ] );
					worker_start_index++;
				}
				worker_mutex->ReleaseMutex();
			}

			AddSolutions( static_cast< List_t< Solution^ >^ >( e->Result ) );
			progress_mutex->WaitOne();
			if( ++finished_workers >= unsigned( workers.Count ) )
			{
				EnableSearchControls( true );
				progressBar1->Value = 100;
				SaveConfig();
				UpdateCharmComboBox( 1 );
			}
			else txtSolutions->Text = StartString( SolutionsFound ) + Convert::ToString( all_solutions.Count );
			progress_mutex->ReleaseMutex();
		}
	}		
	

	System::Void backgroundWorker1_ProgressChanged( Object^ /*sender*/, ProgressChangedEventArgs^ e )
	{
		progress_mutex->WaitOne();
		num_updates++;
		total_progress += e->ProgressPercentage;
		progressBar1->Value = total_progress / workers.Count;
		progress_mutex->ReleaseMutex();
		if( e->UserState )
		{
			AddSolutions( static_cast< List_t< Solution^ >^ >( e->UserState ) );
			txtSolutions->Text = StartString( SolutionsFound ) + Convert::ToString( all_solutions.Count );
		}
	}

	System::Void backgroundWorker1_DoWork( System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e )
	{
		BackgroundWorker^ worker = static_cast< BackgroundWorker^ >( sender );
		ThreadSearchData^ qc = static_cast< ThreadSearchData^ >( e->Argument );
		Query^ query = qc->query;

		List_t< Solution ^ >^ solutions = gcnew List_t< Solution^ >;
		List_t< Armor^ >^ head  = query->rel_armor[ int( Armor::ArmorType::HEAD ) ];
		//List_t< Armor^ >^ head = gcnew List_t< Armor^ >();
		//head->Add( qc->helm );
		List_t< Armor^ >^ body  = query->rel_armor[ int( Armor::ArmorType::BODY ) ];
		List_t< Armor^ >^ arms  = query->rel_armor[ int( Armor::ArmorType::ARMS ) ];
		List_t< Armor^ >^ waist = query->rel_armor[ int( Armor::ArmorType::WAIST ) ];
		List_t< Armor^ >^ legs  = query->rel_armor[ int( Armor::ArmorType::LEGS ) ];
 
		if( head->Count  == 0 ) head->Add( nullptr );
		if( body->Count  == 0 ) body->Add( nullptr );
		if( arms->Count  == 0 ) arms->Add( nullptr );
		if( waist->Count == 0 ) waist->Add( nullptr );
		if( legs->Count  == 0 ) legs->Add( nullptr );
		
		//int solutions_found = 0;
		unsigned last_percent = 0;
		bool new_stuff = false;
		for( int i = 0; i < head->Count; ++i )
		{
			for( int j = 0; j < body->Count; ++j )
			{
				const int progress = ( 100 * ( i * body->Count + j ) ) / ( head->Count * body->Count );
			
				if( !new_stuff )
				{
					worker->ReportProgress( progress - last_percent );
					last_percent = progress;
				}
				else
				{
					List_t< Solution ^ >^ temp = solutions; //handle race-condition: shouldn't modify solution list while iterating through it
					solutions = gcnew List_t< Solution^ >;
					worker->ReportProgress( progress - last_percent, temp );
					last_percent = progress;
					new_stuff = false;
				}
				for( int k = 0; k < arms->Count; ++k )
				{
					for( int l = 0; l < waist->Count; ++l )
					{
						for( int m = 0; m < legs->Count; ++m )
						{
							if( worker->CancellationPending )
							{
								e->Result = solutions;
								return;
							}
							Solution^ job = gcnew Solution();
							job->armors.Add( head[ i ] );
							job->armors.Add( body[ j ] );
							job->armors.Add( arms[ k ] );
							job->armors.Add( waist[ l ] );
							job->armors.Add( legs[ m ] );
							job->charm = qc->charm_template ? gcnew Charm( qc->charm_template ) : nullptr;

							try
							{
								if( job->MatchesQuery( query ) )
								{
									solutions->Add( job );
									new_stuff = true;
									/*if( ++solutions_found >= MAX_LIMIT )
									{
										e->Result = solutions;
										worker->ReportProgress( 100 - last_percent );
										return;
									}*/
								}	
							}
							catch( Exception^ e )						
							{
								MessageBox::Show( e->ToString() );
							}
						}
					}
				}
			}
		}
		worker->ReportProgress( 100 - last_percent );
		e->Result = solutions;
	}
#pragma endregion

	System::Void FindDialogClosed( System::Object^ sender, System::EventArgs^ e )
	{
		find_dialog = nullptr;
	}

	System::Void FindDialogFoundText( System::Object^ sender, System::EventArgs^ e )
	{
		frmFind^ find = (frmFind^)sender;
		if( find->index == -1 )
		{
			txtSolutions->SelectionStart = txtSolutions->Text->Length;
			txtSolutions->SelectionLength = 0;
		}
		else
		{
			txtSolutions->SelectionStart = find->index;
			txtSolutions->SelectionLength = find->length;
		}
		txtSolutions->ScrollToCaret();
		txtSolutions->Focus();
	}

	System::Void KeyDown( System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e )
	{
		if( sender == txtSolutions && e->Control )
		{
			if( e->KeyValue == L'A' )
			{
				txtSolutions->SelectAll();
				e->Handled = true;
				e->SuppressKeyPress = true;
			}
			else if( e->KeyValue == L'F' && !find_dialog )
			{
				find_dialog = gcnew frmFind( txtSolutions );
				find_dialog->DialogClosing += gcnew EventHandler( this, &Form1::FindDialogClosed );
				find_dialog->TextFound += gcnew EventHandler( this, &Form1::FindDialogFoundText );
				find_dialog->Show( this );
				e->Handled = true;
				e->SuppressKeyPress = true;
			}
			else if( e->KeyValue == L'X' )
			{
				
			}
		}
	}

	System::Void cmbSort_SelectedIndexChanged( System::Object^ sender, System::EventArgs^ e )
	{
		static int last_index = -1;
		if( cmbSort->SelectedIndex == last_index ) return;
		last_index = cmbSort->SelectedIndex;
		if( data )
		{
			SaveConfig();
			if( final_solutions.Count > 0 )
			{
				SortResults();
				last_result = nullptr;
				UpdateResultString( %final_solutions );
			}
		}
	}

	System::Void UpdateComboBoxLanguage( ComboBox^ cb )
	{
		cb->BeginUpdate();
		for( int i = 1; i < cb->Items->Count; ++i )
		{
			String^ name = (String^)cb->Items[ i ];
			Skill^ skill = Skill::FindSkill( name );
			Assert( skill, L"No such skill!" );
			cb->Items[ i ] = skill->name;
		}
		cb->EndUpdate();
	}

	System::Void LanguageSelect_Click( System::Object^ sender, System::EventArgs^ e )
	{
		const int old_language = language;
		for( int index = 0; index < mnuLanguage->DropDownItems->Count; ++index )
		{
			ToolStripMenuItem^ item = static_cast< ToolStripMenuItem^ >( mnuLanguage->DropDownItems[ index ] );
			if( mnuLanguage->DropDownItems[ index ] == sender )
			{
				if( index == language )
					return;
				language = index;
				item->Checked = true;
			}
			else item->Checked = false;
		}
		if( old_language == language )
			return;

		updating_language = true;

		StringTable::LoadLanguage( static_cast< ToolStripMenuItem^ >( sender )->ToString() );

		for each( ComboBox^ box in bSkillFilters )
		{
			box->BeginUpdate();
			for( int i = 0, j = 0; i < SkillTag::tags.Count; ++i )
				if( !SkillTag::tags[i]->disable_b )
					box->Items[ j++ ] = SkillTag::tags[ i ]->name;
			box->EndUpdate();
		}
		for each( ComboBox^ box in gSkillFilters )
		{
			box->BeginUpdate();
			for( int i = 0, j = 0; i < SkillTag::tags.Count; ++i )
				if( !SkillTag::tags[i]->disable_g )
					box->Items[ j++ ] = SkillTag::tags[ i ]->name;
			box->EndUpdate();
		}

#define UpdateMenu( X ) mnu##X->Text = StaticString( X )
		UpdateMenu( File );
		UpdateMenu( Options );
		UpdateMenu( Language );
		UpdateMenu( Help );
		UpdateMenu( LoadData );
		UpdateMenu( SaveData );
		UpdateMenu( Exit );
		UpdateMenu( ClearSettings );
		UpdateMenu( AllowBadSkills );
		UpdateMenu( AllowEventArmor );
		UpdateMenu( AllowLowerTierArmor );
		UpdateMenu( AllowJapaneseOnlyDLC );
		UpdateMenu( AllowPiercings );
		UpdateMenu( CheckForUpdates );	
		UpdateMenu( About );
		UpdateMenu( MaxResults );
		UpdateMenu( PrintDecoNames );
		UpdateMenu( SortSkillsAlphabetically );
#undef UpdateMenu
		lblHR->Text = StaticString( HR );
		lblElder->Text = StaticString( VillageQuests );
		lblSlots->Text = StaticString( MaxWeaponSlots );
		lblCharmTable->Text = StaticString( CharmTable );
		lblWhatIsCharmTable->Location = Drawing::Point( lblCharmTable->Location.X + lblCharmTable->Size.Width - 5, lblWhatIsCharmTable->Location.Y );
		grpSort->Text = StaticString( SortResultsBy );
		grpCharmFilter->Text = StaticString( FilterResultsByCharm );
		grpCharms->Text = StaticString( Charms );
		grpResults->Text = StaticString( Results );
		rdoMale->Text = StaticString( Male );
		rdoFemale->Text = StaticString( Female );
		tabBlademaster->Text = StaticString( Blademaster );
		tabGunner->Text = StaticString( Gunner );
		grpBSkills->Text = StaticString( Skills );
		grpGSkills->Text = StaticString( Skills );
		grpBSkillFilters->Text = StaticString( SkillFilters );
		grpGSkillFilters->Text = StaticString( SkillFilters );
		btnCharms->Text = StaticString( MyCharms );
		btnImport->Text = StaticString( Import );
		btnSearch->Text = StaticString( QuickSearch );
		btnAdvancedSearch->Text = StaticString( AdvancedSearch );
		btnCancel->Text = StaticString( Cancel );
		cmbCharmSelect->Items[ 0 ] = StaticString( UseNoCharms );
		cmbCharmSelect->Items[ 1 ] = StaticString( UseOnlyMyCharms );
		cmbCharmSelect->Items[ 2 ] = StaticString( UseOnlySlottedCharms );
		cmbCharmSelect->Items[ 3 ] = StaticString( UseUpToOneSkillCharms );
		cmbCharmSelect->Items[ 4 ] = StaticString( UseOnlyTwoSkillCharms );
		cmbSort->Items[ 0 ] = BasicString( SortByNone );
		cmbSort->Items[ 1 ] = StaticString( DragonRes );
		cmbSort->Items[ 2 ] = StaticString( FireRes );
		cmbSort->Items[ 3 ] = StaticString( IceRes );
		cmbSort->Items[ 4 ] = StaticString( ThunderRes );
		cmbSort->Items[ 5 ] = StaticString( WaterRes );
		cmbSort->Items[ 6 ] = StaticString( BaseDefence );
		cmbSort->Items[ 7 ] = StaticString( MaxDefence );
		cmbSort->Items[ 8 ] = StaticString( Difficulty );
		cmbSort->Items[ 9 ] = StaticString( Rarity );
		cmbSort->Items[ 10 ] = StaticString( SortSlotsSpare );
		cmbSort->Items[ 11 ] = StaticString( SortFamily );
		cmbSort->Items[ 12 ] = StaticString( SortExtraSkills );
		cmbCharmTable->Items[ 0 ] = StaticString( UnknownTable );
		cmbCharmTable->Items[ 18 ] = StaticString( AnyTable );

		charm_solution_map.Clear();
		for each( Solution^ s in all_solutions )
		{
			if( s->charm )
			{
				String^ str = s->charm->GetName();
				if( !charm_solution_map.ContainsKey( str ) )
					charm_solution_map.Add( str, gcnew List_t< Solution^ > );
				charm_solution_map[ str ]->Add( s );
			}
		}
		
		for( int i = 0; i < NumSkills; ++i )
		{
			bSkills[ i ]->BeginUpdate();
			cmbSkillFilter_SelectedIndexChanged( bSkillFilters[ i ], bSkills, bSkillFilters, bIndexMaps );	
			bSkills[ i ]->EndUpdate();
			gSkills[ i ]->BeginUpdate();
			cmbSkillFilter_SelectedIndexChanged( gSkillFilters[ i ], gSkills, gSkillFilters, gIndexMaps );
			gSkills[ i ]->EndUpdate();
		}
		
		UpdateCharmComboBox();

		if( construction_complete )
			CharmDatabase::SaveCustom();
		
		updating_language = false;
		cmbCharms_SelectedIndexChanged( nullptr, nullptr );
	}

	System::Void aboutToolStripMenuItem_Click( System::Object^ sender, System::EventArgs^ e )
	{
		frmAbout about_form;
		about_form.ShowDialog( this );
	}

	System::Void HRChanged( System::Object^ sender, System::EventArgs^ e )
	{
		DeleteOptions( sender, e );
	}

	System::Void DeleteOptions( System::Object^ sender, System::EventArgs^ e )
	{
		blast_options = glast_options = nullptr;
	}

	System::Void SortResults()
	{
		if( cmbSort->SelectedIndex < 1 || sort_off ) return;
		else if( cmbSort->SelectedIndex == 1 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByDragonRes ) );
		else if( cmbSort->SelectedIndex == 2 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByFireRes ) );
		else if( cmbSort->SelectedIndex == 3 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByIceRes ) );
		else if( cmbSort->SelectedIndex == 4 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByThunderRes ) );
		else if( cmbSort->SelectedIndex == 5 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByWaterRes ) );
		else if( cmbSort->SelectedIndex == 6 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByDefence ) );
		else if( cmbSort->SelectedIndex == 7 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByMaxDefence ) );
		else if( cmbSort->SelectedIndex == 8 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByDifficulty ) );
		else if( cmbSort->SelectedIndex == 9 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByRarity ) );
		else if( cmbSort->SelectedIndex == 10 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionBySlotsSpare ) );
		else if( cmbSort->SelectedIndex == 11 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByFamily ) );
		else if( cmbSort->SelectedIndex == 12 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionsByExtraSkills ) );
	}

	bool EndsWithSlots( String^% s )
	{
		return s->EndsWith( L" ---" ) ||
			   s->EndsWith( L"O--" ) ||
			   s->EndsWith( L"OO-" ) ||
			   s->EndsWith( L"OOO" );
	}

	System::Void contextMenuStrip1_Opening( System::Object^ sender, System::ComponentModel::CancelEventArgs^ e )
	{
		if( txtSolutions->Text == L"" ) return;
		cmsSolutions->Items->Clear();
		e->Cancel = true;

		Point pos = txtSolutions->PointToClient( txtSolutions->MousePosition );
		int chr = txtSolutions->GetCharIndexFromPosition( pos );
		int line = txtSolutions->GetLineFromCharIndex( chr );
		String^ str = txtSolutions->Lines[ line ];
		if( str == L"" ) return;
		
		if( str->Length >= 4 )
		{
			if( str->Substring( 1, 2 ) == L"x " || str->Substring( 2, 2 ) == L"x " )
			{
				String^ deco_name = str->Substring( str->IndexOf( L' ' ) + 1 );
				Decoration^ decoration = Decoration::FindDecoration( deco_name );
				if( decoration )
				{
					Utility::UpdateContextMenu( cmsSolutions, decoration );
					e->Cancel = false;
				}
				else if( deco_name->EndsWith( L" " + StaticString( Jewel ) ) )
				{
					//1x Expert +5 Jewel
					const int plus = deco_name->IndexOf( L'+' );
					const unsigned points = Convert::ToUInt32( deco_name->Substring( plus + 1, 1 ) );
					String^ ability_name = deco_name->Substring( 0, plus - 1 );
					Decoration^ decoration = Decoration::FindDecoration( ability_name, points );
					if( decoration )
					{
						Utility::UpdateContextMenu( cmsSolutions, decoration );
						e->Cancel = false;
					}
				}
				return;
			}
		}
		Armor^ armor = Armor::FindArmor( str );
		if( armor )
		{
			Utility::UpdateContextMenu( cmsSolutions, armor );
			e->Cancel = false;
			return;
		}

		int bracket = str->LastIndexOf( L'(' );
		if( bracket != -1 )
		{
			str = str->Substring( 0, bracket - 1 );
			armor = Armor::FindArmor( str );
			if( armor )
			{
				Utility::UpdateContextMenu( cmsSolutions, armor );
				e->Cancel = false;
			}
		}
		if( EndsWithSlots( str ) )
		{
			Utility::UpdateContextMenu( cmsSolutions, str, cmbCharmTable->SelectedIndex );
			e->Cancel = false;
		}
	}

	System::Void cmbCharms_SelectedIndexChanged( System::Object^ sender, System::EventArgs^ e )
	{
		if( updating_language || cmbCharms->SelectedIndex < 0 )
			return;
		last_result = L"";
		final_solutions.Clear();
		String^ charm = (String^)cmbCharms->SelectedItem;
		if( cmbCharms->SelectedIndex == 0 ) // None
		{
			final_solutions.AddRange( %no_charm_solutions );
			SortResults();
			UpdateResultString( %final_solutions );
		}
		else if( cmbCharms->SelectedIndex == 1 ) // All
		{
			final_solutions.AddRange( %all_solutions );
			SortResults();
			UpdateResultString( %final_solutions );
		}
		else if( charm_solution_map.ContainsKey( charm ) )
		{
			final_solutions.AddRange( charm_solution_map[ charm ] );
			SortResults();
			UpdateResultString( %final_solutions );
		}
		else
			UpdateResultString( gcnew List_t< Solution^ > );
	}

	System::Void btnCharms_Click( System::Object^ sender, System::EventArgs^ e )
	{
		ManageCharms mycharms( language, data, mnuSortSkillsAlphabetically->Checked );
		mycharms.ShowDialog( this );

		if( mycharms.detected_charm_table != -1 )
		{
			cmbCharmTable->SelectedIndex = mycharms.detected_charm_table;
			OptionsChanged( nullptr, nullptr );
		}
	}
	
	System::Void UpdateMenuItem_Click( System::Object^ sender, System::EventArgs^ e )
	{
		System::Diagnostics::Process::Start( "http://forums.minegarde.com/topic/5304-athenas-armor-set-search-for-mh3g" );
	}

	System::Void toolStripMenuItem1_Click( System::Object^ sender, System::EventArgs^ e )
	{
		cmbSort->SelectedIndex = 0;
		for each( ComboBox^ cb in bSkills )
		{
			cb->SelectedIndex = -1;
		}
		for each( ComboBox^ cb in gSkills )
		{
			cb->SelectedIndex = -1;
		}
		for each( ComboBox^ cb in bSkillFilters )
		{
			cb->SelectedIndex = 0;
		}
		for each( ComboBox^ cb in gSkillFilters )
		{
			cb->SelectedIndex = 0;
		}
	}
	
	System::Void mnuLoad_Click( System::Object^ sender, System::EventArgs^ e )
	{
		OpenFileDialog dlg;
		dlg.InitialDirectory = System::Environment::CurrentDirectory;
		dlg.DefaultExt = L".ass";
		dlg.AddExtension = true;
		dlg.Filter = StartString( ASSSettings ) + L"(*.ass)|*.ass";
		dlg.FilterIndex = 0;
		::DialogResult res = dlg.ShowDialog();
		if( res == ::DialogResult::OK )
			LoadConfig( dlg.FileName );
	}
	
	System::Void mnuSave_Click( System::Object^ sender, System::EventArgs^ e )
	{
		SaveFileDialog dlg;
		dlg.InitialDirectory = System::Environment::CurrentDirectory;
		dlg.DefaultExt = L".ass";
		dlg.AddExtension = true;
		dlg.Filter = StartString( ASSSettings ) + L"(*.ass)|*.ass";
		dlg.FilterIndex = 0;
		dlg.FileName = L"results.ass";
		::DialogResult res = dlg.ShowDialog();
		if( res == ::DialogResult::OK )
			SaveConfig( dlg.FileName );
	}

	System::Void btnImport_Click( System::Object^ sender, System::EventArgs^ e )
	{
		OpenFileDialog dlg;
		dlg.InitialDirectory = System::Environment::CurrentDirectory;
		dlg.DefaultExt = L".bin";
		dlg.AddExtension = true;
		dlg.Filter = StartString( MasaxMHCharmList ) + L"(*.csv)|CHARM.csv";
		dlg.FilterIndex = 0;
		DialogResult_t res = dlg.ShowDialog();
		//dlg.FileName = L"D:\\Downloads\\GANSIMU_MH3G_0.9.1\\save\\CHARM.csv";
		//DialogResult_t res = DialogResult_t::OK;
		if( res == DialogResult_t::OK )
		{
			ImportCharms ic;
			ic.language = language;
			ic.LoadCharms( dlg.FileName );
			res = ic.ShowDialog();
		}
	}

	System::Void cmbCharmTable_SelectedIndexChanged( System::Object^ sender, System::EventArgs^ e )
	{
		lblWhatIsCharmTable->Visible = cmbCharmTable->SelectedIndex == 0 || cmbCharmTable->SelectedIndex == 18;
		OptionsChanged( sender, e );
	}

	System::Void lblWhatIsCharmTable_Click( System::Object^ sender, System::EventArgs^ e )
	{
		const int table = CharmDatabase::DetectCharmTable();
		if( table != -1 )
		{
			cmbCharmTable->SelectedIndex = table;
			OptionsChanged( sender, e );
		}
	}

	System::Void exitToolStripMenuItem_Click( System::Object^ sender, System::EventArgs^ e )
	{
		this->Close();
	}
};
}
