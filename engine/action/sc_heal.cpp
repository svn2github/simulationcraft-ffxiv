// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simulationcraft.hpp"

// ==========================================================================
// Heal
// ==========================================================================

namespace { // anonymous namespace

struct aoe_player_list_callback_t : public callback_t
{
  action_t* action;
  aoe_player_list_callback_t( action_t* a ) : callback_t(), action( a ) {}

  virtual void execute()
  {
    // Invalidate target cache
    action -> target_cache.is_valid = false;
  }
};

} // end anonymous namespace

// heal_t::heal_t ======== Heal Constructor =================================

heal_t::heal_t( const std::string&  token,
                player_t*           p,
                const spell_data_t* s ) :
  spell_base_t( ACTION_HEAL, token, p, s ),
  group_only(),
  pct_heal(),
  heal_gain( p -> get_gain( name() ) )
{
  if ( sim -> heal_target && target == sim -> target )
    target = sim -> heal_target;
  else if ( target -> is_enemy() )
    target = p;

  weapon_multiplier = 0.0;
  may_crit          = true;
  tick_may_crit     = true;

  stats -> type = STATS_HEAL;
}

void heal_t::init_target_cache()
{
  target_cache.callback = new aoe_player_list_callback_t( this );
  sim -> player_non_sleeping_list.register_callback( target_cache.callback );
}

// heal_t::parse_effect_data ================================================

void heal_t::parse_effect_data( const spelleffect_data_t& e )
{
  base_t::parse_effect_data( e );

  if ( e.ok() )
  {
    if ( e.type() == E_HEAL_PCT )
    {
      pct_heal = e.average( player );
    }
  }
}

// heal_t::calculate_direct_amount ==========================================

double heal_t::calculate_direct_amount( action_state_t* state )
{
  if ( pct_heal )
  {
    double amount = state -> target -> resources.max[ RESOURCE_HEALTH ] * pct_heal;
    
    // Record initial amount to state
    state -> result_raw = amount;
    
    if ( state -> result == RESULT_CRIT )
    {
      amount *= 1.0 + total_crit_bonus();
    }
    
    // Record total amount to state
    state -> result_total = amount;
    return amount;
    
  }
  
  return base_t::calculate_direct_amount( state );
}
// heal_t::execute ==========================================================

void heal_t::execute()
{
  spell_base_t::execute();

  if ( callbacks )
  {
    result_e r = execute_state ? execute_state -> result : RESULT_NONE;
    if ( r != RESULT_NONE )
    {
      action_callback_t::trigger( player -> callbacks.heal[ r ], this );
    }
    if ( ! background ) // OnSpellCast
    {
      action_callback_t::trigger( player -> callbacks.heal[ RESULT_NONE ], this );
    }
  }
}

// heal_t::assess_damage ====================================================

void heal_t::assess_damage( dmg_e heal_type,
                            action_state_t* state )
{
  heal_state_t* s = debug_cast<heal_state_t*>( state );

  s -> target -> assess_heal( school, heal_type, s );

  if ( heal_type == HEAL_DIRECT )
  {
    if ( sim -> log )
    {
      sim -> out_log.printf( "%s %s heals %s for %.0f (%.0f) (%s)",
                     player -> name(), name(),
                     s -> target -> name(), s -> total_result_amount, s -> result_amount,
                     util::result_type_string( s -> result ) );
    }

    if ( callbacks && ! direct_tick_callbacks ) action_callback_t::trigger( player -> callbacks.direct_heal[ school ], this, s );
    if ( direct_tick_callbacks ) action_callback_t::trigger( player -> callbacks.tick_heal[ school ], this, s );
  }
  else // HEAL_OVER_TIME
  {
    if ( sim -> log )
    {
      dot_t* dot = get_dot( s -> target );
      sim -> out_log.printf( "%s %s ticks (%d of %d) %s for %.0f (%.0f) heal (%s)",
                     player -> name(), name(),
                     dot -> current_tick, dot -> num_ticks,
                     s -> target -> name(), s -> total_result_amount, s -> result_amount,
                     util::result_type_string( s -> result ) );
    }

    if ( callbacks ) action_callback_t::trigger( player -> callbacks.tick_heal[ school ], this, s );
  }

  stats -> add_result( s -> result_amount, s -> total_result_amount, ( direct_tick ? HEAL_OVER_TIME : heal_type ), s -> result, s -> block_result, s -> target );
  // Record external healing too
  if ( player != state -> target )
    state -> target -> gains.health -> add( RESOURCE_HEALTH, s -> result_amount, s -> total_result_amount - s -> result_amount );
  else
    heal_gain -> add( RESOURCE_HEALTH, s -> result_amount, s -> total_result_amount - s -> result_amount );
}

// heal_t::find_greatest_difference_player ==================================

player_t* heal_t::find_greatest_difference_player()
{
  double max = 0;
  player_t* max_player = player;
  for ( size_t i = 0; i < sim -> player_list.size(); ++i )
  {
    player_t* p = sim -> player_list[ i ];
    // No love for pets right now
    double diff = p -> is_pet() ? 0 : p -> resources.max[ RESOURCE_HEALTH ] - p -> resources.current[ RESOURCE_HEALTH ];
    if ( diff > max )
    {
      max = diff;
      max_player = p;
    }
  }
  return max_player;
}

// heal_t::find_lowest_player ===============================================

player_t* heal_t::find_lowest_player()
{
  double min = 1.0;
  player_t* max_player = player;

  for ( size_t i = 0; i < sim -> player_no_pet_list.size(); ++i ) // check players only
  {
    player_t* p = sim -> player_no_pet_list[ i ];
    double hp_pct =  p -> resources.pct( RESOURCE_HEALTH );
    if ( hp_pct < min )
    {
      min = hp_pct;
      max_player = p;
    }
  }

  if ( min == 1.0 ) // no player found - check pets
  {
    for ( size_t i = 0; i < sim -> player_list.size(); ++i )
    {
      player_t* p = sim -> player_list[ i ];
      if ( !p -> is_pet() ) continue;
      double hp_pct =  p -> resources.pct( RESOURCE_HEALTH );
      if ( hp_pct < min )
      {
        min = hp_pct;
        max_player = p;
      }
    }
  }

  return max_player;
}

std::vector<player_t*> heal_t::find_lowest_players( int num_players ) const
{
  // vector in which to store lowest N players
  std::vector<player_t*> lowest_N_players = sim -> player_no_pet_list.data();

  while ( lowest_N_players.size() > static_cast< size_t > ( num_players ) )
  {
    // find the remaining player with the highest health
    double max = -1e7;
    size_t max_player_index = 0;
    for ( size_t i = 0; i < lowest_N_players.size(); ++i )
    {
      player_t* p = lowest_N_players[ i ];
      double hp_pct = p -> resources.pct( RESOURCE_HEALTH );
      if ( hp_pct > max )
      {
        max = hp_pct;
        max_player_index = i;
      }
    }
    // remove that player from lowest_N_players
    lowest_N_players.erase( lowest_N_players.begin() + max_player_index );
  }

  return lowest_N_players;
}

// heal_t::num_targets ======================================================

int heal_t::num_targets()
{
  int count = 0;

  for ( size_t i = 0, actors = sim -> actor_list.size(); i < actors; i++ )
  {
    player_t* t = sim -> actor_list[ i ];

    if ( ! t -> is_sleeping() && ! t -> is_enemy() && ( t != target ) )
      if ( ! group_only || ( t -> party == target -> party ) )
        count++;
  }

  return count;
}

// heal_t::available_targets ================================================

size_t heal_t::available_targets( std::vector< player_t* >& tl ) const
{
  tl.clear();
  tl.push_back( target );

  for ( size_t i = 0, actors = sim -> player_non_sleeping_list.size(); i < actors; i++ )
  {
    player_t* t = sim -> player_non_sleeping_list[ i ];

    if ( t != target )
      if ( ! group_only || ( t -> party == target -> party ) )
        tl.push_back( t );
  }

  return tl.size();
}

// heal_t::create_expression ================================================

expr_t* heal_t::create_expression( const std::string& name )
{
  class heal_expr_t : public expr_t
  {
  public:
    heal_t& heal;

    heal_expr_t( const std::string& name, heal_t& h ) :
      expr_t( name ), heal( h ) {}
  };

  if ( name_str == "active_allies" )
  {
    struct active_allies_expr_t : public heal_expr_t
    {
      active_allies_expr_t( heal_t& h ) : heal_expr_t( "active_allies", h ) {}
      virtual double evaluate() { return heal.num_targets(); }
    };
    return new active_allies_expr_t( *this );
  }

  return spell_base_t::create_expression( name );
}
