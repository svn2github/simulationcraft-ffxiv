// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simulationcraft.h"

namespace { // ANONYMOUS NAMESPACE ==========================================


// print_html_action_damage =================================================

static void print_html_action_damage( FILE* file, stats_t* s, player_t* p, int j )
{
  int id = 0;

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t<tr" );
  if ( j & 1 )
  {
    util_t::fprintf( file, " class=\"odd\"" );
  }
  util_t::fprintf( file, ">\n" );

  for ( action_t* a = s -> player -> action_list; a; a = a -> next )
  {
    if ( a -> stats != s ) continue;
    id = a -> id;
    if ( ! a -> background ) break;
  }

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<td class=\"left small\">" );
  if ( p -> sim -> report_details )
    util_t::fprintf( file,
                     "<a href=\"#\" class=\"toggle-details\" rel=\"spell=%i\">%s</a></td>\n",
                     id,
                     s -> name_str.c_str() );
  else
    util_t::fprintf( file,
                     "%s</td>\n",
                     s -> name_str.c_str() );
  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.1f%%</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.1f</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.2fsec</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.1f%%</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.1f%%</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.1f%%</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.1f%%</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.1f%%</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.1f%%</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.1f%%</td>\n"
                   "\t\t\t\t\t\t\t</tr>\n",
                   s -> portion_aps,
                   s -> portion_amount * 100,
                   s -> num_executes,
                   s -> frequency,
                   s -> ape,
                   s -> apet,
                   s -> direct_results[ RESULT_HIT  ].avg_amount,
                   s -> direct_results[ RESULT_CRIT ].avg_amount,
                   s -> direct_results[ RESULT_CRIT ].max_amount ? s -> direct_results[ RESULT_CRIT ].max_amount : s -> direct_results[ RESULT_HIT ].max_amount,
                   s -> direct_results[ RESULT_CRIT ].pct,
                   s -> direct_results[ RESULT_MISS ].pct +
                   s -> direct_results[ RESULT_DODGE  ].pct +
                   s -> direct_results[ RESULT_PARRY  ].pct,
                   s -> direct_results[ RESULT_GLANCE ].pct,
                   s -> direct_results[ RESULT_BLOCK  ].pct,
                   s -> num_ticks,
                   s -> tick_results[ RESULT_HIT  ].avg_amount,
                   s -> tick_results[ RESULT_CRIT ].avg_amount,
                   s -> tick_results[ RESULT_CRIT ].pct,
                   s -> tick_results[ RESULT_MISS ].pct +
                   s -> tick_results[ RESULT_DODGE ].pct +
                   s -> tick_results[ RESULT_PARRY ].pct,
                   100 * s -> total_tick_time / s -> player -> total_seconds );

  if ( p -> sim -> report_details )
  {
    std::string timeline_stat_aps_str                    = "";
    if ( ! s -> timeline_aps_chart.empty() )
    {
      timeline_stat_aps_str = "<img src=\"" + s -> timeline_aps_chart + "\" alt=\"APS Timeline Chart\" />\n";
    }
    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t<tr class=\"details hide\">\n"
                     "\t\t\t\t\t\t\t\t<td colspan=\"20\" class=\"filler\">\n" );

    // Stat Details
    util_t::fprintf ( file,
                      "\t\t\t\t\t\t\t\t\t<h4>Stats details: %s </h4>\n", s -> name_str.c_str() );

    util_t::fprintf ( file,
                      "\t\t\t\t\t\t\t\t\t<table class=\"details\">\n"
                      "\t\t\t\t\t\t\t\t\t\t<tr>\n" );
    util_t::fprintf ( file,
                      "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Executes</th>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Direct Results</th>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Ticks</th>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Tick Results</th>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Execute Time per Execution</th>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Tick Time per  Tick</th>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Actual Amount</th>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Total Amount</th>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Overkill %%</th>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Amount per Total Time</th>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Amount per Total Execute Time</th>\n" );
    util_t::fprintf ( file,
                      "\t\t\t\t\t\t\t\t\t\t</tr>\n"
                      "\t\t\t\t\t\t\t\t\t\t<tr>\n" );
    util_t::fprintf ( file,
                      "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.2f</td>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.2f</td>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.2f</td>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.2f</td>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.4f</td>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.4f</td>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.2f</td>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.2f</td>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.2f</td>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.2f</td>\n"
                      "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.2f</td>\n",
                      s -> num_executes,
                      s -> num_direct_results,
                      s -> num_ticks,
                      s -> num_tick_results,
                      s -> etpe,
                      s -> ttpt,
                      s -> actual_amount,
                      s -> total_amount,
                      s -> overkill_pct,
                      s -> aps,
                      s -> apet );
    util_t::fprintf ( file,
                      "\t\t\t\t\t\t\t\t\t\t</tr>\n"
                      "\t\t\t\t\t\t\t\t\t</table>\n" );


    util_t::fprintf ( file,
                      "\t\t\t\t\t\t\t\t\t<table  class=\"details\">\n" );
    if ( s -> num_direct_results > 0 )
    {
      // Direct Damage
      util_t::fprintf ( file,
                        "\t\t\t\t\t\t\t\t\t\t<tr>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Direct Results</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Count</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Pct</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Average</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Min</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Max</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Total Amount</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Total Amount</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Overkill %%</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t</tr>\n" );
      for ( int i=RESULT_MAX-1; i >= RESULT_NONE; i-- )
      {
        if ( s -> direct_results[ i  ].count )
        {
          util_t::fprintf( file,
                           "\t\t\t\t\t\t\t\t\t\t<tr" );
          if ( i & 1 )
          {
            util_t::fprintf( file, " class=\"odd\"" );
          }
          util_t::fprintf( file, ">\n" );
          util_t::fprintf ( file,
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"left small\">%s</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.2f</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.2f%%</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.2f</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.2f</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t</tr>\n",
                            util_t::result_type_string( i ),
                            s -> direct_results[ i  ].count,
                            s -> direct_results[ i  ].pct,
                            s -> direct_results[ i  ].avg_amount,
                            s -> direct_results[ i  ].min_amount,
                            s -> direct_results[ i  ].max_amount,
                            s -> direct_results[ i  ].actual_amount,
                            s -> direct_results[ i  ].total_amount,
                            s -> direct_results[ i  ].overkill_pct );
        }
      }

    }

    if ( s -> num_tick_results > 0 )
    {
      // Tick Damage
      util_t::fprintf ( file,
                        "\t\t\t\t\t\t\t\t\t\t<tr>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Tick Results</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Count</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Pct</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Average</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Min</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Max</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Actual Amount</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Total Amount</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<th class=\"small\">Overkill %%</th>\n"
                        "\t\t\t\t\t\t\t\t\t\t</tr>\n" );
      for ( int i=RESULT_MAX-1; i >= RESULT_NONE; i-- )
      {
        if ( s -> tick_results[ i  ].count )
        {
          util_t::fprintf( file,
                           "\t\t\t\t\t\t\t\t\t\t<tr" );
          if ( i & 1 )
          {
            util_t::fprintf( file, " class=\"odd\"" );
          }
          util_t::fprintf( file, ">\n" );
          util_t::fprintf ( file,
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"left small\">%s</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.1f</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.2f%%</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.2f</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.0f</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t\t<td class=\"right small\">%.2f</td>\n"
                            "\t\t\t\t\t\t\t\t\t\t</tr>\n",
                            util_t::result_type_string( i ),
                            s -> tick_results[ i  ].count,
                            s -> tick_results[ i  ].pct,
                            s -> tick_results[ i  ].avg_amount,
                            s -> tick_results[ i  ].min_amount,
                            s -> tick_results[ i  ].max_amount,
                            s -> tick_results[ i  ].actual_amount,
                            s -> tick_results[ i  ].total_amount,
                            s -> tick_results[ i  ].overkill_pct );
        }
      }


    }
    util_t::fprintf ( file,
                      "\t\t\t\t\t\t\t\t\t</table>\n" );

    util_t::fprintf ( file,
                      "\t\t\t\t\t\t\t\t\t<div class=\"clear\"></div>\n" );

    util_t::fprintf ( file,
                      "\t\t\t\t\t\t\t\t\t%s\n",
                      timeline_stat_aps_str.c_str() );

    util_t::fprintf ( file,
                      "\t\t\t\t\t\t\t\t\t<div class=\"clear\"></div>\n" );
    // Action Details
    std::vector<std::string> processed_actions;
    size_t size = s -> action_list.size();
    for ( size_t i = 0; i < size; i++ )
    {
      action_t* a = s -> action_list[ i ];

      bool found = false;
      size_t size = processed_actions.size();
      for ( size_t j = 0; j < size && !found; j++ )
        if ( processed_actions[ j ] == a -> name() )
          found = true;
      if( found ) continue;
      processed_actions.push_back( a -> name() );

      util_t::fprintf ( file,
                        "\t\t\t\t\t\t\t\t\t<h4>Action details: %s </h4>\n", a -> name() );
      util_t::fprintf ( file,
                        "\t\t\t\t\t\t\t\t\t<div class=\"float\">\n"
                        "\t\t\t\t\t\t\t\t\t\t<h5>Static Values</h5>\n"
                        "\t\t\t\t\t\t\t\t\t\t<ul>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">id:</span>%i</li>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">school:</span>%s</li>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">resource:</span>%s</li>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">tree:</span>%s</li>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">range:</span>%.1f</li>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">travel_speed:</span>%.4f</li>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">trigger_gcd:</span>%.4f</li>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">base_cost:</span>%.1f</li>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">cooldown:</span>%.2f</li>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">base_execute_time:</span>%.2f</li>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">base_crit:</span>%.2f</li>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">target:</span>%s</li>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">tooltip:</span><span class=\"tooltip\">%s</span></li>\n"
                        "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">description:</span><span class=\"tooltip\">%s</span></li>\n"
                        "\t\t\t\t\t\t\t\t\t\t</ul>\n"
                        "\t\t\t\t\t\t\t\t\t</div>\n"
                        "\t\t\t\t\t\t\t\t\t<div class=\"float\">\n",
                        a -> id,
                        util_t::school_type_string( a-> school ),
                        util_t::resource_type_string( a -> resource ),
                        util_t::talent_tree_string( a -> tree ),
                        a -> range,
                        a -> travel_speed,
                        a -> trigger_gcd,
                        a -> base_cost,
                        a -> cooldown -> duration,
                        a -> base_execute_time,
                        a -> base_crit,
                        a -> target ? a -> target -> name() : "",
                        a -> tooltip(),
                        report_t::encode_html( a -> desc() ).c_str() );
      if( a -> direct_power_mod || a -> base_dd_min || a -> base_dd_max )
      {
        util_t::fprintf ( file,
                          "\t\t\t\t\t\t\t\t\t\t<h5>Direct Damage</h5>\n"
                          "\t\t\t\t\t\t\t\t\t\t<ul>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">may_crit:</span>%s</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">direct_power_mod:</span>%.6f</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">base_dd_min:</span>%.2f</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">base_dd_max:</span>%.2f</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t</ul>\n",
                          a -> may_crit?"true":"false",
                          a -> direct_power_mod,
                          a -> base_dd_min,
                          a -> base_dd_max );
      }
      if( a -> num_ticks )
      {
        util_t::fprintf ( file,
                          "\t\t\t\t\t\t\t\t\t\t<h5>Damage Over Time</h5>\n"
                          "\t\t\t\t\t\t\t\t\t\t<ul>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">tick_may_crit:</span>%s</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">tick_zero:</span>%s</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">tick_power_mod:</span>%.6f</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">base_td:</span>%.2f</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">num_ticks:</span>%i</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">base_tick_time:</span>%.2f</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">hasted_ticks:</span>%s</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">dot_behavior:</span>%s</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t</ul>\n",
                          a -> tick_may_crit?"true":"false",
                          a -> tick_zero?"true":"false",
                          a -> tick_power_mod,
                          a -> base_td,
                          a -> num_ticks,
                          a -> base_tick_time,
                          a -> hasted_ticks?"true":"false",
                          a -> dot_behavior==DOT_REFRESH?"DOT_REFRESH":a -> dot_behavior==DOT_CLIP?"DOT_CLIP":"DOT_WAIT" );
      }
      // Extra Reporting for DKs
      if ( a -> player -> type == DEATH_KNIGHT )
      {
        util_t::fprintf ( file,
                          "\t\t\t\t\t\t\t\t\t\t<h5>Rune Information</h5>\n"
                          "\t\t\t\t\t\t\t\t\t\t<ul>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">Blood Cost:</span>%d</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">Frost Cost:</span>%d</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">Unholy Cost:</span>%d</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">Runic Power Gain:</span>%.2f</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t</ul>\n",
                          a -> rune_cost() & 0x1,
                          ( a -> rune_cost() >> 4 ) & 0x1,
                          ( a -> rune_cost() >> 2 ) & 0x1,
                          a -> rp_gain );
      }
      if( a -> weapon )
      {
        util_t::fprintf ( file,
                          "\t\t\t\t\t\t\t\t\t\t<h5>Weapon</h5>\n"
                          "\t\t\t\t\t\t\t\t\t\t<ul>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">normalized:</span>%s</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">weapon_power_mod:</span>%.6f</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">weapon_multiplier:</span>%.2f</li>\n"
                          "\t\t\t\t\t\t\t\t\t\t</ul>\n",
                          a -> normalize_weapon_speed ? "true" : "false",
                          a -> weapon_power_mod,
                          a -> weapon_multiplier );
      }
      util_t::fprintf ( file,
                        "\t\t\t\t\t\t\t\t\t</div>\n"
                        "\t\t\t\t\t\t\t\t\t<div class=\"clear\"></div>\n" );
    }


    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t</td>\n"
                     "\t\t\t\t\t\t\t</tr>\n" );
  }
}

// print_html_action_resource ===============================================

static void print_html_action_resource( FILE* file, stats_t* s, int j )
{

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t<tr" );
  if ( j & 1 )
  {
    util_t::fprintf( file, " class=\"odd\"" );
  }
  util_t::fprintf( file, ">\n" );

  for ( action_t* a = s -> player -> action_list; a; a = a -> next )
  {
    if ( a -> stats != s ) continue;
    if ( ! a -> background ) break;
  }

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<td class=\"left small\">%s</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"left small\">%s</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.1f%%</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.1f</td>\n"
                   "\t\t\t\t\t\t\t\t<td class=\"right small\">%.1f</td>\n"
                   "\t\t\t\t\t\t\t</tr>\n",
                   s -> name_str.c_str(),
                   util_t::resource_type_string( s -> resource ),
                   s -> resource_portion * 100,
                   s -> apr,
                   s -> rpe );
}

// print_html_gear ==========================================================

static void print_html_gear ( FILE* file, player_t* a )
{
  if ( a -> total_seconds > 0 )
  {
    util_t::fprintf( file,
                     "\t\t\t\t\t\t<div class=\"player-section gear\">\n"
                     "\t\t\t\t\t\t\t<h3 class=\"toggle\">Gear</h3>\n"
                     "\t\t\t\t\t\t\t<div class=\"toggle-content hide\">\n"
                     "\t\t\t\t\t\t\t\t<table class=\"sc\">\n"
                     "\t\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th></th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th>Encoded</th>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n" );

    for ( int i=0; i < SLOT_MAX; i++ )
    {
      item_t& item = a -> items[ i ];

      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t\t<tr>\n"
                       "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">%s</th>\n"
                       "\t\t\t\t\t\t\t\t\t\t<td class=\"left\">%s</td>\n"
                       "\t\t\t\t\t\t\t\t\t</tr>\n",
                       item.slot_name(),
                       item.active() ? item.options_str.c_str() : "empty" );
    }

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t</table>\n"
                     "\t\t\t\t\t\t\t</div>\n"
                     "\t\t\t\t\t\t</div>\n" );
  }
}

// print_html_profile =======================================================

static void print_html_profile ( FILE* file, player_t* a )
{
  if ( a -> total_seconds > 0 )
  {
    std::string profile_str;
    a -> create_profile( profile_str, SAVE_ALL, true );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t<div class=\"player-section profile\">\n"
                     "\t\t\t\t\t\t\t<h3 class=\"toggle\">Profile</h3>\n"
                     "\t\t\t\t\t\t\t<div class=\"toggle-content hide\">\n"
                     "\t\t\t\t\t\t\t\t<div class=\"subsection force-wrap\">\n"
                     "\t\t\t\t\t\t\t\t\t<p>%s</p>\n"
                     "\t\t\t\t\t\t\t\t</div>\n"
                     "\t\t\t\t\t\t\t</div>\n"
                     "\t\t\t\t\t\t</div>\n",
                     profile_str.c_str() );
  }
}


// print_html_stats =========================================================

static void print_html_stats ( FILE* file, player_t* a )
{
  std::string n = a -> name();
  util_t::format_text( n, true );

  if ( a -> total_seconds > 0 )
  {
    util_t::fprintf( file,
                     "\t\t\t\t\t\t<div class=\"player-section stats\">\n"
                     "\t\t\t\t\t\t\t<h3 class=\"toggle\">Stats</h3>\n"
                     "\t\t\t\t\t\t\t<div class=\"toggle-content hide\">\n"
                     "\t\t\t\t\t\t\t\t<table class=\"sc\">\n"
                     "\t\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th></th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th>Raid-Buffed</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th>Unbuffed</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th>Gear Amount</th>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n" );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Strength</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     a -> attribute_buffed[ ATTR_STRENGTH  ],
                     a -> strength(),
                     a -> stats.attribute[ ATTR_STRENGTH  ] );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr class=\"odd\">\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Agility</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     a -> attribute_buffed[ ATTR_AGILITY   ],
                     a -> agility(),
                     a -> stats.attribute[ ATTR_AGILITY   ] );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Stamina</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     a -> attribute_buffed[ ATTR_STAMINA   ],
                     a -> stamina(),
                     a -> stats.attribute[ ATTR_STAMINA   ] );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr class=\"odd\">\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Intellect</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     a -> attribute_buffed[ ATTR_INTELLECT ],
                     a -> intellect(),
                     a -> stats.attribute[ ATTR_INTELLECT ] );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Spirit</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     a -> attribute_buffed[ ATTR_SPIRIT    ],
                     a -> spirit(),
                     a -> stats.attribute[ ATTR_SPIRIT    ] );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr class=\"odd\">\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Health</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     a -> resource_buffed[ RESOURCE_HEALTH ],
                     a -> resource_max[ RESOURCE_HEALTH ],
                     0.0 );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Mana</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     a -> resource_buffed[ RESOURCE_MANA   ],
                     a -> resource_max[ RESOURCE_MANA   ],
                     0.0 );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr class=\"odd\">\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Spell Power</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     a -> buffed_spell_power,
                     a -> composite_spell_power( SCHOOL_MAX ) * a -> composite_spell_power_multiplier(),
                     a -> stats.spell_power );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Spell Hit</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     100 * a -> buffed_spell_hit,
                     100 * a -> composite_spell_hit(),
                     a -> stats.hit_rating  );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr class=\"odd\">\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Spell Crit</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     100 * a -> buffed_spell_crit,
                     100 * a -> composite_spell_crit(),
                     a -> stats.crit_rating );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Spell Haste</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     100 * ( 1 / a -> buffed_spell_haste - 1 ),
                     100 * ( 1 / a -> composite_spell_haste() - 1 ),
                     a -> stats.haste_rating );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr class=\"odd\">\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Spell Penetration</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     100 * a -> buffed_spell_penetration,
                     100 * a -> composite_spell_penetration(),
                     a -> stats.spell_penetration );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Mana Per 5</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     a -> buffed_mp5,
                     a -> composite_mp5(),
                     a -> stats.mp5 );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr class=\"odd\">\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Attack Power</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     a -> buffed_attack_power,
                     a -> composite_attack_power() * a -> composite_attack_power_multiplier(),
                     a -> stats.attack_power );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Melee Hit</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     100 * a -> buffed_attack_hit,
                     100 * a -> composite_attack_hit(),
                     a -> stats.hit_rating );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr class=\"odd\">\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Melee Crit</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     100 * a -> buffed_attack_crit,
                     100 * a -> composite_attack_crit(),
                     a -> stats.crit_rating );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Melee Haste</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     100 * ( 1 / a -> buffed_attack_haste - 1 ),
                     100 * ( 1 / a -> composite_attack_haste() - 1 ),
                     a -> stats.haste_rating );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Swing Speed</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     100 * ( 1 / a -> buffed_attack_speed - 1 ),
                     100 * ( 1 / a -> composite_attack_speed() - 1 ),
                     a -> stats.haste_rating );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr class=\"odd\">\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Expertise</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     100 * a -> buffed_attack_expertise,
                     100 * a -> composite_attack_expertise(),
                     a -> stats.expertise_rating );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Armor</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     a -> buffed_armor,
                     a -> composite_armor(),
                     ( a -> stats.armor + a -> stats.bonus_armor ) );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Tank-Miss</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     100 * a -> buffed_miss,
                     100 * ( a -> composite_tank_miss( SCHOOL_PHYSICAL ) ),
                     0.0  );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr class=\"odd\">\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Tank-Dodge</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     100 * a -> buffed_dodge,
                     100 * ( a -> composite_tank_dodge() - a -> diminished_dodge() ),
                     a -> stats.dodge_rating );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Tank-Parry</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     100 * a -> buffed_parry,
                     100 * ( a -> composite_tank_parry() - a -> diminished_parry() ),
                     a -> stats.parry_rating );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr class=\"odd\">\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Tank-Block</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     100 * a -> buffed_block,
                     100 * a -> composite_tank_block(),
                     a -> stats.block_rating );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Tank-Crit</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     100 * a -> buffed_crit,
                     100 * a -> composite_tank_crit( SCHOOL_PHYSICAL ),
                     0.0 );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t<tr class=\"odd\">\n"
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">Mastery</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"right\">%.0f</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     a -> buffed_mastery,
                     a -> composite_mastery(),
                     a -> stats.mastery_rating );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t</table>\n"
                     "\t\t\t\t\t\t\t</div>\n"
                     "\t\t\t\t\t\t</div>\n" );
  }
}


// print_html_talents_player ================================================

static void print_html_talents( FILE* file, player_t* p )
{
  std::string n = p -> name();
  util_t::format_text( n, true );

  if ( p -> total_seconds > 0 )
  {
    util_t::fprintf( file,
                     "\t\t\t\t\t\t<div class=\"player-section talents\">\n"
                     "\t\t\t\t\t\t\t<h3 class=\"toggle\">Talents</h3>\n"
                     "\t\t\t\t\t\t\t<div class=\"toggle-content hide\">\n" );

    for ( int i = 0; i < MAX_TALENT_TREES; i++ )
    {
      size_t tree_size = p -> talent_trees[ i ].size();

      if ( tree_size == 0 )
        continue;

      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t<div class=\"float\">\n"
                       "\t\t\t\t\t\t\t\t\t<table class=\"sc\">\n"
                       "\t\t\t\t\t\t\t\t\t\t<tr>\n"
                       "\t\t\t\t\t\t\t\t\t\t\t<th class=\"left\">%s</th>\n"
                       "\t\t\t\t\t\t\t\t\t\t\t<th>Rank</th>\n"
                       "\t\t\t\t\t\t\t\t\t\t</tr>\n",
                       util_t::talent_tree_string( p -> tree_type[ i ], false ) );



      for ( size_t j=0; j < tree_size; j++ )
      {
        talent_t* t = p -> talent_trees[ i ][ j ];

        util_t::fprintf( file, "\t\t\t\t\t\t\t\t\t\t<tr%s>\n", ( ( j&1 ) ? " class=\"odd\"" : "" ) );
        util_t::fprintf( file, "\t\t\t\t\t\t\t\t\t\t\t<td class=\"left\">%s</td>\n", t -> t_data -> name_cstr() );
        util_t::fprintf( file, "\t\t\t\t\t\t\t\t\t\t\t<td>%d</td>\n", t -> rank() );
        util_t::fprintf( file, "\t\t\t\t\t\t\t\t\t\t</tr>\n" );
      }
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t\t</table>\n"
                       "\t\t\t\t\t\t\t\t</div>\n" );
    }

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t<div class=\"clear\"></div>\n" );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t</div>\n"
                     "\t\t\t\t\t\t</div>\n" );
  }
}


// print_html_player_scale_factors ==========================================

static void print_html_player_scale_factors( FILE* file, sim_t* sim, player_t* p )
{

  if ( !p -> is_pet() )
  {
    if ( p -> sim -> scaling -> has_scale_factors() )
    {
      int colspan = 0;
      util_t::fprintf( file,
                       "\t\t\t\t\t\t<table class=\"sc mt\">\n" );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t<tr>\n"
                       "\t\t\t\t\t\t\t\t<th><a href=\"#help-scale-factors\" class=\"help\">?</a></th>\n" );
      for ( int i=0; i < STAT_MAX; i++ )
        if ( p -> scales_with[ i ] )
        {
          util_t::fprintf( file,
                           "\t\t\t\t\t\t\t\t<th>%s</th>\n",
                           util_t::stat_type_abbrev( i ) );
          colspan++;
        }
      if ( p -> sim -> scaling -> scale_lag )
      {
        util_t::fprintf( file,
                         "\t\t\t\t\t\t\t\t<th>ms Lag</th>\n" );
        colspan++;
      }
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t</tr>\n" );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t<tr>\n"
                       "\t\t\t\t\t\t\t\t<th class=\"left\">Scale Factors</th>\n" );
      for ( int i=0; i < STAT_MAX; i++ )
        if ( p -> scales_with[ i ] )
          util_t::fprintf( file,
                           "\t\t\t\t\t\t\t\t<td>%.*f</td>\n",
                           p -> sim -> report_precision,
                           p -> scaling.get_stat( i ) );
      if ( p -> sim -> scaling -> scale_lag )
        util_t::fprintf( file,
                         "\t\t\t\t\t\t\t\t<td>%.*f</td>\n",
                         p -> sim -> report_precision,
                         p -> scaling_lag );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t</tr>\n" );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t<tr>\n"
                       "\t\t\t\t\t\t\t\t<th class=\"left\">Normalized</th>\n" );
      for ( int i=0; i < STAT_MAX; i++ )
        if ( p -> scales_with[ i ] )
          util_t::fprintf( file,
                           "\t\t\t\t\t\t\t\t<td>%.*f</td>\n",
                           p -> sim -> report_precision,
                           p -> scaling_normalized.get_stat( i ) );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t</tr>\n" );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t<tr>\n"
                       "\t\t\t\t\t\t\t\t<th class=\"left\">Scale Deltas</th>\n" );
      for ( int i=0; i < STAT_MAX; i++ )
        if ( p -> scales_with[ i ] )
          util_t::fprintf( file,
                           "\t\t\t\t\t\t\t\t<td>%.*f</td>\n",
                           ( i == STAT_WEAPON_OFFHAND_SPEED || i == STAT_WEAPON_SPEED ) ? 2 : 0,
                           p -> sim -> scaling -> stats.get_stat( i ) );
      if ( p -> sim -> scaling -> scale_lag )
        util_t::fprintf( file,
                         "\t\t\t\t\t\t\t\t<td>100</td>\n" );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t</tr>\n" );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t<tr>\n"
                       "\t\t\t\t\t\t\t\t<th class=\"left\">Error</th>\n" );
      for ( int i=0; i < STAT_MAX; i++ )
        if ( p -> scales_with[ i ] )
          util_t::fprintf( file,
                           "\t\t\t\t\t\t\t\t<td>%.*f</td>\n",
                           p -> sim -> report_precision,
                           p -> scaling_error.get_stat( i ) );
      if ( p -> sim -> scaling -> scale_lag )
              util_t::fprintf( file,
                               "\t\t\t\t\t\t\t\t<td>%.*f</td>\n",
                               p -> sim -> report_precision,
                               p -> scaling_lag_error );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t</tr>\n" );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t<tr class=\"left\">\n"
                       "\t\t\t\t\t\t\t\t<th>Gear Ranking</th>\n"
                       "\t\t\t\t\t\t\t\t<td colspan=\"%i\" class=\"filler\">\n"
                       "\t\t\t\t\t\t\t\t\t<ul class=\"float\">\n"
                       "\t\t\t\t\t\t\t\t\t\t<li><a href=\"%s\" class=\"ext\">wowhead</a></li>\n"
                       "\t\t\t\t\t\t\t\t\t\t<li><a href=\"%s\" class=\"ext\">lootrank</a></li>\n"
                       "\t\t\t\t\t\t\t\t\t</ul>\n"
                       "\t\t\t\t\t\t\t\t</td>\n"
                       "\t\t\t\t\t\t\t</tr>\n",
                       colspan,
                       p -> gear_weights_wowhead_link.c_str(),
                       p -> gear_weights_lootrank_link.c_str() );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t<tr class=\"left\">\n"
                       "\t\t\t\t\t\t\t\t<th>Optimizers</th>\n"
                       "\t\t\t\t\t\t\t\t<td colspan=\"%i\" class=\"filler\">\n"
                       "\t\t\t\t\t\t\t\t\t<ul class=\"float\">\n"
                       "\t\t\t\t\t\t\t\t\t\t<li><a href=\"%s\" class=\"ext\">wowreforge</a></li>\n"
                       "\t\t\t\t\t\t\t\t\t</ul>\n"
                       "\t\t\t\t\t\t\t\t</td>\n"
                       "\t\t\t\t\t\t\t</tr>\n",
                       colspan,
                       p -> gear_weights_wowreforge_link.c_str() );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t<tr class=\"left\">\n"
                       "\t\t\t\t\t\t\t\t<th>Stat Ranking</th>\n"
                       "\t\t\t\t\t\t\t\t<td colspan=\"%i\" class=\"filler\">\n"
                       "\t\t\t\t\t\t\t\t\t<ul class=\"float\">\n"
                       "\t\t\t\t\t\t\t\t\t\t<li>",
                       colspan );
      size_t num_scaling_stats = p -> scaling_stats.size();
      for ( size_t i=0; i < num_scaling_stats; i++ )
      {
        if ( i > 0 )
        {
          if ( ( ( p -> scaling.get_stat( p -> scaling_stats[ i - 1 ] ) - p -> scaling.get_stat( p -> scaling_stats[ i ] ) )
              > sqrt ( p -> scaling_compare_error.get_stat( p -> scaling_stats[ i - 1 ] ) * p -> scaling_compare_error.get_stat( p -> scaling_stats[ i - 1 ] ) / 4 + p -> scaling_compare_error.get_stat( p -> scaling_stats[ i ] ) * p -> scaling_compare_error.get_stat( p -> scaling_stats[ i ] ) / 4 ) * 2 ) )
            util_t::fprintf( file, " > " );
          else
            util_t::fprintf( file, " = " );
        }

        util_t::fprintf( file, "%s", util_t::stat_type_abbrev( p -> scaling_stats[ i ] ) );

      }
      util_t::fprintf( file, "</li>\n"
                       "\t\t\t\t\t\t\t\t\t</ul>\n"
                       "\t\t\t\t\t\t\t\t</td>\n"
                       "\t\t\t\t\t\t\t</tr>\n" );

      util_t::fprintf( file,
                       "\t\t\t\t\t\t</table>\n" );
      if ( sim -> iterations < 10000 )
        util_t::fprintf( file,
                         "\t\t\t\t<div class=\"alert\">\n"
                         "\t\t\t\t\t<h3>Warning</h3>\n"
                         "\t\t\t\t\t<p>Scale Factors generated using less than 10,000 iterations will vary significantly from run to run.</p>\n"
                         "\t\t\t\t</div>\n" );
    }
  }
  util_t::fprintf( file,
                   "\t\t\t\t\t</div>\n"
                   "\t\t\t\t</div>\n" );
}

// print_html_player_action_priority_list ===================================

static void print_html_player_action_priority_list( FILE* file, sim_t* sim, player_t* p )
{

  util_t::fprintf( file,
                   "\t\t\t\t\t\t<div class=\"player-section action-priority-list\">\n"
                   "\t\t\t\t\t\t\t<h3 class=\"toggle\">Action Priority List</h3>\n"
                   "\t\t\t\t\t\t\t<div class=\"toggle-content hide\">\n"
                   "\t\t\t\t\t\t\t\t<table class=\"sc\">\n"
                   "\t\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t\t<th class=\"right\">#</th>\n"
                   "\t\t\t\t\t\t\t\t\t\t<th class=\"left\">action,conditions</th>\n"
                   "\t\t\t\t\t\t\t\t\t</tr>\n" );
  int i = 1;
  for ( action_t* a = p -> action_list; a; a = a -> next )
  {
    if ( a -> signature_str.empty() || ! a -> marker ) continue;
    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t<tr" );
    if ( !( i & 1 ) )
    {
      util_t::fprintf( file, " class=\"odd\"" );
    }
    util_t::fprintf( file, ">\n" );
    std::string enc_action = a -> signature_str; report_t::encode_html( enc_action );
    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t\t\t<th class=\"right\">%c</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td class=\"left\">%s</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n",
                     a -> marker,
                     enc_action.c_str() );
    i++;
  }
  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t</table>\n" );

  if( ! p -> action_sequence.empty() )
  {
    std::string& seq = p -> action_sequence;
    if ( seq.size() > 0 )
    {
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t<div class=\"subsection subsection-small\">\n"
                       "\t\t\t\t\t\t\t\t\t<h4>Sample Sequence</h4>\n"
                       "\t\t\t\t\t\t\t\t\t<div class=\"force-wrap mono\">\n"
                       "                    %s\n"
                       "\t\t\t\t\t\t\t\t\t</div>\n"
                       "\t\t\t\t\t\t\t\t</div>\n",
                       seq.c_str() );

      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t<div class=\"subsection subsection-small\">\n"
                       "\t\t\t\t\t\t\t\t\t<h4>Labels</h4>\n"
                       "\t\t\t\t\t\t\t\t\t<div class=\"force-wrap mono\">\n"
                       "                    %s\n"
                       "\t\t\t\t\t\t\t\t\t</div>\n"
                       "\t\t\t\t\t\t\t\t</div>\n",
                       p -> print_action_map( sim -> iterations, 2 ).c_str() );
    }
  }

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t</div>\n"
                   "\t\t\t\t\t\t</div>\n" );

}

// print_html_player_statistics =============================================

static void print_html_player_statistics( FILE* file, player_t* p )
{

// Statistics & Data Analysis

  util_t::fprintf( file,
                   "\t\t\t\t\t<div class=\"player-section gains\">\n"
                   "\t\t\t\t\t\t<h3 class=\"toggle\">Statistics & Data Analysis</h3>\n"
                   "\t\t\t\t\t\t<div class=\"toggle-content hide\">\n"
                   "\t\t\t\t\t\t\t<table class=\"sc\">\n"
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<th>DPS</th>\n"
                   "\t\t\t\t\t\t\t\t\t<th></th>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n" );


  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\"><b>Sample Data</b></td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\"></td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n" );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">Average</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   p -> dps );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">Standard Deviation</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.4f</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   p -> dps_std_dev );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">Minimum</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   p -> dps_min );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">Maximum</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   p -> dps_max );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">Spread ( max - min )</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   p -> dps_max - p -> dps_min );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">Range [ ( max - min ) / 2 * 100%% ]</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   p -> dps ? ( ( p -> dps_max - p -> dps_min ) / 2 ) * 100 / p -> dps : 0 );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">10th Percentile</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   p -> dps_10_percentile );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">90th Percentile</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   p -> dps_90_percentile );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">( 90th Percentile - 10th Percentile )</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   p -> dps_90_percentile - p -> dps_10_percentile );


  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\"><b>Population</b></td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\"></td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n" );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">Standard Deviation of the Average DPS(e)</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.4f</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   p -> dps_std_dev / sqrt( ( float ) p -> sim -> iterations ));

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">%.2f%% Confidence Intervall</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">( %.2f - %.2f )</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   p -> sim -> confidence * 100.0, p -> dps - p -> dps_error, p -> dps + p -> dps_error );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">Normalized %.2f%% Confidence Intervall</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">( %.2f%% - %.2f%% )</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   p -> sim -> confidence * 100.0, p -> dps ? 100 - p -> dps_error * 100 / p -> dps : 0, p -> dps ? 100 + p -> dps_error * 100 / p -> dps : 0 );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\"><b>Approx. Iterations needed for ( always use n>=50 )</b></td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\"></td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n" );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">1%% DPS Error</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">%i</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   ( int ) ( p -> dps ? ( ( p -> dps_error * p -> dps_error * ( ( float ) p -> sim -> iterations ) / ( 0.01 * p -> dps * 0.01 * p -> dps) ) ) : 0 ) );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">0.1%% DPS Error</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">%i</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   ( int ) ( p -> dps ? ( ( p -> dps_error * p -> dps_error * ( ( float ) p -> sim -> iterations ) / ( 0.001 * p -> dps * 0.001 * p -> dps ) ) ) : 0 ) );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">0.1 Scale Factor Error with Delta=300</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">%i</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   ( int ) ( 2.0 * p -> dps_error * p -> dps_error * ( ( float ) p -> sim -> iterations ) / ( 30 * 30 ) ) );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">0.05 Scale Factor Error with Delta=300</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">%i</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   ( int ) ( 2.0 * p -> dps_error * p -> dps_error * ( ( float ) p -> sim -> iterations ) / ( 15 * 15 ) ) );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"left\">0.01 Scale Factor Error with Delta=300</td>\n"
                   "\t\t\t\t\t\t\t\t\t<td class=\"right\">%i</td>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n",
                   ( int ) (  2.0 * p -> dps_error * p -> dps_error * ( ( float ) p -> sim -> iterations ) / ( 3 * 3) ) );


  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t\t</table>\n" );

  std::string timeline_dps_error_str           = "";
  std::string dps_error_str                    = "";

  char buffer[ 1024 ];

  if ( ! p -> timeline_dps_error_chart.empty() )
  {
    snprintf( buffer, sizeof( buffer ), "<img src=\"%s\" alt=\"Timeline DPS Error Chart\" />\n", p -> timeline_dps_error_chart.c_str() );
    timeline_dps_error_str = buffer;
  }

  util_t::fprintf( file,
                   "%s\n",
                   timeline_dps_error_str.c_str() );

  if ( ! p -> dps_error_chart.empty() )
  {
    snprintf( buffer, sizeof( buffer ), "<img src=\"%s\" alt=\"DPS Error Chart\" />\n", p -> dps_error_chart.c_str() );
    dps_error_str = buffer;
  }
  util_t::fprintf( file,
                   "%s\n"
                   "\t\t\t\t\t\t\t</div>\n"
                   "\t\t\t\t\t\t</div>\n",
                   dps_error_str.c_str() );
}

// print_html_player_resources ==============================================

static void print_html_player_resources( FILE* file, player_t* p )
{
// Resources Section

  util_t::fprintf( file,
                   "\t\t\t\t<div class=\"player-section gains\">\n"
                   "\t\t\t\t\t<h3 class=\"toggle\">Resources</h3>\n"
                   "\t\t\t\t\t<div class=\"toggle-content hide\">\n"
                   "\t\t\t\t\t\t<table class=\"sc mt\">\n"
                   "\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"left small\">Resource Usage</th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\">Type</th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\">Res%%</th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-dpr\" class=\"help\">DPR</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\">RPE</th>\n"
                   "\t\t\t\t\t\t\t</tr>\n" );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"left small\">%s</th>\n"
                   "\t\t\t\t\t\t\t\t<td colspan=\"4\" class=\"filler\"></td>\n"
                   "\t\t\t\t\t\t\t</tr>\n",
                   p -> name() );

  int i = 0;
  for ( stats_t* s = p -> stats_list; s; s = s -> next )
  {
    if ( s -> rpe > 0 )
    {
      print_html_action_resource( file, s, i );
      i++;
    }
  }

  for ( pet_t* pet = p -> pet_list; pet; pet = pet -> next_pet )
  {
    bool first=true;

    i = 0;
    for ( stats_t* s = pet -> stats_list; s; s = s -> next )
    {
      if ( s -> rpe > 0 )
      {
        if ( first )
        {
          first = false;
          util_t::fprintf( file,
                           "\t\t\t\t\t\t\t<tr>\n"
                           "\t\t\t\t\t\t\t\t<th class=\"left small\">pet - %s</th>\n"
                           "\t\t\t\t\t\t\t\t<td colspan=\"4\" class=\"filler\"></td>\n"
                           "\t\t\t\t\t\t\t</tr>\n",
                           pet -> name_str.c_str() );
        }
        print_html_action_resource( file, s, i );
        i++;
      }
    }
  }

  util_t::fprintf( file,
                   "\t\t\t\t\t\t</table>\n" );

// Resource Gains Section
  util_t::fprintf( file,

                   "\t\t\t\t\t\t<table class=\"sc\">\n"
                   "\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t<th>Resource Gains</th>\n"
                   "\t\t\t\t\t\t\t\t<th>Type</th>\n"
                   "\t\t\t\t\t\t\t\t<th>Count</th>\n"
                   "\t\t\t\t\t\t\t\t<th>Total</th>\n"
                   "\t\t\t\t\t\t\t\t<th>Average</th>\n"
                   "\t\t\t\t\t\t\t\t<th colspan=\"2\">Overflow</th>\n"
                   "\t\t\t\t\t\t\t</tr>\n" );
  i = 1;
  for ( gain_t* g = p -> gain_list; g; g = g -> next )
  {
    if ( g -> actual > 0 || g -> overflow > 0 )
    {
      double overflow_pct = 100.0 * g -> overflow / ( g -> actual + g -> overflow );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t<tr" );
      if ( !( i & 1 ) )
      {
        util_t::fprintf( file, " class=\"odd\"" );
      }
      util_t::fprintf( file, ">\n" );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t<td class=\"left\">%s</td>\n"
                       "\t\t\t\t\t\t\t\t<td class=\"left\">%s</td>\n"
                       "\t\t\t\t\t\t\t\t<td class=\"right\">%.1f</td>\n"
                       "\t\t\t\t\t\t\t\t<td class=\"right\">%.1f</td>\n"
                       "\t\t\t\t\t\t\t\t<td class=\"right\">%.1f</td>\n"
                       "\t\t\t\t\t\t\t\t<td class=\"right\">%.1f</td>\n"
                       "\t\t\t\t\t\t\t\t<td class=\"right\">%.1f%%</td>\n"
                       "\t\t\t\t\t\t\t</tr>\n",
                       g -> name(),
                       util_t::resource_type_string( g -> type ),
                       g -> count,
                       g -> actual,
                       g -> actual / g -> count,
                       g -> overflow,
                       overflow_pct );
      i++;
    }
  }
  for ( pet_t* pet = p -> pet_list; pet; pet = pet -> next_pet )
  {
    if ( pet -> total_seconds <= 0 ) continue;
    bool first = true;
    for ( gain_t* g = pet -> gain_list; g; g = g -> next )
    {
      if ( g -> actual > 0 || g -> overflow > 0 )
      {
        if ( first )
        {
          first = false;
          util_t::fprintf( file,
                           "\t\t\t\t\t\t\t<tr>\n"
                           "\t\t\t\t\t\t\t\t<th>pet - %s</th>\n"
                           "\t\t\t\t\t\t\t\t<td colspan=\"6\" class=\"filler\"></td>\n"
                           "\t\t\t\t\t\t\t</tr>\n",
                           pet -> name_str.c_str() );
        }
        double overflow_pct = 100.0 * g -> overflow / ( g -> actual + g -> overflow );
        util_t::fprintf( file,
                         "\t\t\t\t\t\t\t<tr>\n"
                         "\t\t\t\t\t\t\t\t<td class=\"left\">%s</td>\n"
                         "\t\t\t\t\t\t\t\t<td class=\"left\">%s</td>\n"
                         "\t\t\t\t\t\t\t\t<td class=\"right\">%.1f</td>\n"
                         "\t\t\t\t\t\t\t\t<td class=\"right\">%.1f</td>\n"
                         "\t\t\t\t\t\t\t\t<td class=\"right\">%.1f</td>\n"
                         "\t\t\t\t\t\t\t\t<td class=\"right\">%.1f</td>\n"
                         "\t\t\t\t\t\t\t\t<td class=\"right\">%.1f%%</td>\n"
                         "\t\t\t\t\t\t\t</tr>\n",
                         g -> name(),
                         util_t::resource_type_string( g -> type ),
                         g -> count,
                         g -> actual,
                         g -> actual / g -> count,
                         g -> overflow,
                         overflow_pct );
      }
    }
  }
  util_t::fprintf( file,
                   "\t\t\t\t\t\t</table>\n" );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t<div class=\"charts charts-left\">\n" );
  for ( i = RESOURCE_NONE; i < RESOURCE_MAX; ++i )
  {
    // hack hack. don't display RESOURCE_RUNE_<TYPE> yet. only shown in tabular data.  WiP
    if ( i == RESOURCE_RUNE_BLOOD || i == RESOURCE_RUNE_UNHOLY || i == RESOURCE_RUNE_FROST ) continue;
    double total_gain=0;
    for ( gain_t* g = p -> gain_list; g; g = g -> next )
    {
      if ( g -> actual > 0 && g -> type == i )
        total_gain += g -> actual;
    }

    if ( total_gain > 0 )
    {
      chart_t::gains( p -> gains_chart, p, ( resource_type ) i );
      if ( ! p -> gains_chart.empty() )
      {
        util_t::fprintf( file,
                         "\t\t\t\t\t\t\t<img src=\"%s\" alt=\"Resource Gains Chart\" />\n",
                         p -> gains_chart.c_str() );
      }
    }
  }
  util_t::fprintf( file,
                   "\t\t\t\t\t\t</div>\n" );


  util_t::fprintf( file,
                   "\t\t\t\t\t\t<div class=\"charts\">\n" );
  for ( int i = RESOURCE_NONE + 1; i < RESOURCE_MAX; i++ )
  {
    if ( p -> resource_max[ i ] > 0 && ! p -> timeline_resource_chart[ i ].empty() )
    {
      util_t::fprintf( file,
                       "\t\t\t\t\t\t<img src=\"%s\" alt=\"Resource Timeline Chart\" />\n",
                       p -> timeline_resource_chart[ i ].c_str() );
    }
  }
  util_t::fprintf( file,
                   "\t\t\t\t\t\t</div>\n"
                   "\t\t\t\t\t<div class=\"clear\"></div>\n" );

  util_t::fprintf( file,
                   "\t\t\t\t\t</div>\n"
                   "\t\t\t\t</div>\n" );
}

// print_html_player_charts =================================================

static void print_html_player_charts( FILE* file, sim_t* sim, player_t* p )
{
  const size_t num_players = sim -> players_by_name.size();

  fputs( "\t\t\t\t<div class=\"player-section\">\n"
         "\t\t\t\t\t<h3 class=\"toggle open\">Charts</h3>\n"
         "\t\t\t\t\t<div class=\"toggle-content\">\n"
         "\t\t\t\t\t\t<div class=\"charts charts-left\">\n", file );

  if ( ! p -> action_dpet_chart.empty() )
  {
    const char* fmt;
    if ( num_players == 1 )
      fmt = "\t\t\t\t\t\t\t<img src=\"%s\" alt=\"Action DPET Chart\" />\n";
    else
      fmt = "\t\t\t\t\t\t\t<span class=\"chart-action-dpet\" title=\"Action DPET Chart\">%s</span>\n";
    fprintf( file, fmt, p -> action_dpet_chart.c_str() );
  }

  if ( ! p -> action_dmg_chart.empty() )
  {
    const char* fmt;
    if ( num_players == 1 )
      fmt = "\t\t\t\t\t\t\t<img src=\"%s\" alt=\"Action Damage Chart\" />\n";
    else
      fmt = "\t\t\t\t\t\t\t<span class=\"chart-action-dmg\" title=\"Action Damage Chart\">%s</span>\n";
    fprintf( file, fmt, p -> action_dmg_chart.c_str() );
  }

  if ( ! p -> scaling_dps_chart.empty() )
  {
    const char* fmt;
    if ( num_players == 1 )
      fmt = "\t\t\t\t\t\t\t<img src=\"%s\" alt=\"Scaling DPS Chart\" />\n";
    else
      fmt = "\t\t\t\t\t\t\t<span class=\"chart-scaling-dps\" title=\"Scaling DPS Chart\">%s</span>\n";
    fprintf( file, fmt, p -> scaling_dps_chart.c_str() );
  }

  fputs( "\t\t\t\t\t\t</div>\n"
         "\t\t\t\t\t\t<div class=\"charts\">\n", file );

  if ( ! p -> reforge_dps_chart.empty() )
  {
    const char* fmt;
    if ( p -> sim -> reforge_plot -> reforge_plot_stat_indices.size() == 2 )
    {
      if ( num_players == 1 )
        fmt = "\t\t\t\t\t\t\t<img src=\"%s\" alt=\"Reforge DPS Chart\" />\n";
      else
        fmt = "\t\t\t\t\t\t\t<span class=\"chart-reforge-dps\" title=\"Reforge DPS Chart\">%s</span>\n";
    }
    else
    {
      if ( true )
        fmt = "\t\t\t\t\t\t\t%s";
      else
      {
        if ( num_players == 1 )
          fmt = "\t\t\t\t\t\t\t<iframe>%s</iframe>\n";
        else
          fmt = "\t\t\t\t\t\t\t<span class=\"chart-reforge-dps\" title=\"Reforge DPS Chart\">%s</span>\n";
      }
    }
    fprintf( file, fmt, p -> reforge_dps_chart.c_str() );
  }

  if ( ! p -> scale_factors_chart.empty() )
  {
    const char* fmt;
    if ( num_players == 1 )
      fmt = "\t\t\t\t\t\t\t<img src=\"%s\" alt=\"Scale Factors Chart\" />\n";
    else
      fmt = "\t\t\t\t\t\t\t<span class=\"chart-scale-factors\" title=\"Scale Factors Chart\">%s</span>\n";
    fprintf( file, fmt, p -> scale_factors_chart.c_str() );
  }

  if ( ! p -> timeline_dps_chart.empty() )
  {
    const char* fmt;
    if ( num_players == 1 )
      fmt = "\t\t\t\t\t\t\t<img src=\"%s\" alt=\"DPS Timeline Chart\" />\n";
    else
      fmt = "\t\t\t\t\t\t\t<span class=\"chart-timeline-dps\" title=\"DPS Timeline Chart\">%s</span>\n";
    fprintf( file, fmt, p -> timeline_dps_chart.c_str() );
  }

  if ( ! p -> distribution_dps_chart.empty() )
  {
    const char* fmt;
    if ( num_players == 1 )
      fmt = "\t\t\t\t\t\t\t<img src=\"%s\" alt=\"DPS Distribution Chart\" />\n";
    else
      fmt = "\t\t\t\t\t\t\t<span class=\"chart-distribution-dps\" title=\"DPS Distribution Chart\">%s</span>\n";
    fprintf( file, fmt, p -> distribution_dps_chart.c_str() );
  }

  if ( ! p -> time_spent_chart.empty() )
  {
    const char* fmt;
    if ( num_players == 1 )
      fmt = "\t\t\t\t\t\t\t<img src=\"%s\" alt=\"Time Spent Chart\" />\n";
    else
      fmt = "\t\t\t\t\t\t\t<span class=\"chart-time-spent\" title=\"Time Spent Chart\">%s</span>\n";
    fprintf( file, fmt, p -> time_spent_chart.c_str() );
  }

  fputs( "\t\t\t\t\t\t</div>\n"
         "\t\t\t\t\t\t<div class=\"clear\"></div>\n"
         "\t\t\t\t\t</div>\n"
         "\t\t\t\t</div>\n", file );
}


// print_html_player_buffs ==================================================

static void print_html_player_buffs( FILE* file, player_t* p )
{
  int i=0;
// Buff Section
  util_t::fprintf( file,
                   "\t\t\t\t<div class=\"player-section buffs\">\n"
                   "\t\t\t\t\t<h3 class=\"toggle open\">Buffs</h3>\n"
                   "\t\t\t\t\t<div class=\"toggle-content\">\n" );

  // Dynamic Buffs table
  util_t::fprintf( file,
                   "\t\t\t\t\t\t<table class=\"sc mb\">\n"
                   "\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"left\"><a href=\"#help-dynamic-buffs\" class=\"help\">Dynamic Buffs</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th>Start</th>\n"
                   "\t\t\t\t\t\t\t\t<th>Refresh</th>\n"
                   "\t\t\t\t\t\t\t\t<th>Interval</th>\n"
                   "\t\t\t\t\t\t\t\t<th>Trigger</th>\n"
                   "\t\t\t\t\t\t\t\t<th>Up-Time</th>\n"
                   "\t\t\t\t\t\t\t\t<th>Benefit</th>\n"
                   "\t\t\t\t\t\t\t</tr>\n" );

  std::vector<buff_t*> dynamic_buffs;
  for ( buff_t* b = p -> buff_list; b; b = b -> next )
    if ( ! b -> quiet && b -> start_count && ! b -> constant )
      dynamic_buffs.push_back( b );
  for ( pet_t* pet = p -> pet_list; pet; pet = pet -> next_pet )
    for ( buff_t* b = pet -> buff_list; b; b = b -> next )
      if ( ! b -> quiet && b -> start_count && ! b -> constant )
        dynamic_buffs.push_back( b );

  for ( i=0; i < ( int ) dynamic_buffs.size(); i++ )
  {
    buff_t* b = dynamic_buffs[ i ];

    std::string buff_name;
    if( b -> player && b -> player -> is_pet() )
    {
      buff_name += b -> player -> name_str + '-';
    }
    buff_name += b -> name();

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t<tr" );
    if ( i & 1 )
    {
      util_t::fprintf( file, " class=\"odd\"" );
    }
    util_t::fprintf( file, ">\n" );
    if ( p -> sim -> report_details )
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t<td class=\"left\"><a href=\"#\" class=\"toggle-details\">%s</a></td>\n",
                       buff_name.c_str() );
    else
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t<td class=\"left\">%s</td>\n",
                       buff_name.c_str() );
    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t<td class=\"right\">%.1f</td>\n"
                     "\t\t\t\t\t\t\t\t<td class=\"right\">%.1f</td>\n"
                     "\t\t\t\t\t\t\t\t<td class=\"right\">%.1fsec</td>\n"
                     "\t\t\t\t\t\t\t\t<td class=\"right\">%.1fsec</td>\n"
                     "\t\t\t\t\t\t\t\t<td class=\"right\">%.0f%%</td>\n"
                     "\t\t\t\t\t\t\t\t<td class=\"right\">%.0f%%</td>\n"
                     "\t\t\t\t\t\t\t</tr>\n",
                     b -> avg_start,
                     b -> avg_refresh,
                     b -> avg_start_interval,
                     b -> avg_trigger_interval,
                     b -> uptime_pct,
                     ( b -> benefit_pct > 0 ? b -> benefit_pct : b -> uptime_pct ) );

    if ( p -> sim -> report_details )
    {
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t<tr class=\"details hide\">\n"
                       "\t\t\t\t\t\t\t\t<td colspan=\"7\" class=\"filler\">\n"
                       "\t\t\t\t\t\t\t\t\t<h4>Database details</h4>\n"
                       "\t\t\t\t\t\t\t\t\t<ul>\n"
                       "\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">id:</span>%.i</li>\n"
                       "\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">cooldown name:</span>%s</li>\n"
                       "\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">tooltip:</span><span class=\"tooltip-wider\">%s</span></li>\n"
                       "\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">max_stacks:</span>%.i</li>\n"
                       "\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">duration:</span>%.2f</li>\n"
                       "\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">cooldown:</span>%.2f</li>\n"
                       "\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">default_chance:</span>%.2f%%</li>\n"
                       "\t\t\t\t\t\t\t\t\t</ul>\n"
                       "\t\t\t\t\t\t\t\t</td>\n",
                       b -> s_id,
                       b -> cooldown -> name_str.c_str(),
                       b -> tooltip(),
                       b -> max_stack,
                       b -> buff_duration,
                       b -> cooldown -> duration,
                       b -> default_chance * 100 );

      util_t::fprintf( file,
      "\t\t\t\t\t\t\t\t<td colspan=\"7\" class=\"filler\">\n"
      "\t\t\t\t\t\t\t\t\t<h4>Stack Uptimes</h4>\n"
      "\t\t\t\t\t\t\t\t\t<ul>\n" );
      for ( unsigned int i= 0; i < b -> stack_uptime.size(); i++ )
      {
        uptime_t* u = b -> stack_uptime[ i ];
        char bn[ 128 ];
        if ( u -> uptime > 0 )
        {
          snprintf( bn, sizeof( bn ), "%s_%d", b -> name(), i );
          util_t::fprintf( file,
                           "\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">%s:</span>%.1f%%</li>\n",
                           bn,
                           u -> uptime * 100.0 );
        }
      }
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t\t</ul>\n"
                       "\t\t\t\t\t\t\t\t</td>\n"
                       "\t\t\t\t\t\t\t</tr>\n" );
    }
  }
  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t</table>\n" );

  // constant buffs
  if ( !p -> is_pet() )
  {
    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t<table class=\"sc\">\n"
                     "\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t<th class=\"left\"><a href=\"#help-constant-buffs\" class=\"help\">Constant Buffs</a></th>\n"
                     "\t\t\t\t\t\t\t\t</tr>\n" );
    i = 1;
    for ( buff_t* b = p -> buff_list; b; b = b -> next )
    {
      if ( b -> quiet || ! b -> start_count || ! b -> constant )
        continue;

      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t<tr" );
      if ( !( i & 1 ) )
      {
        util_t::fprintf( file, " class=\"odd\"" );
      }
      util_t::fprintf( file, ">\n" );
      if ( p -> sim -> report_details )
      {
        util_t::fprintf( file,
                         "\t\t\t\t\t\t\t\t\t<td class=\"left\"><a href=\"#\" class=\"toggle-details\">%s</a></td>\n"
                         "\t\t\t\t\t\t\t\t</tr>\n",
                         b -> name() );


        util_t::fprintf( file,
                         "\t\t\t\t\t\t\t\t<tr class=\"details hide\">\n"
                         "\t\t\t\t\t\t\t\t\t<td>\n"
                         "\t\t\t\t\t\t\t\t\t\t<h4>Database details</h4>\n"
                         "\t\t\t\t\t\t\t\t\t\t<ul>\n"
                         "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">id:</span>%.i</li>\n"
                         "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">cooldown name:</span>%s</li>\n"
                         "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">tooltip:</span><span class=\"tooltip\">%s</span></li>\n"
                         "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">max_stacks:</span>%.i</li>\n"
                         "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">duration:</span>%.2f</li>\n"
                         "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">cooldown:</span>%.2f</li>\n"
                         "\t\t\t\t\t\t\t\t\t\t\t<li><span class=\"label\">default_chance:</span>%.2f%%</li>\n"
                         "\t\t\t\t\t\t\t\t\t\t</ul>\n"
                         "\t\t\t\t\t\t\t\t\t</td>\n"
                         "\t\t\t\t\t\t\t\t</tr>\n",
                         b -> s_id,
                         b -> cooldown -> name_str.c_str(),
                         b -> tooltip(),
                         b -> max_stack,
                         b -> buff_duration,
                         b -> cooldown -> duration,
                         b -> default_chance * 100 );
      }
      else
        util_t::fprintf( file,
                         "\t\t\t\t\t\t\t\t\t<td class=\"left\">%s</td>\n"
                         "\t\t\t\t\t\t\t\t</tr>\n",
                         b -> name() );

      i++;
    }
    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t</table>\n" );
  }


  util_t::fprintf( file,
                   "\t\t\t\t\t\t</div>\n"
                   "\t\t\t\t\t</div>\n" );

}


// print_html_player ========================================================

static void print_html_player_( FILE* file, sim_t* sim, player_t* p, int j )
{
  std::string n = p -> name();
  util_t::format_text( n, true );
  int num_players = ( int ) sim -> players_by_name.size();
  int i;

  util_t::fprintf( file,
                   "\t\t<div id=\"%s\" class=\"player section",
                   n.c_str() );
  if ( num_players > 1 && j == 0 && ! sim -> scaling -> has_scale_factors() && p -> type != ENEMY && p -> type != ENEMY_ADD )
  {
    util_t::fprintf( file, " grouped-first" );
  }
  else if ( ( p -> type == ENEMY || p -> type == ENEMY_ADD ) && j == ( int ) sim -> targets_by_name.size() - 1 )
  {
    util_t::fprintf( file, " final grouped-last" );
  }
  else if ( num_players == 1 )
  {
    util_t::fprintf( file, " section-open" );
  }
  util_t::fprintf( file, "\">\n" );

  if ( ! p -> thumbnail_url.empty()  )
    util_t::fprintf( file,
                     "\t\t\t<a href=\"%s\" class=\"toggle-thumbnail%s\"><img src=\"%s\" alt=\"%s\" class=\"player-thumbnail\"/></a>\n",
                     p -> origin_str.c_str(), ( num_players == 1 ) ? "" : " hide",
                     p -> thumbnail_url.c_str(), p -> name_str.c_str() );

  util_t::fprintf( file,
                   "\t\t\t<h2 class=\"toggle" );
  if ( num_players == 1 )
  {
    util_t::fprintf( file, " open" );
  }

  util_t::fprintf( file, "\">%s&nbsp;:&nbsp;%.0fdps</h2>\n",
                   n.c_str(),
                   p -> dps );

  util_t::fprintf( file,
                   "\t\t\t<div class=\"toggle-content" );
  if ( num_players > 1 )
  {
    util_t::fprintf( file, " hide" );
  }
  util_t::fprintf( file, "\">\n" );

  util_t::fprintf( file,
                   "\t\t\t\t<ul class=\"params\">\n"
                   "\t\t\t\t\t<li><b>Race:</b> %s</li>\n"
                   "\t\t\t\t\t<li><b>Class:</b> %s</li>\n",
                   p -> race_str.c_str(),
                   p -> is_pet() ? util_t::pet_type_string( p -> cast_pet() -> pet_type ) :util_t::player_type_string( p -> type )
                 );

  if ( p -> primary_tree() != TREE_NONE )
    util_t::fprintf( file,
                     "\t\t\t\t\t<li><b>Tree:</b> %s</li>\n",
                     util_t::talent_tree_string( p -> primary_tree() ) );

  util_t::fprintf( file,
                   "\t\t\t\t\t<li><b>Level:</b> %d</li>\n"
                   "\t\t\t\t\t<li><b>Role:</b> %s</li>\n"
                   "\t\t\t\t\t<li><b>Position:</b> %s</li>\n"
                   "\t\t\t\t</ul>\n"
                   "\t\t\t\t<div class=\"clear\"></div>\n",
                   p -> level, util_t::role_type_string( p -> primary_role() ), p -> position_str.c_str() );

  // Main player table
  util_t::fprintf( file,
                   "\t\t\t\t<div class=\"player-section results-spec-gear mt\">\n" );
  if ( p -> is_pet() )
  {
    util_t::fprintf( file,
                     "\t\t\t\t\t<h3 class=\"toggle open\">Results</h3>\n" );
  }
  else
  {
    util_t::fprintf( file,
                     "\t\t\t\t\t<h3 class=\"toggle open\">Results, Spec and Gear</h3>\n" );
  }
  util_t::fprintf( file,
                   "\t\t\t\t\t<div class=\"toggle-content\">\n"
                   "\t\t\t\t\t\t<table class=\"sc\">\n"
                   "\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t<th><a href=\"#help-dps\" class=\"help\">DPS</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th><a href=\"#help-dpse\" class=\"help\">DPS(e)</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th><a href=\"#help-error\" class=\"help\">Error</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th><a href=\"#help-dpr\" class=\"help\">DPR</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th><a href=\"#help-rps-out\" class=\"help\">RPS Out</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th><a href=\"#help-rps-in\" class=\"help\">RPS In</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th>Resource</th>\n"
                   "\t\t\t\t\t\t\t\t<th><a href=\"#help-waiting\" class=\"help\">Waiting</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th><a href=\"#help-apm\" class=\"help\">APM</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th>Active</th>\n"
                   "\t\t\t\t\t\t\t</tr>\n"
                   "\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t<td>%.1f</td>\n"
                   "\t\t\t\t\t\t\t\t<td>%.1f</td>\n"
                   "\t\t\t\t\t\t\t\t<td>%.2f / %.2f%%</td>\n"
                   "\t\t\t\t\t\t\t\t<td>%.1f</td>\n"
                   "\t\t\t\t\t\t\t\t<td>%.1f</td>\n"
                   "\t\t\t\t\t\t\t\t<td>%.1f</td>\n"
                   "\t\t\t\t\t\t\t\t<td>%s</td>\n"
                   "\t\t\t\t\t\t\t\t<td>%.2f%%</td>\n"
                   "\t\t\t\t\t\t\t\t<td>%.1f</td>\n"
                   "\t\t\t\t\t\t\t\t<td>%.1f%%</td>\n"
                   "\t\t\t\t\t\t\t</tr>\n"
                   "\t\t\t\t\t\t</table>\n",
                   p -> dps,
                   p -> dpse,
                   p -> dps_error,
                   p -> dps ? p -> dps_error * 100 / p -> dps : 0,
                   p -> dpr,
                   p -> rps_loss,
                   p -> rps_gain,
                   util_t::resource_type_string( p -> primary_resource() ),
                   p -> total_seconds ? 100.0 * p -> total_waiting / p -> total_seconds : 0,
                   p -> total_seconds ? 60.0 * p -> total_foreground_actions / p -> total_seconds : 0,
                   sim -> total_seconds ? p -> total_seconds / sim -> total_seconds * 100.0 : 0 );

  // Spec and gear
  if ( ! p -> is_pet() )
  {
    util_t::fprintf( file,
                     "\t\t\t\t\t\t<table class=\"sc mt\">\n" );
    if ( p -> origin_str.compare( "unknown" ) )
    {
      std::string enc_url = p -> origin_str;
      util_t::urldecode( enc_url );
      report_t::encode_html( enc_url );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t<tr class=\"left\">\n"
                       "\t\t\t\t\t\t\t\t<th><a href=\"#help-origin\" class=\"help\">Origin</a></th>\n"
                       "\t\t\t\t\t\t\t\t<td><a href=\"%s\" class=\"ext\">%s</a></td>\n"
                       "\t\t\t\t\t\t\t</tr>\n",
                       p -> origin_str.c_str(),
                       enc_url.c_str() );
    }
    if ( ! p -> talents_str.empty() )
    {
      std::string enc_url = p -> talents_str;
      report_t::encode_html( enc_url );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t<tr class=\"left\">\n"
                       "\t\t\t\t\t\t\t\t<th>Talents</th>\n"
                       "\t\t\t\t\t\t\t\t<td><a href=\"%s\" class=\"ext\">%s</a></td>\n"
                       "\t\t\t\t\t\t\t</tr>\n",
                       enc_url.c_str(),
                       enc_url.c_str() );
    }
    std::vector<std::string> glyph_names;
    int num_glyphs = util_t::string_split( glyph_names, p -> glyphs_str, ",/" );
    if ( num_glyphs )
    {
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t<tr class=\"left\">\n"
                       "\t\t\t\t\t\t\t\t<th>Glyphs</th>\n"
                       "\t\t\t\t\t\t\t\t<td>\n"
                       "\t\t\t\t\t\t\t\t\t<ul class=\"float\">\n" );
      for ( int i=0; i < num_glyphs; i++ )
      {
        util_t::fprintf( file,
                         "\t\t\t\t\t\t\t\t\t\t<li>%s</li>\n",
                         glyph_names[ i ].c_str() );
      }
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t\t</ul>\n"
                       "\t\t\t\t\t\t\t\t</td>\n"
                       "\t\t\t\t\t\t\t</tr>\n" );
    }

    util_t::fprintf( file,
                     "\t\t\t\t\t\t</table>\n" );
  }

  // Scale Factors
  print_html_player_scale_factors( file, sim, p );



  print_html_player_charts( file, sim, p );

  // Abilities Section
  util_t::fprintf( file,
                   "\t\t\t\t<div class=\"player-section\">\n"
                   "\t\t\t\t\t<h3 class=\"toggle open\">Abilities</h3>\n"
                   "\t\t\t\t\t<div class=\"toggle-content\">\n"
                   "\t\t\t\t\t\t<table class=\"sc\">\n"
                   "\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"left small\">Damage Stats</th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-dps\" class=\"help\">DPS</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-dps-pct\" class=\"help\">DPS%%</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-count\" class=\"help\">Count</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-interval\" class=\"help\">Interval</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-dpe\" class=\"help\">DPE</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-dpet\" class=\"help\">DPET</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-hit\" class=\"help\">Hit</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-crit\" class=\"help\">Crit</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-max\" class=\"help\">Max</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-crit-pct\" class=\"help\">Crit%%</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-miss-pct\" class=\"help\">Avoid%%</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-glance-pct\" class=\"help\">G%%</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-block-pct\" class=\"help\">B%%</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-ticks\" class=\"help\">Ticks</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-ticks-hit\" class=\"help\">T-Hit</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-ticks-crit\" class=\"help\">T-Crit</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-ticks-crit-pct\" class=\"help\">T-Crit%%</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-ticks-miss-pct\" class=\"help\">T-Avoid%%</a></th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"small\"><a href=\"#help-ticks-uptime\" class=\"help\">Up%%</a></th>\n"
                   "\t\t\t\t\t\t\t</tr>\n" );

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"left small\">%s</th>\n"
                   "\t\t\t\t\t\t\t\t<th class=\"right small\">%.0f</th>\n"
                   "\t\t\t\t\t\t\t\t<td colspan=\"18\" class=\"filler\"></td>\n"
                   "\t\t\t\t\t\t\t</tr>\n",
                   n.c_str(),
                   p -> dps );

  i = 0;
  for ( stats_t* s = p -> stats_list; s; s = s -> next )
  {
    if ( s -> num_executes > 1 || s -> compound_amount > 0 || sim -> debug )
    {
      print_html_action_damage( file, s, p, i );
      i++;
    }
  }

  for ( pet_t* pet = p -> pet_list; pet; pet = pet -> next_pet )
  {
    bool first=true;

    i = 0;
    for ( stats_t* s = pet -> stats_list; s; s = s -> next )
    {
      if ( s -> num_executes || s -> compound_amount > 0 || sim -> debug )
      {
        if ( first )
        {
          first = false;
          util_t::fprintf( file,
                           "\t\t\t\t\t\t\t<tr>\n"
                           "\t\t\t\t\t\t\t\t<th class=\"left small\">pet - %s</th>\n"
                           "\t\t\t\t\t\t\t\t<th class=\"right small\">%.0f / %.0f</th>\n"
                           "\t\t\t\t\t\t\t\t<td colspan=\"18\" class=\"filler\"></td>\n"
                           "\t\t\t\t\t\t\t</tr>\n",
                           pet -> name_str.c_str(),
                           pet -> dps,
                           pet -> dpse );
        }
        print_html_action_damage( file, s, p, i );
        i++;
      }
    }
  }

  util_t::fprintf( file,
                   "\t\t\t\t\t\t</table>\n"
                   "\t\t\t\t\t</div>\n"
                   "\t\t\t\t</div>\n" );


  print_html_player_resources( file, p );

  print_html_player_buffs( file, p );

  util_t::fprintf( file,
                   "\t\t\t\t\t<div class=\"player-section benefits\">\n"
                   "\t\t\t\t\t\t<h3 class=\"toggle\">Benefits & Uptimes</h3>\n"
                   "\t\t\t\t\t\t<div class=\"toggle-content hide\">\n"
                   "\t\t\t\t\t\t\t<table class=\"sc\">\n"
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<th>Benefits</th>\n"
                   "\t\t\t\t\t\t\t\t\t<th>%%</th>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n" );
  i = 1;
  for ( benefit_t* u = p -> benefit_list; u; u = u -> next )
  {
    if ( u -> ratio > 0 )
    {
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t<tr" );
      if ( !( i & 1 ) )
      {
        util_t::fprintf( file, " class=\"odd\"" );
      }
      util_t::fprintf( file, ">\n" );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t\t<td class=\"left\">%s</td>\n"
                       "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.1f%%</td>\n"
                       "\t\t\t\t\t\t\t\t</tr>\n",
                       u -> name(),
                       u -> ratio * 100.0 );
      i++;
    }
  }
  util_t::fprintf( file,
  "\t\t\t\t\t\t\t\t<tr>\n"
  "\t\t\t\t\t\t\t\t\t<th>Uptimes</th>\n"
  "\t\t\t\t\t\t\t\t\t<th>%%</th>\n"
  "\t\t\t\t\t\t\t\t</tr>\n" );
  for ( uptime_t* u = p -> uptime_list; u; u = u -> next )
  {
    if ( u -> uptime > 0 )
    {
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t<tr" );
      if ( !( i & 1 ) )
      {
        util_t::fprintf( file, " class=\"odd\"" );
      }
      util_t::fprintf( file, ">\n" );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t\t<td class=\"left\">%s</td>\n"
                       "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.1f%%</td>\n"
                       "\t\t\t\t\t\t\t\t</tr>\n",
                       u -> name(),
                       u -> uptime * 100.0 );
      i++;
    }
  }

  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t</table>\n"
                   "\t\t\t\t\t\t</div>\n"
                   "\t\t\t\t\t</div>\n" );

  // Procs Section
  util_t::fprintf( file,
                   "\t\t\t\t\t<div class=\"player-section procs\">\n"
                   "\t\t\t\t\t\t<h3 class=\"toggle\">Procs</h3>\n"
                   "\t\t\t\t\t\t<div class=\"toggle-content hide\">\n"
                   "\t\t\t\t\t\t\t<table class=\"sc\">\n"
                   "\t\t\t\t\t\t\t\t<tr>\n"
                   "\t\t\t\t\t\t\t\t\t<th></th>\n"
                   "\t\t\t\t\t\t\t\t\t<th>Count</th>\n"
                   "\t\t\t\t\t\t\t\t\t<th>Interval</th>\n"
                   "\t\t\t\t\t\t\t\t</tr>\n" );
  i = 1;
  for ( proc_t* proc = p -> proc_list; proc; proc = proc -> next )
  {
    if ( proc -> count > 0 )
    {
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t<tr" );
      if ( !( i & 1 ) )
      {
        util_t::fprintf( file, " class=\"odd\"" );
      }
      util_t::fprintf( file, ">\n" );
      util_t::fprintf( file,
                       "\t\t\t\t\t\t\t\t\t<td class=\"left\">%s</td>\n"
                       "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.1f</td>\n"
                       "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.1fsec</td>\n"
                       "\t\t\t\t\t\t\t\t</tr>\n",
                       proc -> name(),
                       proc -> count,
                       proc -> frequency );
      i++;
    }
  }
  util_t::fprintf( file,
                   "\t\t\t\t\t\t\t</table>\n"
                   "\t\t\t\t\t\t</div>\n"
                   "\t\t\t\t\t</div>\n" );




  // Death Analysis

  if ( p -> death_count > 0 )
  {
    std::string distribution_deaths_str                = "";
    if ( ! p -> distribution_deaths_chart.empty() )
    {
      distribution_deaths_str = "<img src=\"" + p -> distribution_deaths_chart + "\" alt=\"Deaths Distribution Chart\" />\n";
    }

    util_t::fprintf( file,
                     "\t\t\t\t\t<div class=\"player-section gains\">\n"
                     "\t\t\t\t\t\t<h3 class=\"toggle\">Deaths</h3>\n"
                     "\t\t\t\t\t\t<div class=\"toggle-content hide\">\n"
                     "\t\t\t\t\t\t\t<table class=\"sc\">\n"
                     "\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t<th></th>\n"
                     "\t\t\t\t\t\t\t\t\t<th></th>\n"
                     "\t\t\t\t\t\t\t\t</tr>\n" );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t<td class=\"left\">death count</td>\n"
                     "\t\t\t\t\t\t\t\t\t<td class=\"right\">%i</td>\n"
                     "\t\t\t\t\t\t\t\t</tr>\n",
                     p -> death_count );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t<td class=\"left\">death count pct</td>\n"
                     "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
                     "\t\t\t\t\t\t\t\t</tr>\n",
                     p -> death_count_pct );
    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t<td class=\"left\">avg death time</td>\n"
                     "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
                     "\t\t\t\t\t\t\t\t</tr>\n",
                     p -> avg_death_time );
    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t<td class=\"left\">min death time</td>\n"
                     "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
                     "\t\t\t\t\t\t\t\t</tr>\n",
                     p -> min_death_time );
    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t<td class=\"left\">max death time</td>\n"
                     "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
                     "\t\t\t\t\t\t\t\t</tr>\n",
                     p -> max_death_time );
    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t<tr>\n"
                     "\t\t\t\t\t\t\t\t\t<td class=\"left\">dmg taken</td>\n"
                     "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
                     "\t\t\t\t\t\t\t\t</tr>\n",
                     p -> total_dmg_taken );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t</table>\n" );

    util_t::fprintf ( file,
                      "\t\t\t\t\t\t\t\t\t<div class=\"clear\"></div>\n" );

    util_t::fprintf ( file,
                      "\t\t\t\t\t\t\t\t\t%s\n",
                      distribution_deaths_str.c_str() );

    util_t::fprintf ( file,
                      "\t\t\t\t\t\t\t</div>\n"
                      "\t\t\t\t\t\t</div>\n" );
  }



  print_html_player_statistics( file, p );

  print_html_player_action_priority_list( file, sim, p );

  print_html_stats( file, p );

  print_html_gear( file, p );

  print_html_talents( file, p );

  print_html_profile( file, p );


  if ( p -> sim -> scaling -> has_scale_factors() && !p -> is_pet() )
  {
    util_t::fprintf( file,
                     "\t\t\t\t\t\t<div class=\"player-section gear-weights\">\n"
                     "\t\t\t\t\t\t\t<h3 class=\"toggle\">Gear Weights</h3>\n"
                     "\t\t\t\t\t\t\t<div class=\"toggle-content hide\">\n"
                     "\t\t\t\t\t\t\t\t<table class=\"sc mb\">\n"
                     "\t\t\t\t\t\t\t\t\t<tr class=\"left\">\n"
                     "\t\t\t\t\t\t\t\t\t\t<th>Pawn Standard</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td>%s</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n"
                     "\t\t\t\t\t\t\t\t\t<tr class=\"left\">\n"
                     "\t\t\t\t\t\t\t\t\t\t<th>Zero Hit/Expertise</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td>%s</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n"
                     "\t\t\t\t\t\t\t\t</table>\n",
                     p -> gear_weights_pawn_std_string.c_str(),
                     p -> gear_weights_pawn_alt_string.c_str() );

    std::string rhada_std = p -> gear_weights_pawn_std_string;
    std::string rhada_alt = p -> gear_weights_pawn_alt_string;

    if ( rhada_std.size() > 10 ) rhada_std.replace( 2, 8, "RhadaTip" );
    if ( rhada_alt.size() > 10 ) rhada_alt.replace( 2, 8, "RhadaTip" );

    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t\t<table class=\"sc\">\n"
                     "\t\t\t\t\t\t\t\t\t<tr class=\"left\">\n"
                     "\t\t\t\t\t\t\t\t\t\t<th>RhadaTip Standard</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td>%s</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n"
                     "\t\t\t\t\t\t\t\t\t<tr class=\"left\">\n"
                     "\t\t\t\t\t\t\t\t\t\t<th>Zero Hit/Expertise</th>\n"
                     "\t\t\t\t\t\t\t\t\t\t<td>%s</td>\n"
                     "\t\t\t\t\t\t\t\t\t</tr>\n"
                     "\t\t\t\t\t\t\t\t</table>\n",
                     rhada_std.c_str(),
                     rhada_alt.c_str() );
    util_t::fprintf( file,
                     "\t\t\t\t\t\t\t</div>\n"
                     "\t\t\t\t\t\t</div>\n" );
  }

  util_t::fprintf( file,
                   "\t\t\t\t\t</div>\n"
                   "\t\t\t\t</div>\n\n" );
}

} // ANONYMOUS NAMESPACE ====================================================



// report_t::print_html_player ====================================================

void report_t::print_html_player( FILE* file, player_t* p, int j )
{
  print_html_player_( file, p -> sim, p, j );
}
