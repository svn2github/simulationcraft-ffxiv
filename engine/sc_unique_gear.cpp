// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simulationcraft.hpp"

namespace { // UNNAMED NAMESPACE

#define maintenance_check( ilvl ) static_assert( ilvl >= 372, "unique item below min level, should be deprecated." )

struct stat_buff_proc_t : public buff_proc_callback_t<stat_buff_t>
{
  stat_buff_proc_t( player_t* p, const special_effect_t& data ) :
    buff_proc_callback_t<stat_buff_t>( p, data )
  {
    buff = stat_buff_creator_t( listener, proc_data.name_str )
           .activated( false )
           .max_stack( proc_data.max_stacks )
           .duration( proc_data.duration )
           .reverse( proc_data.reverse )
           .add_stat( proc_data.stat, proc_data.stat_amount );
  }
};

struct cost_reduction_buff_proc_t : public buff_proc_callback_t<cost_reduction_buff_t>
{
  cost_reduction_buff_proc_t( player_t* p, const special_effect_t& data ) :
    buff_proc_callback_t<cost_reduction_buff_t>( p, data )
  {
    buff = cost_reduction_buff_creator_t( listener, proc_data.name_str )
           .activated( false )
           .max_stack( proc_data.max_stacks )
           .duration( proc_data.duration )
           .reverse( proc_data.reverse )
           .amount( proc_data.discharge_amount * p -> challenge_mode_power_loss_ratio )
           .refreshes( ! proc_data.no_refresh );
  }
};

struct discharge_spell_t : public spell_t
{
  discharge_spell_t( player_t* p, special_effect_t& data ) :
    spell_t( data.name_str, p, spell_data_t::nil() )
  {
    school = ( data.school == SCHOOL_DRAIN ) ? SCHOOL_SHADOW : data.school;
    discharge_proc = true;
    item_proc = true;
    trigger_gcd = timespan_t::zero();
    base_dd_min = data.discharge_amount;
    base_dd_max = data.discharge_amount;
    direct_power_mod = data.discharge_scaling;
    may_crit = ( data.school != SCHOOL_DRAIN ) && ( ( data.override_result_es_mask & RESULT_CRIT_MASK ) ? ( data.result_es_mask & RESULT_CRIT_MASK ) : true ); // Default true
    may_miss = ( data.override_result_es_mask & RESULT_MISS_MASK ) ? ( data.result_es_mask & RESULT_MISS_MASK ) != 0 : may_miss;
    background  = true;
    aoe = data.aoe;
  }
};

struct discharge_attack_t : public attack_t
{
  discharge_attack_t(  player_t* p, special_effect_t& data ) :
    attack_t( data.name_str, p, spell_data_t::nil() )
  {
    school = ( data.school == SCHOOL_DRAIN ) ? SCHOOL_SHADOW : data.school;
    discharge_proc = true;
    item_proc = true;
    trigger_gcd = timespan_t::zero();
    base_dd_min = data.discharge_amount;
    base_dd_max = data.discharge_amount;
    direct_power_mod = data.discharge_scaling;
    may_crit = ( data.school != SCHOOL_DRAIN ) && ( ( data.override_result_es_mask & RESULT_CRIT_MASK ) ? ( data.result_es_mask & RESULT_CRIT_MASK ) : true ); // Default true
    may_miss = ( data.override_result_es_mask & RESULT_MISS_MASK ) ? ( data.result_es_mask & RESULT_MISS_MASK ) != 0 : may_miss;
    may_dodge = ( data.school == SCHOOL_PHYSICAL ) && ( ( data.override_result_es_mask & RESULT_DODGE_MASK ) ? ( data.result_es_mask & RESULT_DODGE_MASK ) : may_dodge );
    may_parry = ( data.school == SCHOOL_PHYSICAL ) && ( ( data.override_result_es_mask & RESULT_PARRY_MASK ) ? ( data.result_es_mask & RESULT_PARRY_MASK ) : may_parry )
                && ( p -> position() == POSITION_FRONT || p ->position()== POSITION_RANGED_FRONT );
    may_block = ( data.school == SCHOOL_PHYSICAL ) && ( ( data.override_result_es_mask & RESULT_BLOCK_MASK ) ? ( data.result_es_mask & RESULT_BLOCK_MASK ) : may_block )
                && ( p -> position() == POSITION_FRONT || p -> position() == POSITION_RANGED_FRONT );
    may_glance = false;
    background  = true;
    aoe = data.aoe;
  }
};

struct discharge_proc_callback_base_t : public discharge_proc_t<action_t>
{
  discharge_proc_callback_base_t( player_t* p, const special_effect_t& data ) :
    discharge_proc_t<action_t>( p, data, nullptr )
  {
    cooldown = p -> get_cooldown( proc_data.name_str );
    cooldown -> duration = proc_data.cooldown;

    if ( proc_data.discharge_amount > 0 )
    {
      discharge_action = new discharge_spell_t( p, proc_data );
    }
    else
    {
      discharge_action = new discharge_attack_t( p, proc_data );
    }
  }
};

// discharge_proc_callback ==================================================

struct discharge_proc_callback_t : public discharge_proc_callback_base_t
{

  discharge_proc_callback_t( player_t* p, const special_effect_t& data ) :
    discharge_proc_callback_base_t( p, data )
  { }
};

// chance_discharge_proc_callback ===========================================

struct chance_discharge_proc_callback_t : public discharge_proc_callback_base_t
{

  chance_discharge_proc_callback_t( player_t* p, const special_effect_t& data ) :
    discharge_proc_callback_base_t( p, data )
  { }

  virtual double proc_chance()
  {
    if ( discharge_stacks == this -> proc_data.max_stacks )
      return 1.0;

    return discharge_proc_callback_base_t::proc_chance();
  }
};

// stat_discharge_proc_callback =============================================

struct stat_discharge_proc_callback_t : public action_callback_t
{
  std::string name_str;
  stat_buff_t* buff;
  action_t* discharge_action;

  stat_discharge_proc_callback_t( const std::string& n,
                                  player_t* p,
                                  stat_e stat,
                                  int ms,
                                  double stat_amount,
                                  school_e school,
                                  double discharge_amount,
                                  double discharge_scaling,
                                  double proc_chance,
                                  timespan_t duration,
                                  timespan_t cooldown,
                                  int aoe,
                                  bool activated = true,
                                  unsigned int override_result_es_mask = 0,
                                  unsigned int result_es_mask = 0 ) :
    action_callback_t( p ), name_str( n )
  {
    if ( ms == 0 ) ms = 1;
    if ( proc_chance == 0 ) proc_chance = 1;

    buff = stat_buff_creator_t( p, n )
           .max_stack( ms )
           .duration( duration )
           .cd( cooldown )
           .chance( proc_chance )
           .activated( activated )
           .add_stat( stat, stat_amount*p->challenge_mode_power_loss_ratio );

    struct discharge_spell_t : public spell_t
    {
      discharge_spell_t( const std::string n, player_t* p, double amount, double scaling, school_e s, int ae,
                         unsigned int override_result_es_mask = 0, unsigned int result_es_mask = 0 ) :
        spell_t( n, p, spell_data_t::nil() )
      {
        school = ( s == SCHOOL_DRAIN ) ? SCHOOL_SHADOW : s;
        discharge_proc = true;
        item_proc = true;
        trigger_gcd = timespan_t::zero();
        base_dd_min = amount;
        base_dd_max = amount;
        direct_power_mod = scaling;
        may_crit = ( s != SCHOOL_DRAIN ) && ( ( override_result_es_mask & RESULT_CRIT_MASK ) ? ( result_es_mask & RESULT_CRIT_MASK ) != 0 : true ); // Default true
        may_miss = ( override_result_es_mask & RESULT_MISS_MASK ) ? ( result_es_mask & RESULT_MISS_MASK ) != 0 : may_miss;
        background  = true;
        aoe = ae;
      }
    };

    struct discharge_attack_t : public attack_t
    {
      discharge_attack_t( const std::string& n, player_t* p, double amount, double scaling, school_e s, int ae,
                          unsigned int override_result_es_mask = 0, unsigned int result_es_mask = 0 ) :
        attack_t( n, p, spell_data_t::nil() )
      {
        school = ( s == SCHOOL_DRAIN ) ? SCHOOL_SHADOW : s;
        discharge_proc = true;
        item_proc = true;
        trigger_gcd = timespan_t::zero();
        base_dd_min = amount;
        base_dd_max = amount;
        direct_power_mod = scaling;
        may_crit = ( s != SCHOOL_DRAIN ) && ( ( override_result_es_mask & RESULT_CRIT_MASK ) ? ( result_es_mask & RESULT_CRIT_MASK ) != 0 : true ); // Default true
        may_miss = ( override_result_es_mask & RESULT_MISS_MASK ) ? ( result_es_mask & RESULT_MISS_MASK ) != 0 : may_miss;
        may_dodge = ( s == SCHOOL_PHYSICAL ) && ( ( override_result_es_mask & RESULT_DODGE_MASK ) ? ( result_es_mask & RESULT_DODGE_MASK ) : may_dodge );
        may_parry = ( s == SCHOOL_PHYSICAL ) && ( ( override_result_es_mask & RESULT_PARRY_MASK ) ? ( result_es_mask & RESULT_PARRY_MASK ) : may_parry )
                    && ( p -> position() == POSITION_FRONT || p -> position() == POSITION_RANGED_FRONT );
        may_block = ( s == SCHOOL_PHYSICAL ) && ( ( override_result_es_mask & RESULT_BLOCK_MASK ) ? ( result_es_mask & RESULT_BLOCK_MASK ) : may_block )
                    && ( p -> position() == POSITION_FRONT || p -> position() == POSITION_RANGED_FRONT );
        may_glance = false;
        background  = true;
        aoe = ae;
      }
    };

    if ( discharge_amount > 0 )
    {
      discharge_action = new discharge_spell_t( name_str, p, discharge_amount*p->challenge_mode_power_loss_ratio, discharge_scaling, school, aoe, override_result_es_mask, result_es_mask );
    }
    else
    {
      discharge_action = new discharge_attack_t( name_str, p, -discharge_amount*p->challenge_mode_power_loss_ratio, discharge_scaling, school, aoe, override_result_es_mask, result_es_mask );
    }
  }

  virtual void deactivate()
  {
    action_callback_t::deactivate();
    buff -> expire();
  }

  virtual void trigger( action_t* a, void* /* call_data */ )
  {
    if ( buff -> trigger( a ) )
    {
      if ( ! allow_self_procs && ( a == discharge_action ) ) return;
      discharge_action -> execute();
    }
  }
};

} // UNNAMED NAMESPACE

// register_touch_of_the_grave ==============================================

static void register_touch_of_the_grave( player_t* p )
{
  assert( p );

  const spell_data_t* s = p -> find_racial_spell( "Touch of the Grave" );

  if ( ! s -> ok() )
  {
    return;
  }

  struct touch_of_the_grave_discharge_spell_t : public spell_t
  {
    touch_of_the_grave_discharge_spell_t( player_t* p, const spell_data_t* s ) :
      spell_t( "touch_of_the_grave", p, s )
    {
      school           = ( s -> effectN( 1 ).trigger() -> get_school_type() == SCHOOL_DRAIN ) ? SCHOOL_SHADOW : s -> effectN( 1 ).trigger() -> get_school_type();
      discharge_proc   = true;
      trigger_gcd      = timespan_t::zero();
      base_dd_min      = s -> effectN( 1 ).trigger() -> effectN( 1 ).average( p );
      base_dd_max      = s -> effectN( 1 ).trigger() -> effectN( 1 ).average( p );
      direct_power_mod = s -> effectN( 1 ).trigger() -> effectN( 1 )._coeff;
      may_crit         = false;
      may_miss         = false;
      background       = true;
      aoe              = 0;
    }

    virtual void impact( action_state_t* s )
    {
      spell_t::impact( s );

      if ( result_is_hit( s -> result ) )
      {
        player -> resource_gain( RESOURCE_HEALTH, s -> result_amount, player -> gains.touch_of_the_grave );
      }
    }
  };


  struct touch_of_the_grave_proc_callback_t : public discharge_proc_t<action_t>
  {
    touch_of_the_grave_proc_callback_t( player_t* p, special_effect_t& data, const spell_data_t* s ) :
      discharge_proc_t<action_t>( p, data, new touch_of_the_grave_discharge_spell_t( p, s ) )
    { }
  };

  special_effect_t se_data;
  se_data.name_str = "touch_of_the_grave";
  se_data.proc_chance = s -> proc_chance();
  se_data.cooldown = timespan_t::from_seconds( 15.0 );

  action_callback_t* cb = new touch_of_the_grave_proc_callback_t( p, se_data, s );

  p -> callbacks.register_attack_callback       ( RESULT_HIT_MASK, cb );
  p -> callbacks.register_harmful_spell_callback( RESULT_HIT_MASK, cb );
}

// register_apparatus_of_khazgoroth =========================================

static void register_apparatus_of_khazgoroth( item_t* item )
{
  maintenance_check( 391 );

  player_t* p = item -> player;

  item -> unique = true;
  bool heroic = item -> heroic();

  struct apparatus_of_khazgoroth_callback_t : public action_callback_t
  {
    bool heroic;
    buff_t* apparatus_of_khazgoroth;
    stat_buff_t* blessing_of_khazgoroth;
    proc_t* proc_apparatus_of_khazgoroth_haste;
    proc_t* proc_apparatus_of_khazgoroth_crit;
    proc_t* proc_apparatus_of_khazgoroth_mastery;

    apparatus_of_khazgoroth_callback_t( player_t* p, bool h ) :
      action_callback_t( p ), heroic( h )
    {

      apparatus_of_khazgoroth = buff_creator_t( p, "titanic_power" )
                                .spell( p -> find_spell( 96923 ) )
                                .activated( false ); // TODO: Duration, cd, etc.?

      blessing_of_khazgoroth  = stat_buff_creator_t( p, "blessing_of_khazgoroth" )
                                .duration( timespan_t::from_seconds( 15.0 ) )
                                .cd( timespan_t::from_seconds( 120.0 ) );

      proc_apparatus_of_khazgoroth_haste   = p -> get_proc( "apparatus_of_khazgoroth_haste"   );
      proc_apparatus_of_khazgoroth_crit    = p -> get_proc( "apparatus_of_khazgoroth_crit"    );
      proc_apparatus_of_khazgoroth_mastery = p -> get_proc( "apparatus_of_khazgoroth_mastery" );
    }

    virtual void trigger( action_t* a, void* /* call_data */ )
    {
      if ( ! a -> weapon ) return;
      if ( a -> proc ) return;

      if ( apparatus_of_khazgoroth -> trigger() )
      {
        if ( blessing_of_khazgoroth -> cooldown -> down() ) return;

        double amount = heroic ? 2875 : 2540;

        blessing_of_khazgoroth->stats.clear();

        // FIXME: This really should be a /use action
        if ( apparatus_of_khazgoroth -> check() == 5 )
        {
          // Highest of Crits/Master/haste is chosen
          blessing_of_khazgoroth -> stats.push_back( stat_buff_t::buff_stat_t( STAT_CRIT_RATING, amount ) );
          if ( a -> player -> stats.haste_rating   > a -> player -> stats.crit_rating ||
               a -> player -> stats.mastery_rating > a -> player -> stats.crit_rating )
          {
            if ( a -> player -> stats.mastery_rating > a -> player -> stats.haste_rating )
            {

              blessing_of_khazgoroth -> stats.push_back( stat_buff_t::buff_stat_t( STAT_MASTERY_RATING, amount ) );
              proc_apparatus_of_khazgoroth_mastery -> occur();
            }
            else
            {

              blessing_of_khazgoroth -> stats.push_back( stat_buff_t::buff_stat_t( STAT_HASTE_RATING, amount ) );
              proc_apparatus_of_khazgoroth_haste -> occur();
            }
          }
          else
          {
            proc_apparatus_of_khazgoroth_crit -> occur();
          }
          apparatus_of_khazgoroth -> expire();
          blessing_of_khazgoroth -> trigger();
        }
      }
    }
  };

  p -> callbacks.register_attack_callback( RESULT_CRIT_MASK, new apparatus_of_khazgoroth_callback_t( p, heroic ) );
}

// register_heart_of_ignacious ==============================================

static void register_heart_of_ignacious( item_t* item )
{
  maintenance_check( 372 );

  player_t* p = item -> player;

  item -> unique = true;

  special_effect_t data;
  data.name_str    = "heart_of_ignacious";
  data.stat        = STAT_SPELL_POWER;
  data.stat_amount = item -> heroic() ? 87 : 77;
  data.max_stacks  = 5;
  data.duration    = timespan_t::from_seconds( 15 );
  data.cooldown    = timespan_t::from_seconds( 2 );

  struct heart_of_ignacious_callback_t : public stat_buff_proc_t
  {
    stat_buff_t* haste_buff;

    heart_of_ignacious_callback_t( item_t& item, const special_effect_t& data ) :
      stat_buff_proc_t( item.player, data )
    {
      haste_buff = stat_buff_creator_t( item.player, "hearts_judgement" )
                   .max_stack( 5 )
                   .duration( timespan_t::from_seconds( 20.0 ) )
                   .cd( timespan_t::from_seconds( 120.0 ) )
                   .add_stat( STAT_HASTE_RATING, item.heroic() ? 363 : 321 );
    }

    void execute( action_t* a, action_state_t* state )
    {
      stat_buff_proc_t::execute( a, state );

      if ( buff -> stack() == buff -> max_stack() )
      {
        if ( haste_buff -> trigger( buff -> max_stack() ) )
        {
          buff -> expire();
        }
      }
    }
  };

  heart_of_ignacious_callback_t* cb = new heart_of_ignacious_callback_t( *item, data );
  p -> callbacks.register_tick_damage_callback( SCHOOL_ALL_MASK, cb );
  p -> callbacks.register_direct_damage_callback( SCHOOL_ALL_MASK, cb  );
}

// register_matrix_restabilizer =============================================

static void register_matrix_restabilizer( item_t* item )
{
  maintenance_check( 397 );

  player_t* p = item -> player;

  item -> unique = true;

  special_effect_t data;
  data.name_str    = "matrix_restabilizer";
  data.cooldown    = timespan_t::from_seconds( 105 );
  data.proc_chance = 0.15;

  struct matrix_restabilizer_callback_t : public proc_callback_t<action_state_t>
  {
    stat_buff_t* buff_matrix_restabilizer_crit;
    stat_buff_t* buff_matrix_restabilizer_haste;
    stat_buff_t* buff_matrix_restabilizer_mastery;


    matrix_restabilizer_callback_t( item_t& i, const special_effect_t& data ) :
      proc_callback_t<action_state_t>( i.player, data )
    {
      double amount = i.heroic() ? 1834 : 1624;

      struct common_buff_creator : public stat_buff_creator_t
      {
        common_buff_creator( player_t* p, const std::string& n ) :
          stat_buff_creator_t ( p, "matrix_restabilizer_" + n )
        {
          duration ( timespan_t::from_seconds( 30 ) );
          activated( false );
        }
      };

      buff_matrix_restabilizer_crit    = common_buff_creator( i.player, "crit" )
                                         .add_stat( STAT_CRIT_RATING, amount );
      buff_matrix_restabilizer_haste   = common_buff_creator( i.player, "haste" )
                                         .add_stat( STAT_HASTE_RATING, amount );
      buff_matrix_restabilizer_mastery = common_buff_creator( i.player, "mastery" )
                                         .add_stat( STAT_MASTERY_RATING, amount );
    }

    virtual void execute( action_t* a, action_state_t* /* call_data */ )
    {
      player_t* p = a -> player;

      if ( p -> stats.crit_rating > p -> stats.haste_rating )
      {
        if ( p -> stats.crit_rating > p -> stats.mastery_rating )
          buff_matrix_restabilizer_crit -> trigger();
        else
          buff_matrix_restabilizer_mastery -> trigger();
      }
      else if ( p -> stats.haste_rating > p -> stats.mastery_rating )
        buff_matrix_restabilizer_haste -> trigger();
      else
        buff_matrix_restabilizer_mastery -> trigger();
    }
  };

  p -> callbacks.register_attack_callback( RESULT_HIT_MASK, new matrix_restabilizer_callback_t( *item, data ) );
}

// register_shard_of_woe ====================================================

static void register_shard_of_woe( item_t* item )
{
  maintenance_check( 379 );

  // February 15 - Hotfix
  // Shard of Woe (Heroic trinket) now reduces the base mana cost of
  // spells by 205, with the exception of Holy and Nature spells --
  // the base mana cost of Holy and Nature spells remains reduced
  // by 405 with this trinket.
  player_t* p = item -> player;

  item -> unique = true;

  for ( school_e i = SCHOOL_NONE; i < SCHOOL_MAX; i++ )
  {
    p -> initial.resource_reduction[ i ] += 205;
  }
}

// register_blazing_power ===================================================

static void register_blazing_power( item_t* item )
{
  maintenance_check( 391 );

  player_t* p = item -> player;

  item -> unique = true;

  struct blazing_power_heal_t : public heal_t
  {
    blazing_power_heal_t( player_t* p, bool heroic ) :
      heal_t( "blaze_of_life", p, ( p -> dbc.spell( heroic ? 97136 : 96966 ) ) )
    {
      trigger_gcd = timespan_t::zero();
      background  = true;
      may_miss = false;
      may_crit = true;
      callbacks = false;
      init();
    }
  };

  struct blazing_power_callback_t : public action_callback_t
  {
    heal_t* heal;
    cooldown_t* cd;
    proc_t* proc;
    rng_t* rng;

    blazing_power_callback_t( player_t* p, heal_t* s ) :
      action_callback_t( p ), heal( s ), proc( 0 ), rng( 0 )
    {
      proc = p -> get_proc( "blazing_power" );
      rng  = p -> get_rng ( "blazing_power" );
      cd = p -> get_cooldown( "blazing_power_callback" );
      cd -> duration = timespan_t::from_seconds( 45.0 );
    }

    virtual void trigger( action_t* a, void* /* call_data */ )
    {
      if ( ( a -> aoe != 0 ) ||
           a -> proc     ||
           a -> dual     ||
           ! a -> harmful   )
        return;

      if ( cd -> down() )
        return;

      if ( rng -> roll( 0.10 ) )
      {
        heal -> target =  heal -> find_lowest_player();
        heal -> execute();
        proc -> occur();
        cd -> start();
      }
    }
  };

  // FIXME: Observe if it procs of non-direct healing spells
  p -> callbacks.register_heal_callback( RESULT_ALL_MASK, new blazing_power_callback_t( p, new blazing_power_heal_t( p, item -> heroic() ) )  );
}

// register_windward_heart ==================================================

static void register_windward_heart( item_t* item )
{
  maintenance_check( 410 );

  player_t* p = item -> player;

  item -> unique = true;

  struct windward_heart_heal_t : public heal_t
  {
    windward_heart_heal_t( player_t* p, bool heroic, bool lfr ) :
      heal_t( "windward", p, ( p -> dbc.spell( heroic ? 109825 : lfr ? 109822 : 108000 ) ) )
    {
      trigger_gcd = timespan_t::zero();
      background  = true;
      may_miss    = false;
      may_crit    = true;
      callbacks   = false;
      init();
    }
  };

  struct windward_heart_callback_t : public action_callback_t
  {
    heal_t* heal;
    cooldown_t* cd;
    proc_t* proc;
    rng_t* rng;

    windward_heart_callback_t( player_t* p, heal_t* s ) :
      action_callback_t( p ), heal( s ), proc( 0 ), rng( 0 )
    {
      proc = p -> get_proc( "windward_heart" );
      rng  = p -> get_rng ( "windward_heart" );
      cd = p -> get_cooldown( "windward_heart_callback" );
      cd -> duration = timespan_t::from_seconds( 20.0 );
    }

    virtual void trigger( action_t* a, void* /* call_data */ )
    {
      if ( ( a -> aoe != 0 ) ||
           a -> proc     ||
           a -> dual     ||
           ! a -> harmful   )
        return;

      if ( cd -> down() )
        return;

      if ( rng -> roll( 0.10 ) )
      {
        heal -> target = heal -> find_lowest_player();
        heal -> execute();
        proc -> occur();
        cd -> start();
      }
    }
  };

  p -> callbacks.register_heal_callback( RESULT_CRIT_MASK, new windward_heart_callback_t( p, new windward_heart_heal_t( p, item -> heroic(), item -> lfr() ) )  );
}

// register_symbiotic_worm ==================================================

static void register_symbiotic_worm( item_t* item )
{
  maintenance_check( 372 );

  player_t* p = item -> player;

  item -> unique = true;

  special_effect_t data;
  data.name_str    = "symbiotic_worm";
  data.stat        = STAT_MASTERY_RATING;
  data.stat_amount = item -> heroic() ? 1089 : 963;
  data.duration    = timespan_t::from_seconds( 10 );
  data.cooldown    = timespan_t::from_seconds( 30 );

  struct symbiotic_worm_callback_t : public stat_buff_proc_t
  {
    symbiotic_worm_callback_t( item_t& i, const special_effect_t& data ) :
      stat_buff_proc_t( i.player, data )
    { }

    virtual void trigger( action_t* a, void* call_data )
    {
      if ( listener -> health_percentage() < 35 )
        stat_buff_proc_t::trigger( a, call_data );
    }
  };

  symbiotic_worm_callback_t* cb = new symbiotic_worm_callback_t( *item, data );
  p -> callbacks.register_resource_loss_callback( RESOURCE_HEALTH, cb );
}

// register_indomitable_pride ================================================

static void register_indomitable_pride( item_t* item )
{
  maintenance_check( 410 );

  player_t* p = item -> player;

  item -> unique = true;

  struct indomitable_pride_callback_t : public action_callback_t
  {
    absorb_buff_t* buff;
    bool heroic, lfr;
    cooldown_t* cd;
    stats_t* stats;
    indomitable_pride_callback_t( player_t* p, bool h, bool l ) :
      action_callback_t( p ), heroic( h ), lfr( l ), cd ( 0 ), stats( 0 )
    {
      stats = listener -> get_stats( "indomitable_pride" );
      stats -> type = STATS_ABSORB;
      // Looks like there is no spell_id for the buff
      buff = absorb_buff_creator_t( p, "indomitable_pride" ).duration( timespan_t::from_seconds( 6.0 ) ).activated( false )
             .source( stats );
      cd = listener -> get_cooldown( "indomitable_pride" );
      cd -> duration = timespan_t::from_seconds( 60.0 );
    }

    virtual void trigger( action_t* /* a */, void*  call_data )
    {
      if ( cd -> remains() <= timespan_t::zero() && listener -> health_percentage() < 50 )
      {
        cd -> start();
        double amount = heroic ? 0.56 : lfr ? 0.43 : 0.50;
        if ( call_data )
          amount *= *( ( double * ) call_data );
        else
          assert( 0 );
        buff -> trigger( 1, amount );
        stats -> add_result( amount, amount, ABSORB, RESULT_HIT );
        stats -> add_execute( timespan_t::zero() );
      }
    }
  };

  action_callback_t* cb = new indomitable_pride_callback_t( p, item -> heroic(), item -> lfr() );
  p -> callbacks.register_resource_loss_callback( RESOURCE_HEALTH, cb );
}

// register_spidersilk_spindle ==============================================

static void register_spidersilk_spindle( item_t* item )
{
  maintenance_check( 391 );

  player_t* p = item -> player;

  item -> unique = true;

  struct spidersilk_spindle_callback_t : public action_callback_t
  {
    absorb_buff_t* buff;
    cooldown_t* cd;
    stats_t* stats;
    spidersilk_spindle_callback_t( player_t* p, bool h ) :
      action_callback_t( p ), cd ( 0 ), stats( 0 )
    {
      stats = listener -> get_stats( "loom_of_fate" );
      stats -> type = STATS_ABSORB;
      buff = absorb_buff_creator_t( p, "loom_of_fate" ).spell( p -> find_spell( h ? 97129 : 96945 ) ).activated( false )
             .source( stats );
      cd = listener -> get_cooldown( "spidersilk_spindle" );
      cd -> duration = timespan_t::from_seconds( 60.0 );
    }

    virtual void trigger( action_t* /* a */, void* /* call_data */ )
    {
      if ( cd -> remains() <= timespan_t::zero() && listener -> health_percentage() < 35 )
      {
        cd -> start();
        double amount = buff -> data().effectN( 1 ).base_value();
        buff -> trigger( 1, amount );
        stats -> add_result( amount, amount, ABSORB, RESULT_HIT );
        stats -> add_execute( timespan_t::zero() );
      }
    }
  };

  action_callback_t* cb = new spidersilk_spindle_callback_t( p, item -> heroic() );
  p -> callbacks.register_resource_loss_callback( RESOURCE_HEALTH, cb );
}

// register_bonelink_fetish =================================================

static void register_bonelink_fetish( item_t* item )
{
  maintenance_check( 410 );

  player_t* p   = item -> player;
  bool heroic   = item -> heroic();
  bool lfr      = item -> lfr();

  uint32_t spell_id = heroic ? 109755 : lfr ? 109753 : 107998;

  struct bonelink_fetish_callback_t : public action_callback_t
  {
    double chance;
    attack_t* attack;
    cooldown_t* cooldown;
    rng_t* rng;

    struct whirling_maw_t : public attack_t
    {
      whirling_maw_t( player_t* p, uint32_t spell_id ) :
        attack_t( "bonelink_fetish", p, ( p -> dbc.spell( spell_id ) ) )
      {
        trigger_gcd = timespan_t::zero();
        background = true;
        may_miss = false;
        may_glance = false;
        may_crit = true;
        proc = true;
        aoe = -1;
        direct_power_mod = data().extra_coeff();
        init();
      }
    };

    bonelink_fetish_callback_t( player_t* p, uint32_t id ) :
      action_callback_t( p ), chance( p -> dbc.spell( id ) -> proc_chance() )
    {
      attack = new whirling_maw_t( p, p -> dbc.spell( id ) -> effectN( 1 ).trigger_spell_id() );

      cooldown = p -> get_cooldown( "bonelink_fetish" );
      cooldown -> duration = timespan_t::from_seconds( 25.0 ); // 25 second ICD

      rng = p -> get_rng ( "bonelink_fetish" );
    }

    virtual void trigger( action_t* a, void* /* call_data */ )
    {
      if ( a -> proc )
        return;

      if ( cooldown -> down() )
        return;

      if ( rng -> roll( chance ) )
      {
        attack -> execute();
        cooldown -> start();
      }
    }
  };

  p -> callbacks.register_attack_callback( RESULT_HIT_MASK, new bonelink_fetish_callback_t( p, spell_id ) );
}

// register_fury_of_the_beast ===============================================

static void register_fury_of_the_beast( item_t* item )
{
  maintenance_check( 416 );

  player_t* p = item -> player;

  item -> unique = true;

  struct fury_of_the_beast_callback_t : public action_callback_t
  {
    buff_t* fury_of_the_beast;
    stat_buff_t* fury_of_the_beast_stack;

    // Event
    struct fury_of_the_beast_event_t : public event_t
    {
      buff_t* buff;
      buff_t* buff_stack;

      fury_of_the_beast_event_t ( player_t* player, buff_t* b, buff_t* q ) :
        event_t( player, "fury_of_the_beast" ), buff( b ), buff_stack( q )
      {
        sim.add_event( this, timespan_t::from_seconds( 1.0 ) );
      }

      virtual void execute()
      {
        if ( buff -> check() && buff_stack -> check() < buff_stack -> max_stack() )
        {
          buff_stack -> buff_duration = buff -> remains(); // hack instead of overriding fury_of_the_beast::expire()
          buff_stack -> trigger();
          new ( sim ) fury_of_the_beast_event_t( player, buff, buff_stack );
        }
      }
    };

    fury_of_the_beast_callback_t( player_t* p, bool h, bool lfr ) :
      action_callback_t( p )
    {
      double amount = h ? 120 : lfr ? 95 : 107; // Amount saved in the stat buff

      fury_of_the_beast = buff_creator_t( p, "fury_of_the_beast" ).spell( p -> find_spell( h ? 109864 : lfr ? 109861 : 108011 ) )
                          .chance( 0.15 ).cd( timespan_t::from_seconds( 55.0 ) );// FIXME: Confirm ICD

      fury_of_the_beast_stack  = stat_buff_creator_t( p, "fury_of_the_beast_stack" )
                                 .spell( p -> find_spell( h ? 109863 : lfr ? 109860 : 108016 ) )
                                 .activated( false )
                                 .add_stat( STAT_AGILITY, amount );
    }

    virtual void trigger( action_t* a, void* /* call_data */ )
    {
      if ( ! a -> weapon ) return;
      if ( a -> proc ) return;

      if ( fury_of_the_beast -> cooldown -> down() ) return;

      if ( fury_of_the_beast -> trigger() )
      {
        // FIXME: check if the stacking buff ticks at 0s or 1s
        new (  *listener -> sim ) fury_of_the_beast_event_t( listener, fury_of_the_beast, fury_of_the_beast_stack );
      }
    }
  };

  p -> callbacks.register_attack_callback( RESULT_HIT_MASK, new fury_of_the_beast_callback_t( p, item -> heroic(), item -> lfr() ) );
}

// register_gurthalak =======================================================

static void register_gurthalak( item_t* item )
{
  maintenance_check( 416 );

  player_t* p   = item -> player;
  bool heroic   = item -> heroic();
  bool lfr      = item -> lfr();
  int  slot     = item -> slot;

  uint32_t tick_damage = heroic ? 12591 : lfr ? 9881 : 11155;

  uint32_t proc_spell_id = heroic ? 109839 : lfr ? 109841 : 107810;

  struct gurthalak_callback_t : public action_callback_t
  {
    double chance;
    spell_t* spell[10];
    dot_t* dot_gurth[10];
    rng_t* rng;
    int slot;

    // FIXME: This should be converted to a pet, which casts 3 Mind Flays,
    // of 3 ticks each, with the last being 2-3 ticks
    // Reia is working on it and it's proving difficult
    // So until that can be done, use 5 spells to act as 5 spawns
    // and simulate 8-9 ticks on one mind flay, with a dot refresh
    struct gurthalak_t : public spell_t
    {
      gurthalak_t( player_t* p, uint32_t tick_damage, const char* name ) :
        spell_t( name, p, ( p -> dbc.spell( 52586 ) ) )
      {
        trigger_gcd = timespan_t::zero();
        background = true;
        tick_may_crit = true;
        hasted_ticks = false;
        proc = true;
        num_ticks = 9; // Casts 3 mind flays, resulting in 8-9 ticks on average
        base_attack_power_multiplier = 1.0;
        base_spell_power_multiplier = 0;

        // Override stats so all 5 tentacles are merged into 1
        stats = p -> get_stats( "gurthalak_voice_of_the_deeps" );
        stats -> school = SCHOOL_SHADOW; // Fix for reporting

        // While this spell ID is the one used by all of the tentacles,
        // It doesn't have a coeff and each version has static damage
        tick_power_mod = 0;
        base_td = tick_damage;

        // Change to DOT_REFRESH in-case we somehow RNG to all holy hell and get 6 up at once
        dot_behavior = DOT_REFRESH;

        init();
      }

      virtual void execute()
      {
        // Casts either 8 or 9 ticks, roughly equal chance for both
        num_ticks = sim -> roll( 0.5 ) ? 9 : 8;

        spell_t::execute();
      }
    };

    gurthalak_callback_t( player_t* p, uint32_t tick_damage, uint32_t proc_spell_id, int slot ) :
      action_callback_t( p ), chance( p -> dbc.spell( proc_spell_id ) -> proc_chance() ),
      slot( slot )
    {
      // Init different spells/dots to act like multiple tentacles up at once
      for ( int i=0; i <= 9; i++ )
      {
        std::string spell_name = "gurthalak_voice_of_the_deeps" + util::to_string( i );
        spell[ i ] = new gurthalak_t( p, tick_damage, spell_name.c_str()  );
        dot_gurth[ i ] = p -> target -> get_dot( spell_name, p );
      }

      rng = p -> get_rng ( "gurthalak" );
    }

    virtual void trigger( action_t* a, void* /* call_data */ )
    {
      if ( a -> proc )
        return;

      // Ensure the action came from the weapon slot that the item is in, unless the action ignores slot
      if ( ! a -> proc_ignores_slot )
      {
        if ( ! a -> weapon ) return;
        if ( a -> weapon -> slot != slot ) return;
      }

      if ( rng -> roll( chance ) )
      {
        // Try and find a non-ticking dot slot to use, if all are taken, use the one that's closest to expiring
        // This is needed for when we're DWing gurth
        int dot_slot = 0;
        for ( int i=0; i <= 9; i++ )
        {
          if ( ! dot_gurth[ i ] -> ticking )
          {
            dot_slot = i;
            break;
          }
          else
          {
            if ( dot_gurth[ dot_slot ] -> remains() < dot_gurth[ dot_slot ] -> remains() )
              dot_slot = i;
          }
        }

        spell[ dot_slot ] -> execute();
      }
    }
  };

  p -> callbacks.register_attack_callback( RESULT_HIT_MASK, new gurthalak_callback_t( p, tick_damage, proc_spell_id, slot ) );
}

// register_nokaled =========================================================

static void register_nokaled( item_t* item )
{
  maintenance_check( 416 );

  player_t* p   = item -> player;
  bool heroic   = item -> heroic();
  bool lfr      = item -> lfr();
  int slot      = item -> slot;
                                      //Fire  Frost   Shadow
  static uint32_t    lfr_spells[] = { 109871, 109869, 109867 };
  static uint32_t normal_spells[] = { 107785, 107789, 107787 };
  static uint32_t heroic_spells[] = { 109872, 109870, 109868 };

  uint32_t* spell_ids = heroic ? heroic_spells : lfr ? lfr_spells : normal_spells;

  uint32_t proc_spell_id = heroic ? 109873 : lfr ? 109866 : 107786;

  struct nokaled_callback_t : public action_callback_t
  {
    double chance;
    spell_t* spells[3];
    rng_t* rng;
    int slot;

    // FIXME: Verify if spells can miss
    struct nokaled_fire_t : public spell_t
    {
      nokaled_fire_t( player_t* p, uint32_t spell_id ) :
        spell_t( "nokaled_fireblast", p, ( p -> dbc.spell( spell_id ) ) )
      {
        trigger_gcd = timespan_t::zero();
        background = true;
        may_miss = false;
        may_crit = true;
        proc = true;
        init();
      }
    };

    struct nokaled_frost_t : public spell_t
    {
      nokaled_frost_t( player_t* p, uint32_t spell_id ) :
        spell_t( "nokaled_iceblast", p, ( p -> dbc.spell( spell_id ) ) )
      {
        trigger_gcd = timespan_t::zero();
        background = true;
        may_miss = false;
        may_crit = true;
        proc = true;
        init();
      }
    };

    struct nokaled_shadow_t : public spell_t
    {
      nokaled_shadow_t( player_t* p, uint32_t spell_id ) :
        spell_t( "nokaled_shadowblast", p, ( p -> dbc.spell( spell_id ) ) )
      {
        trigger_gcd = timespan_t::zero();
        background = true;
        may_miss = false;
        may_crit = true;
        proc = true;
        init();
      }
    };

    nokaled_callback_t( player_t* p, uint32_t ids[], uint32_t proc_spell_id, int slot ) :
      action_callback_t( p ), chance( p -> dbc.spell( proc_spell_id ) -> proc_chance() ),
      slot( slot )
    {
      spells[ 0 ] = new   nokaled_fire_t( p, ids[ 0 ] );
      spells[ 1 ] = new  nokaled_frost_t( p, ids[ 1 ] );
      spells[ 2 ] = new nokaled_shadow_t( p, ids[ 2 ] );

      rng = p -> get_rng ( "nokaled" );
    }

    virtual void trigger( action_t* a, void* /* call_data */ )
    {
      if ( a -> proc )
        return;

      // Only attacks from the slot that have No'Kaled can trigger it
      // unless the action ignores slot
      if ( ! a -> proc_ignores_slot )
      {
        if ( ! a -> weapon )
          return;

        if ( a -> weapon -> slot != slot )
          return;
      }

      if ( rng -> roll( chance ) )
      {
        int r = ( int ) ( rng -> range( 0.0, 2.999 ) );
        assert( r >= 0 && r <= 3 );
        spells[ r ] -> execute();
      }
    }
  };

  p -> callbacks.register_attack_callback( RESULT_HIT_MASK, new nokaled_callback_t( p, spell_ids, proc_spell_id, slot ) );
}

// register_rathrak =========================================================

static void register_rathrak( item_t* item )
{
  maintenance_check( 416 );

  player_t* p = item -> player;
  bool heroic = item -> heroic();
  bool lfr    = item -> lfr();
  uint32_t trigger_spell_id = heroic ? 109854 : lfr ? 109851 : 107831;

  struct rathrak_poison_t : public spell_t
  {
    rathrak_poison_t( player_t* p, uint32_t spell_id ) :
      spell_t( "rathrak", p, ( p -> dbc.spell( spell_id ) ) )
    {
      trigger_gcd = timespan_t::zero();
      background = true;
      may_miss = false; // FIXME: Verify this
      tick_may_crit = true;
      proc = true;
      hasted_ticks = false;
      init();
      cooldown -> duration = timespan_t::from_seconds( 17.0 ); // FIXME: Verify this. Got 17.188sec after 545 procs.
    }
    // Testing shows this is affected by CoE and Spell Dmg modifiers (Shadow Power, etc)
  };

  struct rathrak_callback_t : public action_callback_t
  {
    spell_t* spell;
    rng_t* rng;

    rathrak_callback_t( player_t* p, spell_t* s ) :
      action_callback_t( p ), spell( s )
    {
      rng  = p -> get_rng ( "rathrak" );
    }

    virtual void trigger( action_t* /* a */, void* /* call_data */ )
    {
      if ( ( spell -> cooldown -> remains() <= timespan_t::zero() ) && rng -> roll( 0.15 ) )
      {
        spell -> execute();
      }
    }
  };

  p -> callbacks.register_harmful_spell_callback( RESULT_HIT_MASK, new rathrak_callback_t( p, new rathrak_poison_t( p, trigger_spell_id ) ) );
}

// register_souldrinker =====================================================

static void register_souldrinker( item_t* item )
{
  maintenance_check( 416 );

  player_t* p = item -> player;
  bool heroic = item -> heroic();
  bool lfr    = item -> lfr();
  int slot    = item -> slot;

  struct souldrinker_spell_t : public spell_t
  {
    souldrinker_spell_t( player_t* p, bool h, bool lfr ) :
      spell_t( "souldrinker", p, ( p -> dbc.spell( h ? 109831 : lfr ? 109828 : 108022 ) ) )
    {
      trigger_gcd = timespan_t::zero();
      background = true;
      may_miss = false;
      may_crit = false;
      proc = true;
      init();
    }

    virtual void execute()
    {
      base_dd_min = base_dd_max = data().effectN( 1 ).percent() / 10.0 * player -> resources.max[ RESOURCE_HEALTH ];
      spell_t::execute();
    }

    virtual void player_buff() { }

    virtual double total_dd_multiplier() { return 1.0; }
  };

  struct souldrinker_callback_t : public action_callback_t
  {
    spell_t* spell;
    rng_t* rng;
    int slot;

    souldrinker_callback_t( player_t* p, spell_t* s, int slot ) :
      action_callback_t( p ), spell( s ), slot( slot )
    {
      rng  = p -> get_rng ( "souldrinker" );
    }

    virtual void trigger( action_t* a, void* /* call_data */ )
    {
      // Only the slot the weapon is in can trigger it, e.g. a Souldrinker in the MH can't proc from OH attacks
      // unless the action ignores slot
      // http://elitistjerks.com/f72/t125291-frost_dps_winter_discontent_4_3_a/p12/#post2055642
      if ( ! a -> proc_ignores_slot )
      {
        if ( ! a -> weapon ) return;
        if ( a -> weapon -> slot != slot ) return;
      }

      // No ICD
      if ( rng -> roll( 0.15 ) )
      {
        spell -> execute();
      }
    }
  };

  p -> callbacks.register_attack_callback( RESULT_HIT_MASK, new souldrinker_callback_t( p, new souldrinker_spell_t( p, heroic, lfr ), slot ) );
}

// register_titahk ==========================================================

static void register_titahk( item_t* item )
{
  maintenance_check( 416 );

  player_t* p = item -> player;
  bool heroic = item -> heroic();
  bool lfr    = item -> lfr();

  uint32_t spell_id = heroic ? 109846 : lfr ? 109843 : 107805;
  uint32_t buff_id = p -> dbc.spell( spell_id ) -> effectN( 1 ).trigger_spell_id();

  const spell_data_t* spell = p -> dbc.spell( spell_id );
  const spell_data_t* buff  = p -> dbc.spell( buff_id );

  struct titahk_callback_t : public action_callback_t
  {
    double proc_chance;
    rng_t* rng;
    buff_t* buff_self;
    buff_t* buff_radius; // This buff should be in 20 yards radius but it is contained only on the player for simulation.

    titahk_callback_t( player_t* p, const spell_data_t* spell, const spell_data_t* buff ) :
      action_callback_t( p ),
      proc_chance( spell -> proc_chance() ),
      rng( p -> get_rng( "titahk" ) )
    {
      timespan_t duration = buff -> duration();

      buff_self   = stat_buff_creator_t( p, "titahk_self" ).duration( duration )
                    .cd( timespan_t::from_seconds( 45.0 ) ) // FIXME: Confirm ICD
                    .add_stat( STAT_HASTE_RATING, buff -> effectN( 1 ).base_value() );

      buff_radius = stat_buff_creator_t( p, "titahk_aoe" ).duration( duration )
                    .cd( timespan_t::from_seconds( 45.0 ) )// FIXME: Confirm ICD
                    .add_stat( STAT_HASTE_RATING, buff -> effectN( 2 ).base_value() ); // FIXME: Apply aoe buff to other players
    }

    virtual void trigger( action_t* /* a */, void* /* call_data */ )
    {
      // FIXME: Does this have an ICD?
      if ( ( buff_self -> cooldown -> remains() <= timespan_t::zero() ) && rng -> roll( proc_chance ) )
      {
        buff_self -> trigger();
        buff_radius -> trigger();
      }
    }
  };

  p -> callbacks.register_spell_callback( SCHOOL_SPELL_MASK, new titahk_callback_t( p, spell, buff ) );
}

// register_zen_alchemist_stone =============================================

static void register_zen_alchemist_stone( item_t* item )
{
  struct zen_alchemist_stone_callback : public proc_callback_t<action_state_t>
  {
    stat_buff_t* buff_str;
    stat_buff_t* buff_agi;
    stat_buff_t* buff_int;

    zen_alchemist_stone_callback( item_t& i, const special_effect_t& data ) :
      proc_callback_t<action_state_t>( i.player, data )
    {
      const spell_data_t* spell = listener -> find_spell( 105574 );

      struct common_buff_creator : public stat_buff_creator_t
      {
        common_buff_creator( player_t* p, const std::string& n, const spell_data_t* spell ) :
          stat_buff_creator_t ( p, "zen_alchemist_stone_" + n, spell  )
        {
          duration( p -> find_spell( 60229 ) -> duration() );
          chance( 1.0 );
          activated( false );
        }
      };

      const random_prop_data_t& budget = listener -> dbc.random_property( i.ilevel );
      double value = budget.p_rare[ 0 ] * spell -> effectN( 1 ).m_average();

      buff_str = common_buff_creator( listener, "str", spell )
                 .add_stat( STAT_STRENGTH, value );
      buff_agi = common_buff_creator( listener, "agi", spell )
                 .add_stat( STAT_AGILITY, value );
      buff_int = common_buff_creator( listener, "int", spell )
                 .add_stat( STAT_INTELLECT, value );
    }

    void execute( action_t* a, action_state_t* /* state */ )
    {
      player_t* p = a -> player;

      if ( p -> strength() > p -> agility() )
      {
        if ( p -> strength() > p -> intellect() )
          buff_str -> trigger();
        else
          buff_int -> trigger();
      }
      else if ( p -> agility() > p -> intellect() )
        buff_agi -> trigger();
      else
        buff_int -> trigger();
    }
  };

  maintenance_check( 450 );

  item -> unique = true;

  special_effect_t data;
  data.name_str    = "zen_alchemist_stone";
  data.cooldown    = timespan_t::from_seconds( 55.0 );
  data.proc_chance = 1;

  zen_alchemist_stone_callback* cb = new zen_alchemist_stone_callback( *item, data );
  item -> player -> callbacks.register_direct_damage_callback( SCHOOL_ALL_MASK, cb );
  item -> player -> callbacks.register_tick_damage_callback( SCHOOL_ALL_MASK, cb );
  item -> player -> callbacks.register_direct_heal_callback( RESULT_ALL_MASK, cb );
}

// register_bad_juju ==========================================================

static void register_bad_juju( item_t* item )
{
  // TODO: Gnomes of Doom
  struct bad_juju_callback_t : public stat_buff_proc_t
  {
    std::vector<pet_t*> gnomes;

    bad_juju_callback_t( item_t& i, const special_effect_t& data ) :
      stat_buff_proc_t( i.player, data )
    {
      const spell_data_t* spell = listener -> find_spell( 138939 );

      gnomes.resize( static_cast< int >( spell -> effectN( 1 ).base_value() ) );
    }

    virtual void execute( action_t* action, action_state_t* state )
    {
      stat_buff_proc_t::execute( action, state );
      for ( size_t i = 0; i < gnomes.size(); i++ )
        if ( gnomes[ i ] ) gnomes[ i ] -> summon( buff -> buff_duration );
    }
  };

  maintenance_check( 502 );

  item -> unique = true;

  const spell_data_t* spell = item -> player -> find_spell( 138938 );
  const random_prop_data_t& budget = item -> player -> dbc.random_property( item -> ilevel );

  special_effect_t data;
  data.name_str    = "juju_madness";
  data.ppm         = -0.5; // Real PPM
  data.stat        = STAT_AGILITY;
  data.stat_amount = budget.p_rare[ 0 ] * spell -> effectN( 1 ).m_average();
  data.duration    = spell -> duration();

  item -> player -> callbacks.register_direct_damage_callback( SCHOOL_ALL_MASK, new bad_juju_callback_t( *item, data ) );
}

// register_rune_of_reorigination =============================================

// TODO: How does this interact with rating multipliers
static void register_rune_of_reorigination( item_t* item )
{
  struct rune_of_reorigination_callback_t : public proc_callback_t<action_state_t>
  {
    enum
    {
      BUFF_CRIT = 0,
      BUFF_HASTE,
      BUFF_MASTERY
    };

    stat_buff_t* buff;

    rune_of_reorigination_callback_t( item_t& i, const special_effect_t& data ) :
      proc_callback_t<action_state_t>( i.player, data )
    {
      buff = stat_buff_creator_t( listener, proc_data.name_str )
             .activated( false )
             .duration( proc_data.duration )
             .add_stat( STAT_CRIT_RATING, 0 )
             .add_stat( STAT_HASTE_RATING, 0 )
             .add_stat( STAT_MASTERY_RATING, 0 );
    }

    virtual void execute( action_t* action, action_state_t* /* state */ )
    {
      player_t* p = action -> player;
      double chr = p -> stats.haste_rating;
      double ccr = p -> stats.crit_rating;
      double cmr = p -> stats.mastery_rating;
      if ( p -> sim -> debug )
        p -> sim -> output( "%s rune_of_reorigination procs crit=%.0f haste=%.0f mastery=%.0f",
                            p -> name(), ccr, chr, cmr );

      if ( ccr >= chr )
      {
        // I choose you, crit
        if ( ccr >= cmr )
        {
          buff -> stats[ BUFF_CRIT    ].amount = chr + cmr;
          buff -> stats[ BUFF_HASTE   ].amount = -chr;
          buff -> stats[ BUFF_MASTERY ].amount = -cmr;
        }
        // I choose you, mastery
        else
        {
          buff -> stats[ BUFF_CRIT    ].amount = -ccr;
          buff -> stats[ BUFF_HASTE   ].amount = -chr;
          buff -> stats[ BUFF_MASTERY ].amount = ccr + chr;
        }
      }
      // I choose you, haste
      else if ( chr >= cmr )
      {
        buff -> stats[ BUFF_CRIT    ].amount = -ccr;
        buff -> stats[ BUFF_HASTE   ].amount = ccr + cmr;
        buff -> stats[ BUFF_MASTERY ].amount = -cmr;
      }
      // I choose you, mastery
      else
      {
        buff -> stats[ BUFF_CRIT    ].amount = -ccr;
        buff -> stats[ BUFF_HASTE   ].amount = -chr;
        buff -> stats[ BUFF_MASTERY ].amount = ccr + chr;
      }

      buff -> trigger();
    }
  };

  maintenance_check( 502 );

  item -> unique = true;

  const spell_data_t* spell = item -> player -> find_spell( 139120 );

  special_effect_t data;
  data.name_str    = "rune_of_reorigination";
  data.ppm         = -0.46; // Real PPM
  data.cooldown    = timespan_t::from_seconds( 22 );
  data.duration    = spell -> duration();

  item -> player -> callbacks.register_direct_damage_callback( SCHOOL_ALL_MASK, new rune_of_reorigination_callback_t( *item, data ) );
}

// register_spark_of_zandalar =================================================

static void register_spark_of_zandalar( item_t* item )
{
  maintenance_check( 502 );

  item -> unique = true;

  const spell_data_t* spell = item -> player -> find_spell( 138958 );

  special_effect_t data;
  data.name_str    = "spark_of_zandalar";
  data.ppm         = -5.0; // Real PPM
  data.duration    = spell -> duration();
  data.max_stacks  = spell -> max_stacks();

  struct spark_of_zandalar_callback_t : public proc_callback_t<action_state_t>
  {
    buff_t*      sparks;
    stat_buff_t* buff;

    spark_of_zandalar_callback_t( item_t& i, const special_effect_t& data ) :
      proc_callback_t<action_state_t>( i.player, data )
    {
      sparks = buff_creator_t( listener, proc_data.name_str )
               .activated( false )
               .duration( proc_data.duration )
               .max_stack( proc_data.max_stacks );

      const spell_data_t* spell = listener -> find_spell( 138960 );
      const random_prop_data_t& budget = listener -> dbc.random_property( i.ilevel );

      buff = stat_buff_creator_t( listener, "zandalari_warrior" )
             .duration( spell -> duration() )
             .add_stat( STAT_STRENGTH, budget.p_rare[ 0 ] * spell -> effectN( 2 ).m_average() );
    }

    void execute( action_t* /* action */, action_state_t* /* state */ )
    {
      sparks -> trigger();

      if ( sparks -> stack() == sparks -> max_stack() )
      {
        sparks -> expire();
        buff   -> trigger();
      }
    }
  };

  item -> player -> callbacks.register_direct_damage_callback( SCHOOL_ALL_MASK, new spark_of_zandalar_callback_t( *item, data ) );
};

// register_unnerring_vision_of_leishen =======================================

static void register_unerring_vision_of_leishen( item_t* item )
{
  struct perfect_aim_buff_t : public buff_t
  {
    perfect_aim_buff_t( player_t* p, const spell_data_t* s ) :
      buff_t( buff_creator_t( p, "perfect_aim", s ).activated( false ) )
    { }

    void execute( int stacks, double value, timespan_t duration )
    {
      if ( current_stack == 0 )
      {
        player -> current.spell_crit  += data().effectN( 1 ).percent();
        player -> current.attack_crit += data().effectN( 1 ).percent();
      }

      buff_t::execute( stacks, value, duration );
    }

    void expire_override()
    {
      buff_t::expire_override();

      player -> current.spell_crit  -= data().effectN( 1 ).percent();
      player -> current.attack_crit -= data().effectN( 1 ).percent();
    }
  };

  struct unerring_vision_of_leishen_callback_t : public proc_callback_t<action_state_t>
  {
    perfect_aim_buff_t* buff;

    unerring_vision_of_leishen_callback_t( item_t& i, const special_effect_t& data ) :
      proc_callback_t<action_state_t>( i.player, data )
    { buff = new perfect_aim_buff_t( listener, listener -> find_spell( 138963 ) ); }

    void execute( action_t* /* action */, action_state_t* /* state */ )
    { buff -> trigger(); }
  };

  maintenance_check( 502 );

  item -> unique = true;

  special_effect_t data;
  data.name_str    = "perfect_aim";
  data.ppm         = -0.5; // Real PPM

  unerring_vision_of_leishen_callback_t* cb = new unerring_vision_of_leishen_callback_t( *item, data );

  item -> player -> callbacks.register_spell_direct_damage_callback( SCHOOL_ALL_MASK, cb );
  item -> player -> callbacks.register_spell_tick_damage_callback( SCHOOL_ALL_MASK, cb );
};

// ==========================================================================
// unique_gear::init
// ==========================================================================

void unique_gear::init( player_t* p )
{
  if ( p -> is_pet() ) return;

  if ( p -> race == RACE_UNDEAD )
  {
    register_touch_of_the_grave( p );
  }

  for ( size_t i = 0; i < p -> items.size(); i++ )
  {
    item_t& item = p -> items[ i ];

    if ( item.equip.stat && item.equip.school )
    {
      register_stat_discharge_proc( item, item.equip );
    }
    else if ( item.equip.stat )
    {
      register_stat_proc( p, item.equip );
    }
    else if ( item.equip.cost_reduction && item.equip.school )
    {
      register_cost_reduction_proc( p, item.equip );
    }
    else if ( item.equip.school && item.equip.proc_chance && item.equip.chance_to_discharge )
    {
      register_chance_discharge_proc( item, item.equip );
    }
    else if ( item.equip.school )
    {
      register_discharge_proc( item, item.equip );
    }

    if ( ! strcmp( item.name(), "apparatus_of_khazgoroth"             ) ) register_apparatus_of_khazgoroth           ( &item );
    if ( ! strcmp( item.name(), "bonelink_fetish"                     ) ) register_bonelink_fetish                   ( &item );
    if ( ! strcmp( item.name(), "eye_of_blazing_power"                ) ) register_blazing_power                     ( &item );
    if ( ! strcmp( item.name(), "gurthalak_voice_of_the_deeps"        ) ) register_gurthalak                         ( &item );
    if ( ! strcmp( item.name(), "heart_of_ignacious"                  ) ) register_heart_of_ignacious                ( &item );
    if ( ! strcmp( item.name(), "indomitable_pride"                   ) ) register_indomitable_pride                 ( &item );
    if ( ! strcmp( item.name(), "kiril_fury_of_beasts"                ) ) register_fury_of_the_beast                 ( &item );
    if ( ! strcmp( item.name(), "matrix_restabilizer"                 ) ) register_matrix_restabilizer               ( &item );
    if ( ! strcmp( item.name(), "nokaled_the_elements_of_death"       ) ) register_nokaled                           ( &item );
    if ( ! strcmp( item.name(), "rathrak_the_poisonous_mind"          ) ) register_rathrak                           ( &item );
    if ( ! strcmp( item.name(), "shard_of_woe"                        ) ) register_shard_of_woe                      ( &item );
    if ( ! strcmp( item.name(), "souldrinker"                         ) ) register_souldrinker                       ( &item );
    if ( ! strcmp( item.name(), "spidersilk_spindle"                  ) ) register_spidersilk_spindle                ( &item );
    if ( ! strcmp( item.name(), "symbiotic_worm"                      ) ) register_symbiotic_worm                    ( &item );
    if ( ! strcmp( item.name(), "windward_heart"                      ) ) register_windward_heart                    ( &item );
    if ( ! strcmp( item.name(), "titahk_the_steps_of_time"            ) ) register_titahk                            ( &item );
    if ( ! strcmp( item.name(), "zen_alchemist_stone"                 ) ) register_zen_alchemist_stone               ( &item );
    if ( ! strcmp( item.name(), "bad_juju"                            ) ) register_bad_juju                          ( &item );
    if ( ! strcmp( item.name(), "rune_of_reorigination"               ) ) register_rune_of_reorigination             ( &item );
    if ( ! strcmp( item.name(), "spark_of_zandalar"                   ) ) register_spark_of_zandalar                 ( &item );
    if ( ! strcmp( item.name(), "unerring_vision_of_leishen"          ) ) register_unerring_vision_of_leishen        ( &item );
  }
}

// ==========================================================================
// unique_gear::register_stat_proc
// ==========================================================================

action_callback_t* unique_gear::register_stat_proc( player_t* player,
                                                    special_effect_t& effect )
{
  action_callback_t* cb = new stat_buff_proc_t( player, effect );

  if ( effect.trigger_type == PROC_DAMAGE || effect.trigger_type == PROC_DAMAGE_HEAL )
  {
    player -> callbacks.register_tick_damage_callback( effect.trigger_mask, cb );
    player -> callbacks.register_direct_damage_callback( effect.trigger_mask, cb );
  }
  if ( effect.trigger_type == PROC_HEAL || effect.trigger_type == PROC_DAMAGE_HEAL )
  {
    player -> callbacks.register_tick_heal_callback( effect.trigger_mask, cb );
    player -> callbacks.register_direct_heal_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_TICK_DAMAGE )
  {
    player -> callbacks.register_tick_damage_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_DIRECT_DAMAGE )
  {
    player -> callbacks.register_direct_damage_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_DIRECT_CRIT )
  {
    player -> callbacks.register_direct_crit_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_SPELL_TICK_DAMAGE )
  {
    player -> callbacks.register_spell_tick_damage_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_SPELL_DIRECT_DAMAGE )
  {
    player -> callbacks.register_spell_direct_damage_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_ATTACK )
  {
    player -> callbacks.register_attack_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_SPELL )
  {
    player -> callbacks.register_spell_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_TICK )
  {
    player -> callbacks.register_tick_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_HARMFUL_SPELL )
  {
    player -> callbacks.register_harmful_spell_callback( effect.trigger_mask, cb );
    player -> callbacks.register_tick_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_HEAL_SPELL )
  {
    player -> callbacks.register_heal_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_DAMAGE_HEAL_SPELL )
  {
    player -> callbacks.register_spell_callback( effect.trigger_mask, cb );
    player -> callbacks.register_heal_callback( effect.trigger_mask, cb );
  }

  return cb;
}

// ==========================================================================
// unique_gear::register_cost_reduction_proc
// ==========================================================================

action_callback_t* unique_gear::register_cost_reduction_proc( player_t* player,
                                                              special_effect_t& effect )
{
  action_callback_t* cb = new cost_reduction_buff_proc_t( player, effect );

  if ( effect.trigger_type == PROC_DAMAGE || effect.trigger_type == PROC_DAMAGE_HEAL )
  {
    player -> callbacks.register_tick_damage_callback( effect.trigger_mask, cb );
    player -> callbacks.register_direct_damage_callback( effect.trigger_mask, cb );
  }
  if ( effect.trigger_type == PROC_HEAL || effect.trigger_type == PROC_DAMAGE_HEAL )
  {
    player -> callbacks.register_tick_heal_callback( effect.trigger_mask, cb );
    player -> callbacks.register_direct_heal_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_TICK_DAMAGE )
  {
    player -> callbacks.register_tick_damage_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_DIRECT_DAMAGE )
  {
    player -> callbacks.register_direct_damage_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_DIRECT_CRIT )
  {
    player -> callbacks.register_direct_crit_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_SPELL_TICK_DAMAGE )
  {
    player -> callbacks.register_spell_tick_damage_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_SPELL_DIRECT_DAMAGE )
  {
    player -> callbacks.register_spell_direct_damage_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_TICK )
  {
    player -> callbacks.register_tick_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_ATTACK )
  {
    player -> callbacks.register_attack_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_SPELL )
  {
    player -> callbacks.register_spell_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_HARMFUL_SPELL )
  {
    player -> callbacks.register_harmful_spell_callback( effect.trigger_mask, cb );
    player -> callbacks.register_tick_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_HEAL_SPELL )
  {
    player -> callbacks.register_heal_callback( effect.trigger_mask, cb );
  }
  else if ( effect.trigger_type == PROC_DIRECT_HARMFUL_SPELL )
  {
    player -> callbacks.register_direct_harmful_spell_callback( effect.trigger_mask, cb );
  }

  return cb;
}

// ==========================================================================
// unique_gear::register_discharge_proc
// ==========================================================================

action_callback_t* unique_gear::register_discharge_proc( proc_e        type,
                                                         int64_t            mask,
                                                         const std::string& name,
                                                         player_t*          player,
                                                         int                max_stacks,
                                                         school_e      school,
                                                         double             amount,
                                                         double             scaling,
                                                         double             proc_chance,
                                                         timespan_t         cooldown,
                                                         int                aoe,
                                                         unsigned int       override_result_es_mask,
                                                         unsigned int       result_es_mask )
{
  special_effect_t se;
  se.trigger_type = type;
  se.name_str = name;
  se.max_stacks = max_stacks;
  se.school = school;
  se.discharge_amount = amount;
  se.discharge_scaling = scaling;
  se.proc_chance = proc_chance;
  se.cooldown = cooldown;
  se.aoe = aoe;
  se.override_result_es_mask = override_result_es_mask;
  se.result_es_mask = result_es_mask;

  action_callback_t* cb = new discharge_proc_callback_t( player, se );

  if ( type == PROC_DAMAGE || type == PROC_DAMAGE_HEAL )
  {
    player -> callbacks.register_tick_damage_callback( mask, cb );
    player -> callbacks.register_direct_damage_callback( mask, cb );
  }
  if ( type == PROC_HEAL || type == PROC_DAMAGE_HEAL )
  {
    player -> callbacks.register_tick_heal_callback( mask, cb );
    player -> callbacks.register_direct_heal_callback( mask, cb );
  }
  else if ( type == PROC_TICK_DAMAGE )
  {
    player -> callbacks.register_tick_damage_callback( mask, cb );
  }
  else if ( type == PROC_DIRECT_DAMAGE )
  {
    player -> callbacks.register_direct_damage_callback( mask, cb );
  }
  else if ( type == PROC_DIRECT_CRIT )
  {
    player -> callbacks.register_direct_crit_callback( mask, cb );
  }
  else if ( type == PROC_SPELL_TICK_DAMAGE )
  {
    player -> callbacks.register_spell_tick_damage_callback( mask, cb );
  }
  else if ( type == PROC_SPELL_DIRECT_DAMAGE )
  {
    player -> callbacks.register_spell_direct_damage_callback( mask, cb );
  }
  else if ( type == PROC_TICK )
  {
    player -> callbacks.register_tick_callback( mask, cb );
  }
  else if ( type == PROC_ATTACK )
  {
    player -> callbacks.register_attack_callback( mask, cb );
  }
  else if ( type == PROC_SPELL )
  {
    player -> callbacks.register_spell_callback( mask, cb );
  }
  else if ( type == PROC_SPELL_AND_TICK )
  {
    player -> callbacks.register_spell_callback( mask, cb );
    player -> callbacks.register_tick_callback( mask, cb );
  }
  else if ( type == PROC_HARMFUL_SPELL )
  {
    player -> callbacks.register_harmful_spell_callback( mask, cb );
  }
  else if ( type == PROC_HEAL_SPELL )
  {
    player -> callbacks.register_heal_callback( mask, cb );
  }

  return cb;
}

// ==========================================================================
// unique_gear::register_chance_discharge_proc
// ==========================================================================

action_callback_t* unique_gear::register_chance_discharge_proc( proc_e        type,
                                                                int64_t            mask,
                                                                const std::string& name,
                                                                player_t*          player,
                                                                int                max_stacks,
                                                                school_e      school,
                                                                double             amount,
                                                                double             scaling,
                                                                double             proc_chance,
                                                                timespan_t         cooldown,
                                                                int                aoe,
                                                                unsigned int       override_result_es_mask,
                                                                unsigned int       result_es_mask )
{
  special_effect_t se;
  se.trigger_type = type;
  se.name_str = name;
  se.max_stacks = max_stacks;
  se.school = school;
  se.discharge_amount = amount;
  se.discharge_scaling = scaling;
  se.proc_chance = proc_chance;
  se.cooldown = cooldown;
  se.aoe = aoe;
  se.override_result_es_mask = override_result_es_mask;
  se.result_es_mask = result_es_mask;

  action_callback_t* cb = new chance_discharge_proc_callback_t( player, se );

  if ( type == PROC_DAMAGE || type == PROC_DAMAGE_HEAL )
  {
    player -> callbacks.register_tick_damage_callback( mask, cb );
    player -> callbacks.register_direct_damage_callback( mask, cb );
  }
  if ( type == PROC_HEAL  || type == PROC_DAMAGE_HEAL )
  {
    player -> callbacks.register_tick_heal_callback( mask, cb );
    player -> callbacks.register_direct_heal_callback( mask, cb );
  }
  else if ( type == PROC_TICK_DAMAGE )
  {
    player -> callbacks.register_tick_damage_callback( mask, cb );
  }
  else if ( type == PROC_DIRECT_DAMAGE )
  {
    player -> callbacks.register_direct_damage_callback( mask, cb );
  }
  else if ( type == PROC_DIRECT_CRIT )
  {
    player -> callbacks.register_direct_crit_callback( mask, cb );
  }
  else if ( type == PROC_SPELL_TICK_DAMAGE )
  {
    player -> callbacks.register_spell_tick_damage_callback( mask, cb );
  }
  else if ( type == PROC_SPELL_DIRECT_DAMAGE )
  {
    player -> callbacks.register_spell_direct_damage_callback( mask, cb );
  }
  else if ( type == PROC_TICK )
  {
    player -> callbacks.register_tick_callback( mask, cb );
  }
  else if ( type == PROC_ATTACK )
  {
    player -> callbacks.register_attack_callback( mask, cb );
  }
  else if ( type == PROC_SPELL )
  {
    player -> callbacks.register_spell_callback( mask, cb );
  }
  else if ( type == PROC_SPELL_AND_TICK )
  {
    player -> callbacks.register_spell_callback( mask, cb );
    player -> callbacks.register_tick_callback( mask, cb );
  }
  else if ( type == PROC_HARMFUL_SPELL )
  {
    player -> callbacks.register_harmful_spell_callback( mask, cb );
  }
  else if ( type == PROC_HEAL_SPELL )
  {
    player -> callbacks.register_heal_callback( mask, cb );
  }

  return cb;
}

// ==========================================================================
// unique_gear::register_stat_discharge_proc
// ==========================================================================

action_callback_t* unique_gear::register_stat_discharge_proc( proc_e        type,
                                                              int64_t            mask,
                                                              const std::string& name,
                                                              player_t*          player,
                                                              int                max_stacks,
                                                              stat_e        stat,
                                                              double             stat_amount,
                                                              school_e      school,
                                                              double             min_dmg,
                                                              double             max_dmg,
                                                              double             proc_chance,
                                                              timespan_t         duration,
                                                              timespan_t         cooldown,
                                                              int                aoe,
                                                              unsigned int       override_result_es_mask,
                                                              unsigned int       result_es_mask )
{
  action_callback_t* cb = new stat_discharge_proc_callback_t( name, player, stat, max_stacks, stat_amount, school, min_dmg, max_dmg, proc_chance,
                                                              duration, cooldown, aoe, type == PROC_NONE, override_result_es_mask, result_es_mask );

  if ( type == PROC_DAMAGE || type == PROC_DAMAGE_HEAL )
  {
    player -> callbacks.register_tick_damage_callback( mask, cb );
    player -> callbacks.register_direct_damage_callback( mask, cb );
  }
  if ( type == PROC_HEAL  || type == PROC_DAMAGE_HEAL )
  {
    player -> callbacks.register_tick_heal_callback( mask, cb );
    player -> callbacks.register_direct_heal_callback( mask, cb );
  }
  else if ( type == PROC_TICK_DAMAGE )
  {
    player -> callbacks.register_tick_damage_callback( mask, cb );
  }
  else if ( type == PROC_DIRECT_DAMAGE )
  {
    player -> callbacks.register_direct_damage_callback( mask, cb );
  }
  else if ( type == PROC_DIRECT_CRIT )
  {
    player -> callbacks.register_direct_crit_callback( mask, cb );
  }
  else if ( type == PROC_SPELL_TICK_DAMAGE )
  {
    player -> callbacks.register_spell_tick_damage_callback( mask, cb );
  }
  else if ( type == PROC_SPELL_DIRECT_DAMAGE )
  {
    player -> callbacks.register_spell_direct_damage_callback( mask, cb );
  }
  else if ( type == PROC_TICK )
  {
    player -> callbacks.register_tick_callback( mask, cb );
  }
  else if ( type == PROC_ATTACK )
  {
    player -> callbacks.register_attack_callback( mask, cb );
  }
  else if ( type == PROC_SPELL )
  {
    player -> callbacks.register_spell_callback( mask, cb );
  }
  else if ( type == PROC_HARMFUL_SPELL )
  {
    player -> callbacks.register_harmful_spell_callback( mask, cb );
  }
  else if ( type == PROC_HEAL_SPELL )
  {
    player -> callbacks.register_heal_callback( mask, cb );
  }

  return cb;
}

// ==========================================================================
// unique_gear::register_discharge_proc
// ==========================================================================

action_callback_t* unique_gear::register_discharge_proc( item_t& i,
                                                         special_effect_t& e )
{
  const std::string& name = e.name_str.empty() ? i.name() : e.name_str;

  return register_discharge_proc( e.trigger_type, e.trigger_mask, name, i.player,
                                  e.max_stacks, e.school, e.discharge_amount, e.discharge_scaling,
                                  e.proc_chance, e.cooldown, e.aoe, e.override_result_es_mask, e.result_es_mask );
}

// ==========================================================================
// unique_gear::register_chance_discharge_proc
// ==========================================================================

action_callback_t* unique_gear::register_chance_discharge_proc( item_t& i,
                                                                special_effect_t& e )
{
  const std::string& name = e.name_str.empty() ? i.name() : e.name_str;

  return register_chance_discharge_proc( e.trigger_type, e.trigger_mask, name, i.player,
                                         e.max_stacks, e.school, e.discharge_amount, e.discharge_scaling,
                                         e.proc_chance, e.cooldown, e.aoe, e.override_result_es_mask, e.result_es_mask );
}

// ==========================================================================
// unique_gear::register_stat_discharge_proc
// ==========================================================================

action_callback_t* unique_gear::register_stat_discharge_proc( item_t& i,
                                                              special_effect_t& e )
{
  const std::string& name = e.name_str.empty() ? i.name() : e.name_str;

  return register_stat_discharge_proc( e.trigger_type, e.trigger_mask, name, i.player,
                                       e.max_stacks, e.stat, e.stat_amount,
                                       e.school, e.discharge_amount, e.discharge_scaling,
                                       e.proc_chance, e.duration, e.cooldown, e.aoe, e.override_result_es_mask, e.result_es_mask );
}

// ==========================================================================
// unique_gear::get_equip_encoding
// ==========================================================================

bool unique_gear::get_equip_encoding( std::string&       encoding,
                                      const std::string& name,
                                      bool         heroic,
                                      bool         lfr,
                                      bool         /* ptr */,
                                      const std::string& id )
{
  std::string e;
  unsigned item_id = strtol( id.c_str(), NULL, 10 );

  // Stat Procs
  if      ( name == "abyssal_rune"                        ) e = "OnHarmfulSpellCast_590SP_25%_10Dur_45Cd";
  else if ( name == "anhuurs_hymnal"                      ) e = ( heroic ? "OnSpellCast_1710SP_10%_10Dur_50Cd" : "OnSpellCast_1512SP_10%_10Dur_50Cd" );
  else if ( name == "ashen_band_of_endless_destruction"   ) e = "OnSpellHit_285SP_10%_10Dur_60Cd";
  else if ( name == "ashen_band_of_unmatched_destruction" ) e = "OnSpellHit_285SP_10%_10Dur_60Cd";
  else if ( name == "ashen_band_of_endless_vengeance"     ) e = "OnAttackHit_480AP_1PPM_10Dur_60Cd";
  else if ( name == "ashen_band_of_unmatched_vengeance"   ) e = "OnAttackHit_480AP_1PPM_10Dur_60Cd";
  else if ( name == "ashen_band_of_endless_might"         ) e = "OnAttackHit_480AP_1PPM_10Dur_60Cd";
  else if ( name == "ashen_band_of_unmatched_might"       ) e = "OnAttackHit_480AP_1PPM_10Dur_60Cd";
  else if ( name == "banner_of_victory"                   ) e = "OnAttackHit_1008AP_20%_10Dur_50Cd";
  else if ( name == "bell_of_enraging_resonance"          ) e = ( heroic ? "OnHarmfulSpellCast_2178SP_30%_20Dur_100Cd" : "OnHarmfulSpellCast_1926SP_30%_20Dur_100Cd" );
  else if ( name == "black_magic"                         ) e = "OnSpellHit_250Haste_35%_10Dur_35Cd";
  else if ( name == "blood_of_the_old_god"                ) e = "OnAttackCrit_1284AP_10%_10Dur_50Cd";
  else if ( name == "chuchus_tiny_box_of_horrors"         ) e = "OnAttackHit_258Crit_15%_10Dur_45Cd";
  else if ( name == "comets_trail"                        ) e = "OnAttackHit_726Haste_10%_10Dur_45Cd";
  else if ( name == "corens_chromium_coaster"             ) e = "OnAttackCrit_1000AP_10%_10Dur_50Cd";
  else if ( name == "corens_chilled_chromium_coaster"     ) e = "OnAttackCrit_4000AP_10%_10Dur_50Cd";
  else if ( name == "crushing_weight"                     ) e = ( heroic ? "OnAttackHit_2178Haste_10%_15Dur_75Cd" : "OnAttackHit_1926Haste_10%_15Dur_75Cd" );
  else if ( name == "dark_matter"                         ) e = "OnAttackHit_612Crit_15%_10Dur_45Cd";
  else if ( name == "darkmoon_card_crusade"               ) e = "OnDamage_8SP_10Stack_10Dur";
  else if ( name == "dwyers_caber"                        ) e = "OnDamage_1020Crit_15%_20Dur_50Cd";
  else if ( name == "dying_curse"                         ) e = "OnSpellCast_765SP_15%_10Dur_45Cd";
  else if ( name == "elemental_focus_stone"               ) e = "OnHarmfulSpellCast_522Haste_10%_10Dur_45Cd";
  else if ( name == "embrace_of_the_spider"               ) e = "OnSpellCast_505Haste_10%_10Dur_45Cd";
  else if ( name == "essence_of_the_cyclone"              ) e = ( heroic ? "OnAttackHit_2178Crit_10%_10Dur_50Cd" : "OnAttackHit_1926Crit_10%_10Dur_50Cd" );
  else if ( name == "eye_of_magtheridon"                  ) e = "OnSpellMiss_170SP_10Dur";
  else if ( name == "eye_of_the_broodmother"              ) e = "OnSpellDamageHeal_25SP_5Stack_10Dur";
  else if ( name == "flare_of_the_heavens"                ) e = "OnHarmfulSpellCast_850SP_10%_10Dur_45Cd";
  else if ( name == "fluid_death"                         ) e = "OnAttackHit_38Agi_10Stack_15Dur";
  else if ( name == "forge_ember"                         ) e = "OnSpellHit_512SP_10%_10Dur_45Cd";
  else if ( name == "fury_of_the_five_flights"            ) e = "OnAttackHit_16AP_20Stack_10Dur";
  else if ( name == "gale_of_shadows"                     ) e = ( heroic ? "OnSpellTickDamage_17SP_20Stack_15Dur" : "OnSpellTickDamage_15SP_20Stack_15Dur" );
  else if ( name == "grace_of_the_herald"                 ) e = ( heroic ? "OnAttackHit_1710Crit_10%_10Dur_75Cd" : "OnAttackHit_924Crit_10%_10Dur_75Cd" );
  else if ( name == "grim_toll"                           ) e = "OnAttackHit_612Crit_15%_10Dur_45Cd";
  else if ( name == "harrisons_insignia_of_panache"       ) e = "OnAttackHit_918Mastery_10%_20Dur_95Cd"; // TO-DO: Confirm ICD
  else if ( name == "heart_of_rage"                       ) e = ( heroic ? "OnAttackHit_2178Str_10%_20Dur_100Cd" : "OnAttackHit_1926Str_10%_20Dur_100Cd" ); // TO-DO: Confirm ICD.
  else if ( name == "heart_of_solace"                     ) e = ( heroic ? "OnAttackHit_1710Str_10%_20Dur_100Cd" : "OnAttackHit_1512Str_10%_20Dur_100Cd" );
  else if ( name == "heart_of_the_vile"                   ) e = "OnAttackHit_924Crit_10%_10Dur_75Cd";
  else if ( name == "heartsong"                           ) e = "OnSpellDamageHeal_200Spi_25%_15Dur_20Cd";
  else if ( name == "herkuml_war_token"                   ) e = "OnAttackHit_17AP_20Stack_10Dur";
  else if ( name == "illustration_of_the_dragon_soul"     ) e = "OnDamageHealSpellCast_20SP_10Stack_10Dur";
  else if ( name == "key_to_the_endless_chamber"          ) e = ( heroic ? "OnAttackHit_1710Agi_10%_15Dur_75Cd" : "OnAttackHit_1290Agi_10%_15Dur_75Cd" );
  else if ( name == "left_eye_of_rajh"                    ) e = ( heroic ? "OnAttackCrit_1710Agi_50%_10Dur_50Cd" : "OnAttackCrit_1512Agi_50%_10Dur_50Cd" );
  else if ( name == "license_to_slay"                     ) e = "OnAttackHit_38Str_10Stack_15Dur";
  else if ( name == "mark_of_defiance"                    ) e = "OnSpellHit_150Mana_15%_15Cd";
  else if ( name == "mirror_of_truth"                     ) e = "OnAttackCrit_1000AP_10%_10Dur_50Cd";
  else if ( name == "mithril_pocketwatch"                 ) e = "OnHarmfulSpellCast_590SP_10%_10Dur_45Cd";
  else if ( name == "mithril_stopwatch"                   ) e = "OnHarmfulSpellCast_2040SP_10%_10Dur_45Cd"; // FIXME: Confirm ICD
  else if ( name == "mithril_wristwatch"                  ) e = "OnHarmfulSpellCast_5082SP_10%_10Dur_45Cd"; // FIXME: Confirm ICD
  else if ( name == "mjolnir_runestone"                   ) e = "OnAttackHit_665Haste_15%_10Dur_45Cd";
  else if ( name == "muradins_spyglass"                   ) e = ( heroic ? "OnSpellDamage_20SP_10Stack_10Dur" : "OnSpellDamage_18SP_10Stack_10Dur" );
  else if ( name == "necromantic_focus"                   ) e = ( heroic ? "OnSpellTickDamage_44Mastery_10Stack_10Dur" : "OnSpellTickDamage_39Mastery_10Stack_10Dur" );
  else if ( name == "needleencrusted_scorpion"            ) e = "OnAttackCrit_678crit_10%_10Dur_50Cd";
  else if ( name == "pandoras_plea"                       ) e = "OnSpellCast_751SP_10%_10Dur_45Cd";
  else if ( name == "petrified_pickled_egg"               ) e = "OnHeal_2040Haste_10%_10Dur_50Cd"; // FIXME: Confirm ICD
  else if ( name == "thousandyear_pickled_egg"            ) e = "OnHeal_5082Haste_10%_10Dur_50Cd"; // FIXME: Confirm ICD
  else if ( name == "corens_cold_chromium_coaster"        ) e = "OnAttackCrit_10848ap_10%_10Dur_50Cd"; // FIXME: Confirm ICD
  else if ( name == "porcelain_crab"                      ) e = ( heroic ? "OnAttackHit_1710Mastery_10%_20Dur_95Cd" : "OnAttackHit_918Mastery_10%_20Dur_95Cd" ); // TO-DO: Confirm ICD.
  else if ( name == "prestors_talisman_of_machination"    ) e = ( heroic ? "OnAttackHit_2178Haste_10%_15Dur_75Cd" : "OnAttackHit_1926Haste_10%_15Dur_75Cd" ); // TO-DO: Confirm ICD.
  else if ( name == "purified_lunar_dust"                 ) e = "OnSpellCast_304MP5_10%_15Dur_45Cd";
  else if ( name == "pyrite_infuser"                      ) e = "OnAttackCrit_1234AP_10%_10Dur_50Cd";
  else if ( name == "quagmirrans_eye"                     ) e = "OnHarmfulSpellCast_320Haste_10%_6Dur_45Cd";
  else if ( name == "right_eye_of_rajh"                   ) e = ( heroic ? "OnAttackCrit_1710Str_50%_10Dur_50Cd" : "OnAttackCrit_1512Str_50%_10Dur_50Cd" );
  else if ( name == "schnotzzs_medallion_of_command"      ) e = "OnAttackHit_918Mastery_10%_20Dur_95Cd"; // TO-DO: Confirm ICD.
  else if ( name == "sextant_of_unstable_currents"        ) e = "OnSpellCrit_190SP_20%_15Dur_45Cd";
  else if ( name == "shiffars_nexus_horn"                 ) e = "OnSpellCrit_225SP_20%_10Dur_45Cd";
  else if ( name == "stonemothers_kiss"                   ) e = "OnSpellCast_1164Crit_10%_20Dur_75Cd";
  else if ( name == "stump_of_time"                       ) e = "OnHarmfulSpellCast_1926SP_10%_15Dur_75Cd";
  else if ( name == "sundial_of_the_exiled"               ) e = "OnHarmfulSpellCast_590SP_10%_10Dur_45Cd";
  else if ( name == "talisman_of_sinister_order"          ) e = "OnSpellCast_918Mastery_10%_20Dur_95Cd"; // TO-DO: Confirm ICD.
  else if ( name == "tendrils_of_burrowing_dark"          ) e = ( heroic ? "OnSpellCast_1710SP_10%_15Dur_75Cd" : "OnSpellCast_1290SP_10%_15Dur_75Cd" ); // TO-DO: Confirm ICD
  else if ( name == "the_hungerer"                        ) e = ( heroic ? "OnAttackHit_1730Haste_100%_15Dur_60Cd" : "OnAttackHit_1532Haste_100%_15Dur_60Cd" );
  else if ( name == "theralions_mirror"                   ) e = ( heroic ? "OnHarmfulSpellCast_2178Mastery_10%_20Dur_100Cd" : "OnHarmfulSpellCast_1926Mastery_10%_20Dur_100Cd" );
  else if ( name == "tias_grace"                          ) e = ( heroic ? "OnAttackHit_34Agi_10Stack_15Dur" : "OnAttackHit_30Agi_10Stack_15Dur" );
  else if ( name == "unheeded_warning"                    ) e = "OnAttackHit_1926AP_10%_10Dur_50Cd";
  else if ( name == "vessel_of_acceleration"              ) e = ( heroic ? "OnAttackCrit_93Crit_5Stack_20Dur" : "OnAttackCrit_82Crit_5Stack_20Dur" );
  else if ( name == "witching_hourglass"                  ) e = ( heroic ? "OnSpellCast_1710Haste_10%_15Dur_75Cd" : "OnSpellCast_918Haste_10%_15Dur_75Cd" );
  else if ( name == "wrath_of_cenarius"                   ) e = "OnSpellHit_132SP_5%_10Dur";
  else if ( name == "fall_of_mortality"                   ) e = ( heroic ? "OnHealCast_2178Spi_15Dur_75Cd" : "OnHealCast_1926Spi_15Dur_75Cd" );
  else if ( name == "darkmoon_card_tsunami"               ) e = "OnHeal_80Spi_5Stack_20Dur";
  else if ( name == "phylactery_of_the_nameless_lich"     ) e = ( heroic ? "OnSpellTickDamage_1206SP_30%_20Dur_100Cd" : "OnSpellTickDamage_1073SP_30%_20Dur_100Cd" );
  else if ( name == "whispering_fanged_skull"             ) e = ( heroic ? "OnAttackHit_1250AP_35%_15Dur_45Cd" : "OnAttackHit_1110AP_35%_15Dur_45Cd" );
  else if ( name == "charred_twilight_scale"              ) e = ( heroic ? "OnHarmfulSpellCast_861SP_10%_15Dur_45Cd" : "OnHarmfulSpellCast_763SP_10%_15Dur_45Cd" );
  else if ( name == "sharpened_twilight_scale"            ) e = ( heroic ? "OnAttackHit_1472AP_35%_15Dur_45Cd" : "OnAttackHit_1304AP_35%_15Dur_45Cd" );

  // 4.3
  else if ( name == "will_of_unbinding"                   ) e = ( heroic ? "OnHarmfulSpellCast_99Int_100%_10Dur_10Stack" : lfr ? "OnHarmfulSpellCast_78Int_100%_10Dur_10Stack" : "OnHarmfulSpellCast_88Int_100%_10Dur_10Stack" );
  else if ( name == "eye_of_unmaking"                     ) e = ( heroic ? "OnAttackHit_99Str_100%_10Dur_10Stack" : lfr ? "OnAttackHit_78Str_100%_10Dur_10Stack" : "OnAttackHit_88Str_100%_10Dur_10Stack" );
  else if ( name == "wrath_of_unchaining"                 ) e = ( heroic ? "OnAttackHit_99Agi_100%_10Dur_10Stack" : lfr ? "OnAttackHit_78Agi_100%_10Dur_10Stack" : "OnAttackHit_88Agi_100%_10Dur_10Stack" );
  else if ( name == "heart_of_unliving"                   ) e = ( heroic ? "OnHealCast_99Spi_100%_10Dur_10Stack" : lfr ? "OnHealCast_78Spi_100%_10Dur_10Stack" : "OnHealCast_88Spi_100%_10Dur_10Stack" );
  else if ( name == "resolve_of_undying"                  ) e = ( heroic ? "OnAttackHit_99Dodge_100%_10Dur_10Stack" : lfr ? "OnAttackHit_99Dodge_100%_10Dur_10Stack" : "OnAttackHit_88Dodge_100%_10Dur_10Stack" );
  else if ( name == "creche_of_the_final_dragon"          ) e = ( heroic ? "OnDamage_3278Crit_15%_20Dur_115Cd" : lfr ? "OnDamage_2573Crit_15%_20Dur_115Cd" : "OnDamage_2904Crit_15%_20Dur_115Cd" ); // FIXME: ICD needs testing, assuming same as insignia of corrupted mind
  else if ( name == "starcatcher_compass"                 ) e = ( heroic ? "OnDamage_3278Haste_15%_20Dur_115Cd" : lfr ? "OnDamage_2573Haste_15%_20Dur_115Cd" : "OnDamage_2904Haste_15%_20Dur_115Cd" ); // FIXME: ICD needs testing, assuming same as insignia of corrupted mind
  else if ( name == "seal_of_the_seven_signs"             ) e = ( heroic ? "OnHeal_3278Haste_15%_20Dur_115Cd" : lfr ? "OnHeal_2573Haste_15%_20Dur_115Cd" : "OnHeal_2904Haste_15%_20Dur_115Cd" ); // FIXME: ICD needs testing, assuming same as insignia of corrupted mind
  else if ( name == "soulshifter_vortex"                  ) e = ( heroic ? "OnDamage_3278Mastery_15%_20Dur_115Cd" : lfr ? "OnDamage_2573Mastery_15%_20Dur_115Cd" : "OnDamage_2904Mastery_15%_20Dur_115Cd" ); // FIXME: ICD needs testing, assuming same as insignia of corrupted mind
  else if ( name == "insignia_of_the_corrupted_mind"      ) e = ( heroic ? "OnDamage_3278Haste_15%_20Dur_115Cd" : lfr ? "OnDamage_2573Haste_15%_20Dur_115Cd" : "OnDamage_2904Haste_15%_20Dur_115Cd" ); // ICD confirmed on PTR 2011/11/14
  else if ( name == "foul_gift_of_the_demon_lord"         ) e = "OnSpellDamageHeal_1149Mastery_15%_20Dur_45Cd"; // TO-DO: Confirm ICD. Min seen 50.112sec
  else if ( name == "arrow_of_time"                       ) e = "OnAttackHit_1149Haste_20%_20Dur_45Cd";   // TO-DO: Confirm ICD
  else if ( name == "rosary_of_light"                     ) e = "OnAttackHit_1149Crit_15%_20Dur_45Cd";   // TO-DO: Confirm ICD. Min seen 50.934sec.
  else if ( name == "varothens_brooch"                    ) e = "OnAttackHit_1149Mastery_20%_20Dur_45Cd";   // TO-DO: Confirm ICD. Min seen 50.192sec.

  // Cata PvP Trinkets
  //390
  if      ( name == "cataclysmic_gladiators_insignia_of_victory"   ) e = "OnAttackHit_1452Str_15%_20Dur_55Cd";
  else if ( name == "cataclysmic_gladiators_insignia_of_conquest"  ) e = "OnAttackHit_1452Agi_15%_20Dur_55Cd";
  else if ( name == "cataclysmic_gladiators_insignia_of_dominance" ) e = "OnSpellDamage_1452SP_25%_20Dur_55Cd";

  // MoP
  if      ( name == "vision_of_the_predator"              ) e = "OnSpellDamage_3386Crit_15%_30Dur_105Cd";
  else if ( name == "carbonic_carbuncle"                  ) e = "OnDirectDamage_3386Crit_15%_30Dur_105Cd";
  else if ( name == "windswept_pages"                     ) e = "OnDirectDamage_3386Haste_15%_20Dur_65Cd";
  else if ( name == "searing_words"                       ) e = "OnDirectCrit_3386Agi_45%_25Dur_85Cd";
  else if ( name == "light_of_the_cosmos"                 ) e = "OnSpellTickDamage_" + std::string( heroic ? "3653" : lfr ? "2866" : "3236" ) + "Int_15%_20Dur_45Cd";
  else if ( name == "essence_of_terror"                   ) e = "OnSpellDamage_"     + std::string( heroic ? "7796" : lfr ? "6121" : "6908" ) + "Haste_15%_20Dur_105Cd";
  else if ( name == "terror_in_the_mists"                 ) e = "OnDirectDamage_"    + std::string( heroic ? "7796" : lfr ? "6121" : "6908" ) + "Crit_15%_20Dur_105Cd";
  else if ( name == "darkmist_vortex"                     ) e = "OnDirectDamage_"    + std::string( heroic ? "7796" : lfr ? "6121" : "6908" ) + "Haste_15%_20Dur_105Cd";
  else if ( name == "relic_of_yulon"                      ) e = "OnSpellDamage_3027Int_20%_15Dur_50Cd";
  else if ( name == "relic_of_chiji"                      ) e = "OnHealCast_3027Spi_20%_15Dur_45Cd";
  else if ( name == "relic_of_xuen" && item_id == 79327   ) e = "OnAttackHit_3027Str_20%_15Dur_45Cd";
  else if ( name == "relic_of_xuen" && item_id == 79328   ) e = "OnAttackCrit_3027Agi_20%_15Dur_55Cd";
  else if ( name == "bottle_of_infinite_stars"            ) e = "OnAttackHit_"       + std::string( heroic ? "3653" : lfr ? "2866" : "3236" ) + "Agi_15%_20Dur_45Cd";
  else if ( name == "vial_of_dragons_blood"               ) e = "OnAttackHit_"       + std::string( heroic ? "3653" : lfr ? "2866" : "3236" ) + "Dodge_15%_20Dur_45Cd";
  else if ( name == "lei_shens_final_orders"              ) e = "OnAttackHit_"       + std::string( heroic ? "3653" : lfr ? "2866" : "3236" ) + "Str_15%_20Dur_45Cd";
  else if ( name == "qinxis_polarizing_seal"              ) e = "OnHealHit_"         + std::string( heroic ? "3653" : lfr ? "2866" : "3236" ) + "Int_15%_20Dur_45Cd";
  else if ( name == "spirits_of_the_sun"                  ) e = "OnHealHit_"         + std::string( heroic ? "7796" : lfr ? "6121" : "6908" ) + "Spi_15%_20Dur_105Cd";

//MoP Tank Trinkets
  else if ( name == "stuff_of_nightmares"                 ) e = "OnAttackHit_"       + std::string( heroic ? "7796" : lfr ? "6121" : "6908" ) + "Dodge_15%_20Dur_105Cd"; //assuming same ICD as spirits of the sun etc since the proc value is the same
  else if ( name == "iron_protector_talisman"             ) e = "OnAttackHit_3386Dodge_15%_15Dur_45Cd";

  // 5.2 Trinkets
  else if ( name == "talisman_of_bloodlust"               ) e = "OnDirectDamage_"    + std::string( heroic ? "1736" : lfr ? "1277" : "1538" ) + "Haste_3RPPM_5Stack_10Dur";
  else if ( name == "primordius_talisman_of_rage"         ) e = "OnDirectDamage_"    + std::string( heroic ? "1736" : lfr ? "1277" : "1538" ) + "Str_3RPPM_5Stack_10Dur";
  else if ( name == "gaze_of_the_twins"                   ) e = "OnAttackCrit_"      + std::string( heroic ? "3238" : lfr ? "2381" : "2868" ) + "Crit_0.8RPPMAttackCrit_3Stack_20Dur";
  else if ( name == "renatakis_soul_charm"                ) e = "OnDirectDamage_"    + std::string( heroic ? "1505" : lfr ? "1107" : "1333" ) + "Agi_0.56RPPM_10Stack_20Dur_2Tick_22Cd";
  else if ( name == "fabled_feather_of_jikun"             ) e = "OnDirectDamage_"    + std::string( heroic ? "1505" : lfr ? "1107" : "1333" ) + "Str_0.56RPPM_10Stack_20Dur_2Tick_22Cd";

  else if ( name == "wushoolays_final_choice"             ) e = "OnSpellDamage_"     + std::string( heroic ? "1505" : lfr ? "1107" : "1333" ) + "Int_0.56RPPM_10Stack_20Dur_2Tick_22Cd";
  else if ( name == "breath_of_the_hydra"                 ) e = "OnSpellTickDamage_" + std::string( heroic ? "8279" : lfr ? "6088" : "7333" ) + "Int_0.5RPPM_20Dur";
  else if ( name == "chayes_essence_of_brilliance"        ) e = "OnHarmfulSpellCrit_"+ std::string( heroic ? "8279" : lfr ? "6088" : "7333" ) + "Int_0.77RPPMSpellCrit_20Dur";

  else if ( name == "brutal_talisman_of_the_shadopan_assault" ) e = "OnDirectDamage_8800Str_15%_15Dur_75Cd";
  else if ( name == "vicious_talisman_of_the_shadopan_assault" ) e = "OnDirectDamage_8800Agi_15%_20Dur_105Cd";
  else if ( name == "volatile_talisman_of_the_shadopan_assault" ) e = "OnHarmfulSpellHit_8800Haste_15%_10Dur_45Cd";

//MoP PvP Trinkets (FIXME: Confirm proc data.. tooltips are broken and spells are not really finalized)
  //483
  else if ( name == "malevolent_gladiators_insignia_of_victory" ) e = "OnAttackHit_3603Str_15%_20Dur_55Cd";
  else if ( name == "malevolent_gladiators_insignia_of_conquest" ) e = "OnAttackHit_3603Agi_15%_20Dur_55Cd";
  else if ( name == "malevolent_gladiators_insignia_of_dominance" ) e = "OnSpellDamage_3603SP_25%_20Dur_55Cd";
  //464
  else if ( name == "dreadful_gladiators_insignia_of_victory" ) e = "OnAttackHit_3017Str_15%_20Dur_55Cd";
  else if ( name == "dreadful_gladiators_insignia_of_conquest" ) e = "OnAttackHit_3017Agi_15%_20Dur_55Cd";
  else if ( name == "dreadful_gladiators_insignia_of_dominance" ) e = "OnSpellDamage_3017SP_25%_20Dur_55Cd";

  // Stat Procs with Tick Increases
  else if ( name == "dislodged_foreign_object"            ) e = ( heroic ? "OnHarmfulSpellCast_121SP_10Stack_10%_20Dur_45Cd_2Tick" : "OnHarmfulSpellCast_105SP_10Stack_10%_20Dur_45Cd_2Tick" );

  // Discharge Procs
  else if ( name == "bandits_insignia"                    ) e = "OnAttackHit_1880Arcane_15%_45Cd";
  else if ( name == "darkmoon_card_hurricane"             ) e = "OnAttackHit_-7000Nature_1PPM_nocrit";
  else if ( name == "darkmoon_card_volcano"               ) e = "OnSpellDamage_1200+10Fire_1600Int_30%_12Dur_45Cd";
  else if ( name == "extract_of_necromantic_power"        ) e = "OnSpellTickDamage_1050Shadow_10%_15Cd";
  else if ( name == "lightning_capacitor"                 ) e = "OnSpellCrit_750Nature_3Stack_2.5Cd";
  else if ( name == "timbals_crystal"                     ) e = "OnSpellTickDamage_380Shadow_10%_15Cd";
  else if ( name == "thunder_capacitor"                   ) e = "OnSpellCrit_1276Nature_4Stack_2.5Cd";
  else if ( name == "bryntroll_the_bone_arbiter"          ) e = ( heroic ? "OnAttackHit_2538Drain_11%" : "OnAttackHit_2250Drain_11%" );
  else if ( name == "cunning_of_the_cruel"                ) e = ( heroic ? "OnSpellDamage_3978.8+35.3Shadow_45%_9Cd_aoe" : lfr ? "OnSpellDamage_3122.6+27.7Shadow_45%_9Cd_aoe" : "OnSpellDamage_3524.5+31.3Shadow_45%_9Cd_aoe" );
  else if ( name == "vial_of_shadows"                     ) e = ( heroic ? "OnAttackHit_-5682+33.90Physical_45%_9Cd_NoDodge_NoParry_NoBlock" : lfr ? "OnAttackHit_-4460.5+26.60Physical_45%_9Cd_NoDodge_NoParry_NoBlock" : "OnAttackHit_-5035+30.00Physical_45%_9Cd_NoDodge_NoParry_NoBlock" ); // ICD, base damage, and ap coeff determined experimentally on heroic version. Assuming dbc has wrong base damage. Normal and LFR assumed to be changed by the same %
  else if ( name == "reign_of_the_unliving"               ) e = ( heroic ? "OnSpellDirectCrit_2117Fire_3Stack_2.0Cd" : "OnSpellDirectCrit_1882Fire_3Stack_2.0Cd" );
  else if ( name == "reign_of_the_dead"                   ) e = ( heroic ? "OnSpellDirectCrit_2117Fire_3Stack_2.0Cd" : "OnSpellDirectCrit_1882Fire_3Stack_2.0Cd" );
  else if ( name == "solace_of_the_defeated"              ) e = ( heroic ? "OnSpellCast_18MP5_8Stack_10Dur" : "OnSpellCast_16MP5_8Stack_10Dur" );
  else if ( name == "solace_of_the_fallen"                ) e = ( heroic ? "OnSpellCast_18MP5_8Stack_10Dur" : "OnSpellCast_16MP5_8Stack_10Dur" );

  // Variable Stack Discharge Procs
  else if ( name == "variable_pulse_lightning_capacitor"  ) e = ( heroic ? "OnSpellCrit_3300.7Nature_15%_10Stack_2.5Cd_chance" : "OnSpellCrit_2926.3Nature_15%_10Stack_2.5Cd_chance" );

  // Enchants
  if      ( name == "lightweave_1"                        ) e = "OnSpellCast_295SP_35%_15Dur_60Cd";
  else if ( name == "lightweave_embroidery_1"             ) e = "OnSpellCast_295SP_35%_15Dur_60Cd";
  else if ( name == "lightweave_2" ||
            name == "lightweave_embroidery_2"             ) e = "OnSpellDamageHeal_580Int_25%_15Dur_64Cd";
  else if ( name == "lightweave_3" ||
            name == "lightweave_embroidery_3"             ) e = "OnSpellDamageHeal_2000Int_25%_15Dur_57Cd";
  else if ( name == "darkglow_1"                          ) e = "OnSpellCast_250Spi_35%_15Dur_60Cd";
  else if ( name == "darkglow_embroidery_1"               ) e = "OnSpellCast_250Spi_35%_15Dur_60Cd";
  else if ( name == "darkglow_2"                          ) e = "OnSpellCast_580Spi_30%_15Dur_45Cd";
  else if ( name == "darkglow_embroidery_2"               ) e = "OnSpellCast_580Spi_30%_15Dur_45Cd";
  else if ( name == "darkglow_3"                          ) e = "OnSpellCast_3000Spi_25%_15Dur_57Cd";
  else if ( name == "darkglow_embroidery_3"               ) e = "OnSpellCast_3000Spi_25%_15Dur_57Cd";
  else if ( name == "swordguard_1"                        ) e = "OnAttackHit_400AP_20%_15Dur_60Cd";
  else if ( name == "swordguard_embroidery_1"             ) e = "OnAttackHit_400AP_20%_15Dur_60Cd";
  else if ( name == "swordguard_2"                        ) e = "OnAttackHit_1000AP_15%_15Dur_55Cd";
  else if ( name == "swordguard_embroidery_2"             ) e = "OnAttackHit_1000AP_15%_15Dur_55Cd";
  else if ( name == "swordguard_3"                        ) e = "OnAttackHit_4000AP_15%_15Dur_57Cd";
  else if ( name == "swordguard_embroidery_3"             ) e = "OnAttackHit_4000AP_15%_15Dur_57Cd";
  else if ( name == "flintlockes_woodchucker"             ) e = "OnAttackHit_1100Physical_300Agi_10%_10Dur_40Cd_nocrit"; // TO-DO: Confirm ICD.

  // DK Runeforges
  else if ( name == "rune_of_cinderglacier"               ) e = "custom";
  else if ( name == "rune_of_razorice"                    ) e = "custom";
  else if ( name == "rune_of_the_fallen_crusader"         ) e = "custom";

  if ( e.empty() ) return false;

  util::tolower( e );

  encoding = e;

  return true;
}

// ==========================================================================
// unique_gear::get_use_encoding
// ==========================================================================

bool unique_gear::get_use_encoding( std::string&       encoding,
                                    const std::string& name,
                                    bool         heroic,
                                    bool         lfr,
                                    bool         /* ptr */,
                                    const std::string& /* id */ )
{
  std::string e;

  // Simple
  if      ( name == "ancient_petrified_seed"       ) e = ( heroic ? "1441Agi_15Dur_60Cd"  : "1277Agi_15Dur_60Cd" );
  else if ( name == "brawlers_trophy"              ) e = "1700Dodge_20Dur_120Cd";
  else if ( name == "core_of_ripeness"             ) e = "1926Spi_20Dur_120Cd";
  else if ( name == "electrospark_heartstarter"    ) e = "567Int_20Dur_120Cd";
  else if ( name == "energy_siphon"                ) e = "408SP_20Dur_120Cd";
  else if ( name == "ephemeral_snowflake"          ) e = "464Haste_20Dur_120Cd";
  else if ( name == "essence_of_the_eternal_flame" ) e = ( heroic ? "1441Str_15Dur_60Cd" : "1277Str_15Dur_60Cd" );
  else if ( name == "fiery_quintessence"           ) e = ( heroic ? "1297Int_25Dur_90Cd"  : "1149Int_25Dur_90Cd" );
  else if ( name == "figurine__demon_panther"      ) e = "1425Agi_20Dur_120Cd";
  else if ( name == "figurine__dream_owl"          ) e = "1425Spi_20Dur_120Cd";
  else if ( name == "figurine__jeweled_serpent"    ) e = "1425Sp_20Dur_120Cd";
  else if ( name == "figurine__king_of_boars"      ) e = "1425Str_20Dur_120Cd";
  else if ( name == "impatience_of_youth"          ) e = "1605Str_20Dur_120Cd";
  else if ( name == "jaws_of_defeat"               ) e = ( heroic ? "OnSpellCast_125HolyStorm_CostRd_10Stack_20Dur_120Cd" : "OnSpellCast_110HolyStorm_CostRd_10Stack_20Dur_120Cd" );
  else if ( name == "living_flame"                 ) e = "505SP_20Dur_120Cd";
  else if ( name == "maghias_misguided_quill"      ) e = "716SP_20Dur_120Cd";
  else if ( name == "magnetite_mirror"             ) e = ( heroic ? "1425Str_15Dur_90Cd" : "1075Str_15Dur_90Cd" );
  else if ( name == "mark_of_khardros"             ) e = ( heroic ? "1425Mastery_15Dur_90Cd" : "1260Mastery_15Dur_90Cd" );
  else if ( name == "mark_of_norgannon"            ) e = "491Haste_20Dur_120Cd";
  else if ( name == "mark_of_supremacy"            ) e = "1024AP_20Dur_120Cd";
  else if ( name == "mark_of_the_firelord"         ) e = ( heroic ? "1441Int_15Dur_60Cd"  : "1277Int_15Dur_60Cd" );
  else if ( name == "moonwell_chalice"             ) e = "1700Mastery_20Dur_120Cd";
  else if ( name == "moonwell_phial"               ) e = "1700Dodge_20Dur_120Cd";
  else if ( name == "might_of_the_ocean"           ) e = ( heroic ? "1425Str_15Dur_90Cd" : "765Str_15Dur_90Cd" );
  else if ( name == "platinum_disks_of_battle"     ) e = "752AP_20Dur_120Cd";
  else if ( name == "platinum_disks_of_sorcery"    ) e = "440SP_20Dur_120Cd";
  else if ( name == "platinum_disks_of_swiftness"  ) e = "375Haste_20Dur_120Cd";
  else if ( name == "rickets_magnetic_fireball"    ) e = "1700Crit_20Dur_120Cd"; // FIXME: "Your attacks may occasionally attract small celestial objects."
  else if ( name == "rune_of_zeth"                 ) e = ( heroic ? "1441Int_15Dur_60Cd" : "1277Int_15Dur_60Cd" );
  else if ( name == "scale_of_fates"               ) e = "432Haste_20Dur_120Cd";
  else if ( name == "sea_star"                     ) e = ( heroic ? "1425Sp_20Dur_120Cd" : "765Sp_20Dur_120Cd" );
  else if ( name == "shard_of_the_crystal_heart"   ) e = "512Haste_20Dur_120Cd";
  else if ( name == "shard_of_woe"                 ) e = "1935Haste_10Dur_60Cd";
  else if ( name == "skardyns_grace"               ) e = ( heroic ? "1425Mastery_20Dur_120Cd" : "1260Mastery_20Dur_120Cd" );
  else if ( name == "sliver_of_pure_ice"           ) e = "1625Mana_120Cd";
  else if ( name == "soul_casket"                  ) e = "1926Sp_20Dur_120Cd";
  else if ( name == "souls_anguish"                ) e = "765Str_15Dur_90Cd";
  else if ( name == "spirit_world_glass"           ) e = "336Spi_20Dur_120Cd";
  else if ( name == "talisman_of_resurgence"       ) e = "599SP_20Dur_120Cd";
  else if ( name == "unsolvable_riddle"            ) e = "1605Agi_20Dur_120Cd";
  else if ( name == "wrathstone"                   ) e = "856AP_20Dur_120Cd";
  // 4.3
  else if ( name == "bottled_wishes"               ) e = ( heroic ? "2585SP_15Dur_90Cd" : lfr ? "2029SP_15Dur_90Cd" : "2290SP_15Dur_90Cd" );
  else if ( name == "kiroptyric_sigil"             ) e = ( heroic ? "2585Agi_15Dur_90Cd" : lfr ? "2029Agi_15Dur_90Cd" : "2290Agi_15Dur_90Cd" );
  else if ( name == "rotting_skull"                ) e = ( heroic ? "2585Str_15Dur_90Cd" : lfr ? "2029Str_15Dur_90Cd" : "2290Str_15Dur_90Cd" );
  else if ( name == "fire_of_the_deep"             ) e = ( heroic ? "2585Dodge_15Dur_90Cd" : lfr ? "2029Dodge_15Dur_90Cd" : "2290Dodge_15Dur_90Cd" );
  else if ( name == "reflection_of_the_light"      ) e = ( heroic ? "2585SP_15Dur_90Cd" : lfr ? "2029SP_15Dur_90Cd" : "2290SP_15Dur_90Cd" );

  // MoP

  else if ( name == "flashfrozen_resin_globule"    ) e = "4232Int_25Dur_150Cd";
  else if ( name == "flashing_steel_talisman"      ) e = "4232Agi_15Dur_90Cd";
  else if ( name == "vial_of_ichorous_blood"       ) e = "4241Spi_20Dur_120Cd";
  else if ( name == "lessons_of_the_darkmaster"    ) e = "4232Str_20Dur_120Cd";
  else if ( name == "daelos_final_words"           ) e = "5633Str_10Dur_90Cd";
  else if ( name == "gerps_perfect_arrow"          ) e = "3480Agi_20Dur_120Cd";
  else if ( name == "jade_bandit_figurine"         ) e = std::string( heroic ? "4059" : lfr ? "3184" : "3595" ) + "Haste_15Dur_60Cd";
  else if ( name == "jade_charioteer_figurine"     ) e = std::string( heroic ? "4059" : lfr ? "3184" : "3595" ) + "Haste_15Dur_60Cd";
  else if ( name == "jade_magistrate_figurine"     ) e = std::string( heroic ? "4059" : lfr ? "3184" : "3595" ) + "Crit_15Dur_60Cd";
  else if ( name == "jade_courtesan_figurine"      ) e = std::string( heroic ? "4059" : lfr ? "3184" : "3595" ) + "Spi_15Dur_60Cd";
  else if ( name == "jade_warlord_figurine"        ) e = std::string( heroic ? "4059" : lfr ? "3184" : "3595" ) + "Mastery_15Dur_60Cd";
  else if ( name == "hawkmasters_talon"            ) e = "3595Haste_15Dur_60Cd";
  else if ( name == "laochins_liquid_courage"      ) e = "2822Mastery_15Dur_60Cd";
  else if ( name == "relic_of_niuzao"              ) e = "8871Dodge_12Dur_60Cd";
  else if ( name == "brawlers_statue"              ) e = "4576Dodge_20Dur_120Cd";
  else if ( name == "heart_of_fire"                ) e = "4232Dodge_20Dur_120Cd";
  else if ( name == "blossom_of_pure_snow"         ) e = "3595Crit_15Dur_60Cd";
  else if ( name == "iron_belly_wok"               ) e = "3595Haste_15Dur_60Cd";
  else if ( name == "shockcharger_medallion"       ) e = "3838Int_15Dur_60Cd";
  else if ( name == "helmbreaker_medallion"        ) e = "3838Crit_15Dur_60Cd";
  else if ( name == "arrowflight_medallion"        ) e = "3838Crit_15Dur_60Cd";
  else if ( name == "vaporshield_medallion"        ) e = "3838Mastery_15Dur_60Cd";
  else if ( name == "heartwarmer_medallion"        ) e = "3838Spirit_15Dur_60Cd";
  else if ( name == "cutstitcher_medallion"        ) e = "3838Spirit_15Dur_60Cd";
  else if ( name == "medallion_of_mystifying_vapors" ) e = "3838Mastery_15Dur_60Cd";
  else if ( name == "skullrender_medallion"        ) e = "3838Crit_15Dur_60Cd";
  else if ( name == "staticcasters_medallion"      ) e = "3838Int_15Dur_60Cd";
  else if ( name == "woundripper_medallion"        ) e = "3838Crit_15Dur_60Cd";
  else if ( name == "mending_badge_of_the_shieldwall"  ) e = "2693Spirit_15Dur_60Cd";
  else if ( name == "knightly_badge_of_the_shieldwall" ) e = "2693Mastery_15Dur_60Cd";
  else if ( name == "arcane_badge_of_the_shieldwall"   ) e = "2693Haste_15Dur_60Cd";
  else if ( name == "deadeye_badge_of_the_shieldwall"  ) e = "2693Mastery_15Dur_60Cd";
  else if ( name == "durable_badge_of_the_shieldwall"  ) e = "2693Mastery_15Dur_60Cd";
  else if ( name == "dominators_mending_badge"     ) e = "2693Spirit_15Dur_60Cd";
  else if ( name == "dominators_knightly_badge"    ) e = "2693Mastery_15Dur_60Cd";
  else if ( name == "dominators_arcane_badge"      ) e = "2693Haste_15Dur_60Cd";
  else if ( name == "dominators_deadeye_badge"     ) e = "2693Mastery_15Dur_60Cd";
  else if ( name == "dominators_durable_badge"     ) e = "2693Mastery_15Dur_60Cd";


  // MoP PvP
  else if ( name == "dreadful_gladiators_badge_of_dominance"   ) e = "4275SP_20Dur_120Cd";
  else if ( name == "dreadful_gladiators_badge_of_victory"     ) e = "4275Str_20Dur_120Cd";
  else if ( name == "dreadful_gladiators_badge_of_conquest"    ) e = "4275Agi_20Dur_120Cd";
  else if ( name == "malevolent_gladiators_badge_of_dominance" ) e = "5105SP_20Dur_120Cd";
  else if ( name == "malevolent_gladiators_badge_of_victory"   ) e = "5105Str_20Dur_120Cd";
  else if ( name == "malevolent_gladiators_badge_of_conquest"  ) e = "5105Agi_20Dur_120Cd";

  // Hybrid
  else if ( name == "fetish_of_volatile_power"   ) e = ( heroic ? "OnHarmfulSpellCast_64Haste_8Stack_20Dur_120Cd" : "OnHarmfulSpellCast_57Haste_8Stack_20Dur_120Cd" );
  else if ( name == "talisman_of_volatile_power" ) e = ( heroic ? "OnHarmfulSpellCast_64Haste_8Stack_20Dur_120Cd" : "OnHarmfulSpellCast_57Haste_8Stack_20Dur_120Cd" );
  else if ( name == "vengeance_of_the_forsaken"  ) e = ( heroic ? "OnAttackHit_250AP_5Stack_20Dur_120Cd" : "OnAttackHit_215AP_5Stack_20Dur_120Cd" );
  else if ( name == "victors_call"               ) e = ( heroic ? "OnAttackHit_250AP_5Stack_20Dur_120Cd" : "OnAttackHit_215AP_5Stack_20Dur_120Cd" );
  else if ( name == "nevermelting_ice_crystal"   ) e = "OnSpellCrit_184Crit_5Stack_20Dur_180Cd_reverse";

  // Engineering Tinkers
  else if ( name == "pyrorocket"                   ) e = "1165Fire_45Cd";  // temporary for backwards compatibility
  else if ( name == "hand_mounted_pyro_rocket"     ) e = "1165Fire_45Cd";
  else if ( name == "hyperspeed_accelerators"      ) e = "240Haste_12Dur_60Cd";
  else if ( name == "tazik_shocker"                ) e = "4800Nature_120Cd";
  else if ( name == "quickflip_deflection_plates"  ) e = "1500Armor_12Dur_60Cd";

  if ( e.empty() ) return false;

  util::tolower( e );

  encoding = e;

  return true;
}
