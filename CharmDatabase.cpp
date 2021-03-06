#include "stdafx.h"
#include "CharmDatabase.h"
#include "Solution.h"
#include "Armor.h"
#include "Skill.h"
#include <fstream>
#include <cmath>

using namespace System;

static const unsigned TotalTables = 19;
static const unsigned AllTable = TotalTables - 1;

template< typename T >
void MaxOut( T% curr, const int num )
{
	curr = Math::Max( curr, (T)num );
}

template< typename T >
void MinOut( T% curr, const int num )
{
	curr = Math::Min( curr, (T)num );
}

unsigned CalcMaxCharmType( Query^ query )
{
	if( query->hr >= 4 || query->elder_star >= 8 )
	{
		return ( query->hr >= 6 ) ? 3 : 2;
	}
	else
	{
		return ( query->hr >= 3 || query->elder_star >= 6 ) ? 1 : 0;
	}
}

void GetCharms( List_t< Charm^ >^ list, const unsigned charm_table, List_t< Skill^ >% skills, const unsigned max_charm_type )
{
	const unsigned max_slots = Math::Min( 3u, max_charm_type + 1 );
	for each( Skill^ skill in skills )
	{
		int mx[ 4 ] = { 0, 0, 0, 0 };
		for( unsigned slots = max_slots+1; slots --> 0; )
		{
			mx[ slots ] = 0;
			for( unsigned typ = 0; typ <= max_charm_type; ++typ )
			{
				array< signed char, 2 >^ arr = CharmDatabase::min_max[ charm_table, typ ]->max_single;
				MaxOut( mx[ slots ], arr[ skill->ability->static_index, slots ] );
			}
			if( mx[ slots ] <= 0 ||
				max_slots > 0 && slots == 0 && mx[ slots ] <= mx[ 1 ] )
				continue;

			Charm^ ct = gcnew Charm;
			ct->num_slots = slots;
			ct->abilities.Add( gcnew AbilityPair( skill->ability, mx[ slots ] ) );
			
			if( charm_table > 0 && charm_table <= 17 )
			{
				String^ charm_string = ct->GetName();
				CharmLocationDatum^ loc = nullptr;
				if( CharmDatabase::location_cache->ContainsKey( charm_string ) )
					loc = CharmDatabase::location_cache[ charm_string ];
				else
				{
					loc = Utility::FindCharm( skill->ability, mx[ slots ], nullptr, 0, slots );
					CharmDatabase::location_cache->Add( charm_string, loc );
				}
				if( loc->examples[ charm_table - 1 ] != nullptr )
				{
					Charm^ example_charm = gcnew Charm( loc->examples[ charm_table - 1 ] );
					if( example_charm->abilities.Count > 0 && example_charm->abilities[ 0 ]->ability->relevant )
					{
						for( unsigned i = mx[ slots ]; i --> 1; )
						{
							ct->abilities[0]->amount = i;
							String^ e = ct->GetName();
							if( CharmDatabase::location_cache->ContainsKey( e ) )
								loc = CharmDatabase::location_cache[ e ];
							else
							{
								loc = Utility::FindCharm( skill->ability, i, nullptr, 0, slots );
								CharmDatabase::location_cache->Add( e, loc );
							}
							if( loc->examples[ charm_table - 1 ] == nullptr )
							{
								example_charm->remove_ability2_when_ability1_reduced_to = i;
								break;
							}
						}
						ct = example_charm;
					}
				}
			}

			list->Add( ct );
		}
	}
}

void AddSlotOnlyCharms( List_t< Charm^ >^ res, Query^ query, const unsigned max_slots, const unsigned max_charm_type, const bool (&have)[ 4 ] )
{
	for( unsigned slots = 3; slots > 0; --slots )
	{
		if( !have[ slots ] && CharmDatabase::have_slots[ query->charm_table, max_charm_type ] & ( 1 << slots ) )
		{
			res->Add( gcnew Charm( slots ) );
			break;
		}
	}
	return;
}

List_t< Charm^ >^ CharmDatabase::GetCharms( Query^ query, const bool use_two_skill_charms )
{
	List_t< Charm^ >^ res = gcnew List_t< Charm^ >;
	bool have[ 4 ] = { false, false, false, false };
	const unsigned max_charm_type = CalcMaxCharmType( query );

	//one skill charms
	::GetCharms( res, query->charm_table, query->skills, max_charm_type );

	if( use_two_skill_charms && query->charm_table > 0 )
	{
		//two skill charms
		TableSlotDatum^ table_data = min_max[ query->charm_table, max_charm_type ];
		for( int i = 1; i < query->skills.Count; ++i )
		{
			for( int j = 0; j < i; ++j )
			{
				for( unsigned slots = 0; slots < 4; slots++ )
				{
					List_t< Charm^ >^ to_add = table_data->two_skill_data[ slots ][ query->skills[ i ]->ability->static_index, query->skills[ j ]->ability->static_index ];
					if( to_add )
						res->AddRange( to_add );
				}
			}
		}
	}

	//no skill charms
	for each( Charm^ charm in res )
	{
		have[ charm->num_slots ] = true;
	}
	
	if( query->hr >= 4 || query->elder_star >= 8 )
	{
		AddSlotOnlyCharms( res, query, 3, max_charm_type, have );
	}
	else if( query->hr >= 3 || query->elder_star >= 6 )
	{
		AddSlotOnlyCharms( res, query, 2, max_charm_type, have );
	}
	else
	{
		AddSlotOnlyCharms( res, query, 1, max_charm_type, have );
	}
	return res;
}

#define CUSTOM_TXT L"Data/mycharms.txt"

void CharmDatabase::SaveCustom()
{
	//slots,skill1,num,skill2,num
	IO::StreamWriter fout( CUSTOM_TXT );
	fout.WriteLine( L"#Format: NumSlots,Skill1,Points1,Skill2,Points2" );
	for each( Charm^ ch in mycharms )
	{
		fout.Write( Convert::ToString( ch->num_slots ) );
		for( int i = 0; i < 2; ++i )
		{
			if( i < ch->abilities.Count )
				fout.Write( L"," + ch->abilities[ i ]->ability->name + L"," + Convert::ToString( ch->abilities[ i ]->amount ) );
			else fout.Write( L",," );
		}
		fout.WriteLine();
	}
}

void CharmDatabase::LoadCustom()
{
	mycharms.Clear();
	if( !IO::File::Exists( CUSTOM_TXT ) )
	{
		Ability^ autoguard = nullptr;
		for each( Ability^ ab in Ability::static_abilities )
		{
			if( ab->auto_guard )
			{
				autoguard = ab;
				break;
			}
		}
		if( !autoguard )
			return;

		Charm^ agcharm = gcnew Charm();
		agcharm->custom = true;
		agcharm->hacked = false;
		agcharm->num_slots = 0;
		agcharm->abilities.Add( gcnew AbilityPair( autoguard, 10 ) );

		mycharms.Add( agcharm );
		return;
	}

	IO::StreamReader fin( CUSTOM_TXT );
	String^ temp;
	System::Text::StringBuilder sb;
	bool cheats = false;
	while( !fin.EndOfStream )
	{
		temp = fin.ReadLine();
		if( temp == L"" || temp[ 0 ] == L'#' ) continue;
		List_t< String^ > split;
		Utility::SplitString( %split, temp, L',' );
		if( split.Count != 5 )
		{
			Windows::Forms::MessageBox::Show( nullptr, L"Incorrect number of commas", temp );
			continue;
		}
		//slots,skill1,num,skill2,num
		Charm^ charm = gcnew Charm();
		
		try
		{
			charm->num_slots = Convert::ToUInt32( split[ 0 ] );

			if( split[ 1 ] != L"" )
			{
				if( split[ 1 ] == L"ElementAtk" )
					split[ 1 ] = L"Elemental";
				else if( split[ 1 ] == L"EdgeMaster" )
					split[ 1 ] = L"Edgemaster";
				Ability^ ab = Ability::FindAbility( split[ 1 ] );
				if( !ab )
				{
					Windows::Forms::MessageBox::Show( nullptr, L"\"" + split[ 1 ] + L"\": No such skill", temp );
					continue;
				}
				charm->abilities.Add( gcnew AbilityPair( ab, Convert::ToInt32( split[ 2 ] ) ) );
			}
			if( split[ 3 ] != L"" )
			{
				Ability^ ab = Ability::FindAbility( split[ 3 ] );
				if( !ab )
				{
					Windows::Forms::MessageBox::Show( nullptr, L"\"" + split[ 3 ] + L"\": No such skill", temp );
					continue;
				}
				charm->abilities.Add( gcnew AbilityPair( ab, Convert::ToInt32( split[ 4 ] ) ) );
			}
		}
		catch( Exception^ e )
		{
			Windows::Forms::MessageBox::Show( nullptr, L"Could not read skill points:\n" + e->ToString(), temp );
			continue;
		}
		
		if( !CharmExists( charm ) &&
			!CharmIsLegal( charm ) )
		{
			cheats = true;
			sb.AppendLine( charm->GetName() );
		}
		else
		{
			charm->custom = true;
			mycharms.Add( charm );
		}
	}
	if( cheats )
		System::Windows::Forms::MessageBox::Show( StaticString( Cheater ) + L"\n\n" + sb.ToString() );
}

#pragma region Charm Generation
ref struct StaticData
{
#pragma region Skill1Table
	static array< array< unsigned char >^ >^ skill1_table =
	{
		//mystery
		{
			3, 1, 8, // 気絶
			1, 1, 5, // 麻痺
			2, 1, 5, // 睡眠
			0, 1, 5, // 毒
			6, 1, 7, // 対防御DOWN
			100, 1, 10, // 盗み無効
			4, 1, 7, // 耐泥耐雪
			5, 1, 7, // 抗菌
			24, 1, 7, // 耐震
			25, 1, 10, // 耐暑
			26, 1, 10, // 耐寒
			9, 1, 4, // 攻撃
			16, 1, 4, // 達人
			78, 1, 6, // 爆弾強化
			76, 1, 6, // 笛
			77, 1, 6, // 砲術
			10, 1, 4, // 防御
			43, 1, 6, // 火耐性
			44, 1, 6, // 水耐性
			45, 1, 6, // 雷耐性
			46, 1, 6, // 氷耐性
			47, 1, 6, // 龍耐性
			11, 1, 8, // 体力
			101, 1, 8, // 高速設置
			82, 1, 8, // 腹減り
			83, 1, 10, // 食いしん坊
			98, 1, 8, // 運搬
			88, 1, 10, // 採取
			89, 1, 8, // 高速収集
			90, 1, 10, // 気まぐれ
			80, 1, 8, // 広域
			97, 1, 8, // 千里眼
			86, 1, 10, // 調合成功率
			87, 1, 8, // 調合数
			81, 1, 8, // 効果持続
			96, 1, 8, // 観察眼
			7, 1, 8, // こやし
			99, 1, 8, // 狩人
			32, 1, 8, // 気配
			62, 1, 6, // 通常弾追加
			63, 1, 6, // 貫通弾追加
			64, 1, 6, // 散弾追加
			65, 1, 6, // 榴弾追加
			66, 1, 6, // 拡散弾追加
			67, 1, 6, // 斬裂弾追加
			58, 1, 6, // 精密射撃
			70, 1, 7, // 麻痺瓶追加
			71, 1, 8, // 睡眠瓶追加
			69, 1, 8, // 毒瓶追加
			72, 1, 7, // 強撃瓶追加
			73, 1, 8, // 接撃瓶追加
			74, 1, 8, // 減気瓶追加
			102, 1, 8, // 燃鱗
			104, 1, 6, // 水流
			105, 1, 6, // 泳ぎ
			103, 1, 7, // 酸素
			115, 1, 3, // 采配
			116, 1, 3, // 号令
			75, 1, 6, // 爆破瓶追加
			68, 1, 6, // 爆破弾追加
		},

		//shining
		{
			3, 1, 8, // 気絶
			1, 1, 5, // 麻痺
			2, 1, 5, // 睡眠
			0, 1, 5, // 毒
			6, 1, 7, // 対防御DOWN
			4, 1, 7, // 耐泥耐雪
			5, 1, 7, // 抗菌
			24, 1, 7, // 耐震
			23, 1, 4, // 風圧
			9, 1, 4, // 攻撃
			16, 1, 4, // 達人
			36, 1, 4, // 特殊攻撃
			37, 1, 7, // 火属性攻撃
			38, 1, 7, // 水属性攻撃
			39, 1, 7, // 雷属性攻撃
			40, 1, 7, // 氷属性攻撃
			41, 1, 7, // 龍属性攻撃
			19, 1, 6, // KO
			20, 1, 6, // 減気攻撃
			107, 1, 6, // 底力
			78, 1, 6, // 爆弾強化
			76, 1, 6, // 笛
			77, 1, 6, // 砲術
			10, 1, 4, // 防御
			43, 1, 6, // 火耐性
			44, 1, 6, // 水耐性
			45, 1, 6, // 雷耐性
			46, 1, 6, // 氷耐性
			47, 1, 6, // 龍耐性
			13, 1, 7, // 加護
			11, 1, 8, // 体力
			12, 1, 7, // 回復速度
			79, 1, 6, // 回復量
			29, 1, 6, // 回避性能
			14, 1, 6, // 納刀
			101, 1, 8, // 高速設置
			84, 1, 4, // 食事
			21, 1, 6, // 体術
			28, 1, 6, // 気力回復
			82, 1, 8, // 腹減り
			98, 1, 8, // 運搬
			80, 1, 8, // 広域
			97, 1, 8, // 千里眼
			33, 1, 6, // ガード性能
			34, 1, 4, // ガード強化
			51, 1, 4, // 研ぎ師
			62, 1, 6, // 通常弾追加
			63, 1, 6, // 貫通弾追加
			64, 1, 6, // 散弾追加
			65, 1, 6, // 榴弾追加
			66, 1, 6, // 拡散弾追加
			67, 1, 6, // 斬裂弾追加
			54, 1, 4, // 装填速度
			58, 1, 6, // 精密射撃
			102, 1, 8, // 燃鱗
			104, 1, 6, // 水流
			105, 1, 6, // 泳ぎ
			103, 1, 7, // 酸素
			115, 1, 5, // 采配
			116, 1, 5, // 号令
			75, 1, 6, // 爆破瓶追加
			68, 1, 6, // 爆破弾追加
		},

		//ancient
		{
			22, 1, 5, // 聴覚保護
			23, 1, 5, // 風圧
			36, 1, 6, // 特殊攻撃
			37, 1, 7, // 火属性攻撃
			38, 1, 7, // 水属性攻撃
			39, 1, 7, // 雷属性攻撃
			40, 1, 7, // 氷属性攻撃
			41, 1, 7, // 龍属性攻撃
			17, 1, 5, // 痛撃
			18, 1, 5, // 重撃
			15, 1, 5, // 溜め短縮
			109, 1, 6, // 本気
			19, 1, 6, // KO
			20, 1, 6, // 減気攻撃
			107, 1, 6, // 底力
			113, 1, 5, // 属性耐性
			13, 1, 7, // 加護
			79, 1, 6, // 回復量
			29, 1, 6, // 回避性能
			30, 1, 6, // 回避距離
			14, 1, 6, // 納刀
			84, 1, 5, // 食事
			27, 1, 5, // スタミナ
			21, 1, 6, // 体術
			28, 1, 6, // 気力回復
			48, 1, 5, // 斬れ味
			33, 1, 6, // ガード性能
			34, 1, 6, // ガード強化
			59, 1, 5, // 通常弾強化
			60, 1, 5, // 貫通弾強化
			61, 1, 5, // 散弾強化
			54, 1, 6, // 装填速度
			57, 1, 5, // 反動
			115, 1, 7, // 采配
			116, 1, 7, // 号令
		},

		//weathered
		{
			22, 3, 5, // 聴覚保護
			23, 3, 5, // 風圧
			36, 3, 6, // 特殊攻撃
			42, 1, 5, // 属性攻撃
			37, 3, 7, // 火属性攻撃
			38, 3, 7, // 水属性攻撃
			39, 3, 7, // 雷属性攻撃
			40, 3, 7, // 氷属性攻撃
			41, 3, 7, // 龍属性攻撃
			17, 3, 5, // 痛撃
			18, 3, 5, // 重撃
			55, 1, 5, // 装填数
			15, 3, 5, // 溜め短縮
			109, 3, 6, // 本気
			19, 3, 6, // KO
			20, 3, 6, // 減気攻撃
			107, 3, 6, // 底力
			113, 3, 5, // 属性耐性
			13, 3, 7, // 加護
			79, 3, 6, // 回復量
			29, 3, 6, // 回避性能
			30, 3, 6, // 回避距離
			14, 3, 6, // 納刀
			84, 3, 5, // 食事
			27, 3, 5, // スタミナ
			21, 3, 6, // 体術
			28, 3, 6, // 気力回復
			93, 1, 5, // 運気
			95, 1, 5, // 捕獲
			94, 1, 5, // 剥ぎ取り
			49, 1, 5, // 匠
			50, 1, 5, // 剣術
			48, 3, 5, // 斬れ味
			53, 1, 5, // 抜刀会心
			52, 1, 5, // 抜刀減気
			33, 3, 6, // ガード性能
			34, 3, 6, // ガード強化
			59, 3, 5, // 通常弾強化
			60, 3, 5, // 貫通弾強化
			61, 3, 5, // 散弾強化
			56, 1, 5, // 速射
			54, 3, 6, // 装填速度
			57, 3, 5, // 反動
			112, 1, 3, // 属性解放
			108, 1, 5, // 逆境
			85, 1, 3, // 肉食
			31, 1, 3, // 飛距離
			91, 1, 3, // ハチミツ
			8, 1, 3, // 細菌学
			110, 1, 3, // 闘魂
			111, 1, 3, // 無傷
			114, 1, 5, // 根性
			115, 3, 7, // 采配
			116, 3, 7, // 号令
			92, 3, 7, // 護石王
		},
	};
#pragma endregion
#pragma region Skill2Table
	static array< array< signed char >^ >^ skill2_table =
	{
		//mystery
		{},

		//shining
		{
			3, -10, 10, // 気絶
			1, -10, 7, // 麻痺
			2, -10, 7, // 睡眠
			0, -10, 7, // 毒
			6, -10, 8, // 対防御DOWN
			100, -10, 10, // 盗み無効
			4, -10, 8, // 耐泥耐雪
			5, -10, 8, // 抗菌
			24, -10, 8, // 耐震
			25, -10, 10, // 耐暑
			26, -10, 10, // 耐寒
			9, -7, 7, // 攻撃
			16, -7, 7, // 達人
			36, -4, 4, // 特殊攻撃
			78, -8, 8, // 爆弾強化
			76, -8, 8, // 笛
			77, -8, 8, // 砲術
			10, -7, 7, // 防御
			43, -10, 10, // 火耐性
			44, -10, 10, // 水耐性
			45, -10, 10, // 雷耐性
			46, -10, 10, // 氷耐性
			47, -10, 10, // 龍耐性
			11, -10, 10, // 体力
			12, -4, 4, // 回復速度
			101, -10, 10, // 高速設置
			82, -10, 10, // 腹減り
			83, -10, 13, // 食いしん坊
			98, -10, 10, // 運搬
			88, -10, 13, // 採取
			89, -10, 10, // 高速収集
			90, -10, 13, // 気まぐれ
			80, -10, 10, // 広域
			97, -10, 10, // 千里眼
			86, -10, 13, // 調合成功率
			87, -10, 10, // 調合数
			81, -10, 10, // 効果持続
			96, -10, 10, // 観察眼
			7, -10, 10, // こやし
			99, -10, 10, // 狩人
			32, -10, 10, // 気配
			62, -8, 8, // 通常弾追加
			63, -10, 10, // 貫通弾追加
			64, -10, 10, // 散弾追加
			65, -10, 10, // 榴弾追加
			66, -10, 10, // 拡散弾追加
			67, -10, 10, // 斬裂弾追加
			58, -10, 10, // 精密射撃
			70, -10, 10, // 麻痺瓶追加
			71, -10, 10, // 睡眠瓶追加
			69, -10, 10, // 毒瓶追加
			72, -10, 10, // 強撃瓶追加
			73, -10, 10, // 接撃瓶追加
			74, -10, 10, // 減気瓶追加
			102, -10, 10, // 燃鱗
			104, -10, 10, // 水流
			105, -10, 10, // 泳ぎ
			103, -10, 10, // 酸素
			115, -7, 7, // 采配
			116, -7, 7, // 号令
			75, -10, 10, // 爆破瓶追加
			68, -10, 10, // 爆破弾追加
		},

		//ancient
		{
			3, -10, 13, // 気絶
			22, -3, 3, // 聴覚保護
			23, -10, 3, // 風圧
			9, -10, 10, // 攻撃
			16, -10, 10, // 達人
			36, -10, 4, // 特殊攻撃
			37, -10, 9, // 火属性攻撃
			38, -10, 9, // 水属性攻撃
			39, -10, 9, // 雷属性攻撃
			40, -10, 9, // 氷属性攻撃
			41, -10, 9, // 龍属性攻撃
			17, -3, 3, // 痛撃
			18, -3, 3, // 重撃
			15, -3, 3, // 溜め短縮
			109, -3, 4, // 本気
			19, -3, 4, // KO
			20, -3, 4, // 減気攻撃
			107, -3, 4, // 底力
			78, -10, 10, // 爆弾強化
			76, -10, 10, // 笛
			77, -10, 10, // 砲術
			10, -10, 10, // 防御
			113, -3, 3, // 属性耐性
			43, -10, 13, // 火耐性
			44, -10, 13, // 水耐性
			45, -10, 13, // 雷耐性
			46, -10, 13, // 氷耐性
			47, -10, 13, // 龍耐性
			13, -10, 9, // 加護
			11, -10, 13, // 体力
			12, -10, 12, // 回復速度
			79, -3, 4, // 回復量
			29, -3, 4, // 回避性能
			30, -3, 4, // 回避距離
			14, -10, 4, // 納刀
			84, -3, 3, // 食事
			27, -3, 3, // スタミナ
			21, -3, 4, // 体術
			28, -3, 4, // 気力回復
			82, -10, 10, // 腹減り
			80, -10, 12, // 広域
			97, -10, 12, // 千里眼
			48, -3, 3, // 斬れ味
			33, -3, 4, // ガード性能
			34, -3, 4, // ガード強化
			51, -10, 8, // 研ぎ師
			59, -3, 3, // 通常弾強化
			60, -3, 3, // 貫通弾強化
			61, -3, 3, // 散弾強化
			54, -3, 4, // 装填速度
			57, -3, 3, // 反動
			102, -10, 13, // 燃鱗
			104, -10, 10, // 水流
			105, -10, 10, // 泳ぎ
			103, -10, 10, // 酸素
			115, -10, 10, // 采配
			116, -10, 10, // 号令
		},

		//weathered
		{
			3, -8, 11, // 気絶
			22, -1, 3, // 聴覚保護
			23, -8, 3, // 風圧
			9, -8, 10, // 攻撃
			16, -8, 10, // 達人
			36, -8, 4, // 特殊攻撃
			42, -3, 3, // 属性攻撃
			37, -8, 9, // 火属性攻撃
			38, -8, 9, // 水属性攻撃
			39, -8, 9, // 雷属性攻撃
			40, -8, 9, // 氷属性攻撃
			41, -8, 9, // 龍属性攻撃
			17, -1, 3, // 痛撃
			18, -1, 3, // 重撃
			55, -3, 3, // 装填数
			15, -1, 3, // 溜め短縮
			109, -1, 4, // 本気
			19, -1, 4, // KO
			20, -1, 4, // 減気攻撃
			107, -1, 4, // 底力
			78, -8, 10, // 爆弾強化
			76, -8, 10, // 笛
			77, -8, 10, // 砲術
			10, -8, 10, // 防御
			113, -1, 3, // 属性耐性
			43, -8, 11, // 火耐性
			44, -8, 11, // 水耐性
			45, -8, 11, // 雷耐性
			46, -8, 11, // 氷耐性
			47, -8, 11, // 龍耐性
			13, -8, 9, // 加護
			11, -8, 11, // 体力
			12, -8, 11, // 回復速度
			79, -1, 4, // 回復量
			29, -1, 4, // 回避性能
			30, -1, 4, // 回避距離
			14, -8, 4, // 納刀
			84, -1, 3, // 食事
			27, -1, 3, // スタミナ
			21, -1, 4, // 体術
			28, -1, 4, // 気力回復
			82, -8, 10, // 腹減り
			93, -3, 3, // 運気
			95, -3, 3, // 捕獲
			94, -3, 3, // 剥ぎ取り
			80, -8, 10, // 広域
			97, -8, 10, // 千里眼
			49, -3, 3, // 匠
			50, -3, 3, // 剣術
			48, -1, 3, // 斬れ味
			53, -3, 3, // 抜刀会心
			52, -3, 3, // 抜刀減気
			33, -1, 4, // ガード性能
			34, -1, 4, // ガード強化
			51, -8, 8, // 研ぎ師
			59, -1, 3, // 通常弾強化
			60, -1, 3, // 貫通弾強化
			61, -1, 3, // 散弾強化
			56, -3, 3, // 速射
			54, -1, 4, // 装填速度
			57, -1, 3, // 反動
			112, -5, 7, // 属性解放
			102, -8, 11, // 燃鱗
			104, -8, 10, // 水流
			105, -8, 10, // 泳ぎ
			103, -8, 10, // 酸素
			108, -3, 3, // 逆境
			85, -5, 5, // 肉食
			31, -5, 5, // 飛距離
			91, -5, 5, // ハチミツ
			8, -5, 5, // 細菌学
			110, -3, 3, // 闘魂
			111, -3, 3, // 無傷
			121, -3, 3, // 不動
			106, -3, 3, // 水の心
			114, -3, 3, // 根性
			119, -3, 3, // 刀匠
			120, -3, 3, // 射手
			117, -3, 3, // 状態耐性
			118, -3, 3, // 怒
			115, -8, 10, // 采配
			116, -8, 10, // 号令
			92, -8, 10, // 護石王
		},
	};
#pragma endregion
#pragma region Slot Table
	static array< array< unsigned char >^ >^ slot_table =
	{
		//mystery
		{
			70, 100, 100,
			75, 100, 100,
			80, 100, 100,
			85, 100, 100,
			90, 100, 100,
			92, 100, 100,
			95, 100, 100,
			99, 100, 100,
			100, 100, 100,
		},

		//shining =
		{
			50, 85, 100,
			58, 88, 100,
			65, 90, 100,
			72, 92, 100,
			79, 95, 100,
			83, 97, 100,
			86, 98, 100,
			89, 99, 100,
			91, 99, 100,
			95, 100, 100,
			97, 100, 100,
			99, 100, 100,
			99, 100, 100,
			100, 100, 100,
		},

		//ancient =
		{
			22, 72, 92,
			41, 76, 94,
			49, 79, 95,
			57, 82, 96,
			65, 85, 97,
			70, 88, 98,
			76, 91, 99,
			82, 94, 99,
			87, 97, 99,
			91, 99, 100,
			94, 99, 100,
			96, 99, 100,
			99, 100, 100,
			99, 100, 100,
			99, 100, 100,
			100, 100, 100,
		},

		//weathered =
		{
			22, 72, 92,
			41, 76, 96,
			48, 78, 96,
			59, 81, 97,
			63, 83, 97,
			6, 86, 98,
			76, 88, 98,
			76, 91, 99,
			84, 94, 99,
			88, 98, 100,
			90, 98, 100,
			93, 99, 100,
			97, 99, 100,
			98, 100, 100,
			99, 100, 100,
			99, 100, 100,
			100, 100, 100,
		},
	};
#pragma endregion
#pragma region Skill2Chance Table	
	static array< unsigned char >^ skill2_chance_table =
	{
		100,
		35,
		25,
		20
	};
#pragma endregion
};

int rnd( const int n )
{
	Assert( n < 65536 && n >= 0, L"Bad RND" );
	if( n == 0 ) return 176;

	int x = n * 16 - n * 4 - n;
	long long y = x * 16;
	long long z = y * 11367737L / 4294967296L;
	long long m = y - ( ( y - z ) / 2L + z ) / 32768L * 65363L;
	return (int)( m & 0xFFFF );
}

unsigned GetNumSlots( const unsigned charm_type, const int slot_table, const unsigned roll )
{
	Assert( (int)charm_type < StaticData::slot_table->Length, L"Bad charm type" );

	const unsigned table_index = Math::Min( slot_table, StaticData::slot_table[ charm_type ]->Length / 3 ) - 1;
	const unsigned for2 = StaticData::slot_table[ charm_type ][ table_index * 3 + 1 ];
	const unsigned for3 = StaticData::slot_table[ charm_type ][ table_index * 3 + 2 ];
	const unsigned for1 = StaticData::slot_table[ charm_type ][ table_index * 3 + 0 ];
	if( roll >= for2 )
	{
		return ( roll >= for3 ) ? 3 : 2;
	}
	else
	{
		return ( roll >= for1 ) ? 1 : 0;
	}
}

void GenerateCharm( const unsigned charm_type, const unsigned table, int n )
{
	//check charm_type < StaticData::skill1_table->Length
	array< unsigned char >^ skill1_table = StaticData::skill1_table[ charm_type ];
	const unsigned num_skills1 = skill1_table->Length / 3;
	//check charm_type < StaticData::skill2_table->Length
	array< signed char >^ skill2_table = StaticData::skill2_table[ charm_type ];
	const unsigned num_skills2 = skill2_table->Length / 3;

	//skill 1
	n = rnd( n );
	const int skill1_index = n % num_skills1;

	//skill 1 point
	n = rnd( n );
	const int skill1_name = skill1_table[ skill1_index * 3 ];
	const int skill1_min = skill1_table[ skill1_index * 3 + 1 ];
	const int skill1_max = skill1_table[ skill1_index * 3 + 2 ];
	const int point1 = skill1_min + n % ( skill1_max - skill1_min + 1 );

	//has skill 2?
	int skill2_index = -1, point2 = 0, skill2_min = 0, skill2_max = 0, skill2_name = 0;
	n = rnd( n );
	if( n % 100 >= StaticData::skill2_chance_table[ charm_type ] )
	{
		//skill 2
		n = rnd( n );
		skill2_index = n % num_skills2;
		skill2_name = skill2_table[ skill2_index * 3 ];
		skill2_min = skill2_table[ skill2_index * 3 + 1 ];
		skill2_max = skill2_table[ skill2_index * 3 + 2 ];

		//skill 2 point
		n = rnd( n );
		if( n % 2 == 1 ) //positive number
		{
			n = rnd( n );
			point2 = 1 + n % skill2_max;
		}
		else //negative number
		{
			n = rnd( n );
			point2 = skill2_min + n % ( 1 - skill2_min );
		}

		if( skill1_name == skill2_name )
		{
			skill2_min = skill2_max = point2 = 0;
			skill2_name = skill2_index = Ability::static_abilities.Count;
		}
	}

	const int slot_table = (int)Math::Floor( point1*10.0 / skill1_max + ( (point2 > 0) ? point2*10.0 / skill2_max : 0 ) );

	//slots
	n = rnd( n );
	const int num_slots = GetNumSlots( charm_type, slot_table, n % 100 );

	Charm^ charm = gcnew Charm( num_slots );
	charm->abilities.Add( gcnew AbilityPair( Ability::static_abilities[ skill1_name ], point1 ) );
	
	for( unsigned ct = charm_type; ct < 4; ++ct )
	{
		TableSlotDatum^ current_table = CharmDatabase::min_max[ table, ct ];
		TableSlotDatum^ all_table = CharmDatabase::min_max[ AllTable, ct ];

		MaxOut( current_table->max_single[ skill1_name, num_slots ], point1 );
		MaxOut( all_table->max_single[ skill1_name, num_slots ], point1 );

		CharmDatabase::have_slots[ table, charm_type ] |= 1 << num_slots;
		CharmDatabase::have_slots[ AllTable, charm_type ] |= 1 << num_slots;
	}

	if( point2 != 0 )
	{
		charm->abilities.Add( gcnew AbilityPair( Ability::static_abilities[ skill2_name ], point2 ) );
		/*if( skill2_name == 119 && table == 5 )
		{
			Assert( false, L"asdf" );
		}*/

		for( unsigned ct = charm_type; ct < 4; ++ct )
		{
			TableSlotDatum^ current_table = CharmDatabase::min_max[ table, ct ];
			TableSlotDatum^ all_table = CharmDatabase::min_max[ AllTable, ct ];

			MaxOut( current_table->max_single[ skill2_name, num_slots ], point2 );
			MaxOut( all_table->max_single[ skill2_name, num_slots ], point2 );
			
			List_t< Charm^ >^ list11 = current_table->two_skill_data[ num_slots ][ skill2_name, skill1_name ];
			Assert( list11 != nullptr, L"null list11" );
			Charm::AddToOptimalList( *list11, charm );

			/*List_t< Charm^ >^ list12 = current_table->two_skill_data[ num_slots ][ skill1_name, skill2_name ];
			Assert( list12 != nullptr, L"null list12" );
			Charm::AddToOptimalList( *list12, charm );*/

			List_t< Charm^ >^ list21 = all_table->two_skill_data[ num_slots ][ skill2_name, skill1_name ];
			Assert( list21 != nullptr, L"null list21" );
			Charm::AddToOptimalList( *list21, charm );

			/*List_t< Charm^ >^ list22 = all_table->two_skill_data[ num_slots ][ skill1_name, skill2_name ];
			Assert( list22 != nullptr, L"null list22" );
			Charm::AddToOptimalList( *list22, charm );*/
		}
	}
	
	const unsigned charm_hash = charm->GetHash();
	const unsigned bits = ( 1 << table ) | ( 0x80000000 >> charm_type );
	if( CharmDatabase::hash_to_table->ContainsKey( charm_hash ) )
		CharmDatabase::hash_to_table[ charm_hash ] |= bits;
	else CharmDatabase::hash_to_table->Add( charm_hash, bits );
}

void CharmDatabase::GenerateCharmTable()
{
	//mark all abilities as relevant for making optimal charm lists
	for each( Ability^ a in Ability::static_abilities )
	{
		a->relevant = true;
	}

	const unsigned num_skills = Ability::static_abilities.Count;
	const unsigned num_charm_types = StaticData::slot_table->Length;

	//set up databases
	array< int >^ table_seeds = { 1, 15, 5, 13, 4, 3, 9, 12, 26, 18, 163, 401, 6, 2, 489, 802, 1203 };
	Assert( TotalTables == table_seeds->Length + 2, L"Incorrect number of table seeds." );
	min_max = gcnew array< TableSlotDatum^, 2 >( TotalTables, num_charm_types );
	hash_to_table = gcnew Map_t< unsigned, unsigned >();
	location_cache = gcnew Map_t< System::String^, CharmLocationDatum^ >();
	have_slots = gcnew array< unsigned char, 2 >( TotalTables, num_charm_types );
	for( unsigned t = 0; t < TotalTables; ++t )
	{
		for( unsigned ct = 0; ct < num_charm_types; ++ct )
		{
			TableSlotDatum^ d = gcnew TableSlotDatum();
			min_max[ t, ct ] = d;

			d->max_single = gcnew array< signed char, 2 >( num_skills, 4 );

			if( t > 0 )
			{
				d->two_skill_data = gcnew array< array< List_t< Charm^ >^, 2 >^ >( 4 );
				for( unsigned s = 0; s < 4; ++s )
				{
					d->two_skill_data[ s ] = gcnew array< List_t< Charm^ >^, 2 >( num_skills, num_skills );
					for( unsigned skill1 = 1; skill1 < num_skills; ++skill1 )
					{
						for( unsigned skill2 = 0; skill2 < skill1; ++skill2 )
						{
							d->two_skill_data[ s ][ skill1, skill2 ] = gcnew List_t< Charm^ >();
							d->two_skill_data[ s ][ skill2, skill1 ] = d->two_skill_data[ s ][ skill1, skill2 ];
						}
					}
				}
			}
		}
	}

	/*const unsigned table = 9;
	int n = table_seeds[ table - 1 ];
	do
	{		
		GenerateCharm( 3, table, n );
		n = rnd( n );
	}
	while( n != table_seeds[ table - 1 ] );
	return;*/

	//generate charms
	//array< unsigned >^ counts = gcnew array< unsigned >( 17 );
	for( int t = 0; t < table_seeds->Length; ++t )
	{
		for( unsigned charm_type = 0; charm_type < num_charm_types; ++charm_type )
		{
			int n = table_seeds[ t ];
			do
			{		
				GenerateCharm( charm_type, t+1, n );
				n = rnd( n );
				//counts[ t ]++;
			}
			while( n != table_seeds[ t ] );
		}
	}

	//turn abilities back to not-yet-relevant
	for each( Ability^ a in Ability::static_abilities )
	{
		a->relevant = false;
	}

	//fix up have_slot table
	for( unsigned t = 1; t < TotalTables; ++t )
	{
		for( unsigned ct = 1; ct < num_charm_types; ++ct )
			have_slots[ t, ct ] |= have_slots[ t, ct - 1 ];
	}

	//calculate "don't know" tables
	for( unsigned ct = 0; ct < num_charm_types; ++ct )
	{
		array< signed char, 2 >::Copy( min_max[ 1, ct ]->max_single, 0, min_max[ 0, ct ]->max_single, 0, 4 * num_skills );
		have_slots[ 0, ct ] = have_slots[ 1, ct ];

		for( unsigned t = 2; t < TotalTables-1; ++t )
		{
			//skip cursed tables
			if( t == 12 || t == 13  || t == 15 || t == 16 || t == 17 )
				continue;

			for( unsigned skill = 0; skill < num_skills; ++skill )
				for( unsigned slots = 0; slots < 4; ++slots )
				{
					const signed char m = min_max[ t, ct ]->max_single[ skill, slots ];
					MinOut( min_max[ 0, ct ]->max_single[ skill, slots ], m );
				}
			have_slots[ 0, ct ] &= have_slots[ t, ct ];
		}
	}
}

#pragma endregion

int GetBitPositionFromRight( int v )
{
	int c;
	// if v is 1, returns 31.
	if (v & 0x1) 
	{
		// special case for odd v (assumed to happen half of the time)
		c = 0;
	}
	else
	{
		c = 1;
		if ((v & 0xffff) == 0) 
		{  
			v >>= 16;  
			c += 16;
		}
		if ((v & 0xff) == 0) 
		{  
			v >>= 8;  
			c += 8;
		}
		if ((v & 0xf) == 0) 
		{  
			v >>= 4;
			c += 4;
		}
		if ((v & 0x3) == 0) 
		{  
			v >>= 2;
			c += 2;
		}
		c -= v & 0x1;
	}
	return c;
}

int CheckCount( array< unsigned >^ counts )
{
	unsigned best = 0, num_best = 1;
	for( int i = 1; i < counts->Length; ++i )
	{
		if( counts[ i ] > counts[ best ] )
		{
			best = i;
			num_best = 1;
		}
		else if( counts[ i ] == counts[ best ] )
			num_best++;
	}
	if( num_best == 1 && counts[ best ] > 0 )
		return best + 1;
	return -1;
}

bool IsAutoguardCharm( Charm^ charm )
{
	return charm->num_slots == 0 &&
		charm->abilities.Count == 1 &&
		charm->abilities[ 0 ]->amount == 10 &&
		charm->abilities[ 0 ]->ability->auto_guard;
}

int CharmDatabase::DetectCharmTable()
{
	array< unsigned >^ counts = gcnew array< unsigned >( 17 );
	unsigned unknown = 0;

	for each( Charm^ charm in mycharms )
	{
		if( IsAutoguardCharm( charm ) )
			continue;
		const unsigned charm_hash = charm->GetHash();
		if( hash_to_table->ContainsKey( charm_hash ) )
		{
			//check unique tables
			const int bits = hash_to_table[ charm_hash ] & 0x0FFFFFFF;
			if( bits == ( bits & -bits ) )
			{
				const int table = GetBitPositionFromRight( bits );
				counts[ table - 1 ]++;
			}
		}
		else unknown++;
	}
	int res = CheckCount( counts );
	if( res != -1 )
	{
		String^ text = FormatString2( FoundNCharmsUniqueToTable, counts[ res - 1 ], res ) + L".";
		if( res == 11 || res == 12 || res == 15 || res == 16 || res == 17 )
		text += L" " + StaticString( SorryAboutTheCharms );
		System::Windows::Forms::MessageBox::Show( text );
		return res;
	}

	counts = gcnew array< unsigned >( 17 );
	for each( Charm^ charm in mycharms )
	{
		const unsigned charm_hash = charm->GetHash();
		if( hash_to_table->ContainsKey( charm_hash ) )
		{
			//check unique tables
			const int bits = hash_to_table[ charm_hash ];
			for( int i = 1; i <= counts->Length; ++i )
			{
				if( bits & ( 1 << i ) )
					counts[ i - 1 ]++;
			}
		}
	}
	res = CheckCount( counts );
	if( res != -1 )
	{
		String^ text = FormatString2( FoundNCharmsOnTable, counts[ res - 1 ], res ) + L".";
		if( res == 11 || res == 12 || res == 15 || res == 16 || res == 17 )
			text += L" " + StaticString( SorryAboutTheCharms );
		System::Windows::Forms::MessageBox::Show( text );
		return res;
	}
	System::Windows::Forms::MessageBox::Show( StaticString( FindTableFailed ) );
	return -1;
}

bool CharmDatabase::CharmExists( Charm^ charm )
{
	return IsAutoguardCharm( charm ) || hash_to_table->ContainsKey( charm->GetHash() );
}

bool CharmDatabase::CharmIsLegal( Charm^ charm )
{
	if( charm->num_slots >= 4 )
		return false;
	if( charm->abilities.Count == 0 || IsAutoguardCharm( charm ) )
		return true;
	else if( charm->abilities.Count == 1 )
	{
		return charm->abilities[ 0 ]->amount <= min_max[ AllTable, 3 ]->max_single[ charm->abilities[ 0 ]->ability->static_index, charm->num_slots ];
	}
	else if( charm->abilities.Count == 2 )
	{
		List_t< Charm^ >^ best_charms = min_max[ AllTable, 3 ]->two_skill_data[ charm->num_slots ][ charm->abilities[ 0 ]->ability->static_index, charm->abilities[ 1 ]->ability->static_index ];
		for each( Charm^ b in best_charms )
		{
			if( b->abilities[ 0 ]->ability->static_index == charm->abilities[ 0 ]->ability->static_index &&
				b->abilities[ 1 ]->ability->static_index == charm->abilities[ 1 ]->ability->static_index &&
				b->abilities[ 0 ]->amount >= charm->abilities[ 0 ]->amount &&
				b->abilities[ 1 ]->amount >= charm->abilities[ 1 ]->amount
				||
				b->abilities[ 0 ]->ability->static_index == charm->abilities[ 1 ]->ability->static_index &&
				b->abilities[ 1 ]->ability->static_index == charm->abilities[ 0 ]->ability->static_index &&
				b->abilities[ 0 ]->amount >= charm->abilities[ 1 ]->amount &&
				b->abilities[ 1 ]->amount >= charm->abilities[ 0 ]->amount )
				return true;
		}
	}
	return false;
}