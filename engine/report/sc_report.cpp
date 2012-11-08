// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simulationcraft.hpp"
#include "sc_report.hpp"

// ==========================================================================
// Report
// ==========================================================================

namespace { // UNNAMED NAMESPACE

template <unsigned HW, typename Fwd, typename Out>
void sliding_window_average( Fwd first, Fwd last, Out out )
{
  typedef typename std::iterator_traits<Fwd>::value_type value_t;
  typedef typename std::iterator_traits<Fwd>::difference_type diff_t;
  diff_t n = std::distance( first, last );
  diff_t HALFWINDOW = static_cast<diff_t>( HW );

  if ( n >= 2 * HALFWINDOW )
  {
    value_t window_sum = value_t();

    // Fill right half of sliding window
    Fwd right = first;
    for ( diff_t count = 0; count < HALFWINDOW; ++count )
      window_sum += *right++;

    // Fill left half of sliding window
    for ( diff_t count = HALFWINDOW; count < 2 * HALFWINDOW; ++count )
    {
      window_sum += *right++;
      *out++ = window_sum / ( count + 1 );
    }

    // Slide until window hits end of data
    while ( right != last )
    {
      window_sum += *right++;
      *out++ = window_sum / ( 2 * HALFWINDOW + 1 );
      window_sum -= *first++;
    }

    // Empty right half of sliding window
    for ( diff_t count = 2 * HALFWINDOW; count > HALFWINDOW; --count )
    {
      *out++ = window_sum / count;
      window_sum -= *first++;
    }
  }
  else
  {
    // input is pathologically small compared to window size, just average everything.
    std::fill_n( out, n, std::accumulate( first, last, value_t() ) / n );
  }
}

template <unsigned HW, typename Range, typename Out>
inline Range& sliding_window_average( Range& r, Out out )
{
  sliding_window_average<HW>( range::begin( r ), range::end( r ), out );
  return r;
}

struct buff_is_dynamic
{
  bool operator() ( const buff_t* b ) const
  {
    if ( ! b -> quiet && b -> start_count && ! b -> constant )
      return false;

    return true;
  }
};

struct buff_is_constant
{
  bool operator() ( const buff_t* b ) const
  {
    if ( ! b -> quiet && b -> start_count && b -> constant )
      return false;

    return true;
  }
};

struct buff_comp
{
  bool operator()( const buff_t* i, const buff_t* j )
  {
    // Aura&Buff / Pet
    if ( ( ! i -> player || ! i -> player -> is_pet() ) && j -> player && j -> player -> is_pet() )
      return true;
    // Pet / Aura&Buff
    else if ( i -> player && i -> player -> is_pet() && ( ! j -> player || ! j -> player -> is_pet() ) )
      return false;
    // Pet / Pet
    else if ( i -> player && i -> player -> is_pet() && j -> player && j -> player -> is_pet() )
    {
      if ( i -> player -> name_str.compare( j -> player -> name_str ) == 0 )
        return ( i -> name_str.compare( j -> name_str ) < 0 );
      else
        return ( i -> player -> name_str.compare( j -> player -> name_str ) < 0 );
    }

    return ( i -> name_str.compare( j -> name_str ) < 0 );
  }
};

} // UNNAMED NAMESPACE ====================================================

// report::print_profiles ===================================================

void report::print_profiles( sim_t* sim )
{
  int k = 0;
  for ( unsigned int i = 0; i < sim -> actor_list.size(); i++ )
  {
    player_t* p = sim -> actor_list[i];
    if ( p -> is_pet() ) continue;

    k++;
    FILE* file = NULL;

    if ( !p -> report_information.save_gear_str.empty() ) // Save gear
    {
      file = fopen( p -> report_information.save_gear_str.c_str(), "w" );
      if ( ! file )
      {
        sim -> errorf( "Unable to save gear profile %s for player %s\n", p -> report_information.save_gear_str.c_str(), p -> name() );
      }
      else
      {
        std::string profile_str = "";
        p -> create_profile( profile_str, SAVE_GEAR );
        fprintf( file, "%s", profile_str.c_str() );
        fclose( file );
      }
    }

    if ( !p -> report_information.save_talents_str.empty() ) // Save talents
    {
      file = fopen( p -> report_information.save_talents_str.c_str(), "w" );
      if ( ! file )
      {
        sim -> errorf( "Unable to save talents profile %s for player %s\n", p -> report_information.save_talents_str.c_str(), p -> name() );
      }
      else
      {
        std::string profile_str = "";
        p -> create_profile( profile_str, SAVE_TALENTS );
        fprintf( file, "%s", profile_str.c_str() );
        fclose( file );
      }
    }

    if ( !p -> report_information.save_actions_str.empty() ) // Save actions
    {
      file = fopen( p -> report_information.save_actions_str.c_str(), "w" );
      if ( ! file )
      {
        sim -> errorf( "Unable to save actions profile %s for player %s\n", p -> report_information.save_actions_str.c_str(), p -> name() );
      }
      else
      {
        std::string profile_str = "";
        p -> create_profile( profile_str, SAVE_ACTIONS );
        fprintf( file, "%s", profile_str.c_str() );
        fclose( file );
      }
    }

    std::string file_name = p -> report_information.save_str;

    if ( file_name.empty() && sim -> save_profiles )
    {
      file_name  = sim -> save_prefix_str;
      file_name += p -> name_str;
      if ( sim -> save_talent_str != 0 )
      {
        file_name += "_";
        file_name += p -> primary_tree_name();
      }
      file_name += sim -> save_suffix_str;
      file_name += ".simc";
      util::urlencode( util::format_text( file_name, sim -> input_is_utf8 ) );
    }

    if ( file_name.empty() ) continue;

    file = fopen( file_name.c_str(), "w" );
    if ( ! file )
    {
      sim -> errorf( "Unable to save profile %s for player %s\n", file_name.c_str(), p -> name() );
      continue;
    }

    std::string profile_str = "";
    p -> create_profile( profile_str );
    fprintf( file, "%s", profile_str.c_str() );
    fclose( file );
  }

  // Save overview file for Guild downloads
  //if ( /* guild parse */ )
  if ( sim -> save_raid_summary )
  {
    FILE* file = NULL;

    std::string filename = "Raid_Summary.simc";
    std::string player_str = "#Raid Summary\n";
    player_str += "# Contains ";
    player_str += util::to_string( k );
    player_str += " Players.\n\n";

    for ( unsigned int i = 0; i < sim -> actor_list.size(); i++ )
    {
      player_t* p = sim -> actor_list[ i ];
      if ( p -> is_pet() ) continue;

      std::string file_name = p -> report_information.save_str;
      std::string profile_name;

      if ( file_name.empty() && sim -> save_profiles )
      {
        file_name  = "# Player: ";
        file_name += p -> name_str;
        file_name += " Spec: ";
        file_name += p -> primary_tree_name();
        file_name += " Role: ";
        file_name += util::role_type_string( p -> primary_role() );
        file_name += "\n";
        profile_name += sim -> save_prefix_str;
        profile_name += p -> name_str;
        if ( sim -> save_talent_str != 0 )
        {
          profile_name += "_";
          profile_name += p -> primary_tree_name();
        }
        profile_name += sim -> save_suffix_str;
        profile_name += ".simc";
        util::urlencode( util::format_text( profile_name, sim -> input_is_utf8 ) );
        file_name += profile_name;
        file_name += "\n\n";
      }
      player_str += file_name;
    }


    file = fopen( filename.c_str(), "w" );
    if ( ! file )
    {
      sim -> errorf( "Unable to save overview profile %s\n", filename.c_str() );
    }
    else
    {
      fprintf( file, "%s", player_str.c_str() );
      fclose( file );
    }
  }
}

// report::print_spell_query ==============================================

void report::print_spell_query( sim_t* sim, unsigned level )
{
  spell_data_expr_t* sq = sim -> spell_query;
  assert( sq );

  FILE * file = NULL;
  xml_node_t* root = NULL;
  if ( ! sim -> spell_query_xml_output_file_str.empty() )
  {
    file = fopen( sim -> spell_query_xml_output_file_str.c_str(), "w" );
    if ( ! file )
    {
      sim -> errorf( "Unable to open spell query xml output file '%s', using stdout instead\n", sim -> spell_query_xml_output_file_str.c_str() );
      file = stdout;
    }
    root = new xml_node_t( "spell_query" );
  }

  for ( std::vector<uint32_t>::iterator i = sq -> result_spell_list.begin(); i != sq -> result_spell_list.end(); ++i )
  {
    if ( sq -> data_type == DATA_TALENT )
    {
      if ( root )
        spell_info::talent_to_xml( sim, sim -> dbc.talent( *i ), root );
      else
        util::fprintf( sim -> output_file, "%s", spell_info::talent_to_str( sim, sim -> dbc.talent( *i ) ).c_str() );
    }
    else if ( sq -> data_type == DATA_EFFECT )
    {
      std::ostringstream sqs;
      const spell_data_t* spell = sim -> dbc.spell( sim -> dbc.effect( *i ) -> spell_id() );
      if ( spell )
      {
        if ( root )
          spell_info::effect_to_xml( sim, spell, sim -> dbc.effect( *i ), root );
        else
          spell_info::effect_to_str( sim, spell, sim -> dbc.effect( *i ), sqs );
      }
      util::fprintf( sim -> output_file, "%s", sqs.str().c_str() );
    }
    else
    {
      const spell_data_t* spell = sim -> dbc.spell( *i );
      if ( root )
        spell_info::to_xml( sim, spell, root, level );
      else
        util::fprintf( sim -> output_file, "%s", spell_info::to_str( sim, spell, level ).c_str() );
    }
  }

  if ( root )
  {
    util::fprintf( file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );
    root -> print_xml( file );
    delete root;
  }
}

// report::print_suite ====================================================

void report::print_suite( sim_t* sim )
{
  report::print_text( sim -> output_file, sim, sim -> report_details != 0 );
  report::print_html( sim );
  report::print_xml( sim );
  report::print_profiles( sim );
}

void report::print_html_sample_data( report::sc_html_stream& os, sim_t* sim, sample_data_t& data, const std::string& name )
{
  // Print Statistics of a Sample Data Container

  os.printf(
    "\t\t\t\t\t<div>\n"
    "\t\t\t\t\t\t<h3 class=\"toggle\">%s</h3>\n"
    "\t\t\t\t\t\t<div class=\"toggle-content hide\">\n",
    name.c_str() );

  if ( data.basics_analyzed() )
  {
    int i = 0;

    os << "\t\t\t\t\t\t\t<table class=\"details\">\n";

    os << "\t\t\t\t\t\t\t\t<tr";
    if ( !( i & 1 ) )
    {
      os << " class=\"odd\"";
    }
    os << ">\n";
    os << "\t\t\t\t\t\t\t\t\t<th class=\"left\"><b>Sample Data</b></td>\n"
       << "\t\t\t\t\t\t\t\t\t<th class=\"right\">" << data.name_str << "</td>\n"
       << "\t\t\t\t\t\t\t\t</tr>\n";

    ++i;
    os << "\t\t\t\t\t\t\t\t<tr";
    if ( !( i & 1 ) )
    {
      os << " class=\"odd\"";
    }
    os << ">\n";
    os.printf(
      "\t\t\t\t\t\t\t\t\t<td class=\"left\">Count</td>\n"
      "\t\t\t\t\t\t\t\t\t<td class=\"right\">%d</td>\n"
      "\t\t\t\t\t\t\t\t</tr>\n",
      data.size() );

    ++i;
    os << "\t\t\t\t\t\t\t\t<tr";
    if ( !( i & 1 ) )
    {
      os << " class=\"odd\"";
    }
    os << ">\n";
    os.printf(
      "\t\t\t\t\t\t\t\t<tr>\n"
      "\t\t\t\t\t\t\t\t\t<td class=\"left\">Mean</td>\n"
      "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
      "\t\t\t\t\t\t\t\t</tr>\n",
      data.mean );

    if ( !data.simple || data.min_max )
    {
      ++i;
      os << "\t\t\t\t\t\t\t\t<tr";
      if ( !( i & 1 ) )
      {
        os << " class=\"odd\"";
      }
      os << ">\n";

      os.printf(
        "\t\t\t\t\t\t\t\t<tr>\n"
        "\t\t\t\t\t\t\t\t\t<td class=\"left\">Minimum</td>\n"
        "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
        "\t\t\t\t\t\t\t\t</tr>\n",
        data.min );

      ++i;
      os << "\t\t\t\t\t\t\t\t<tr";
      if ( !( i & 1 ) )
      {
        os << " class=\"odd\"";
      }
      os << ">\n";
      os.printf(
        "\t\t\t\t\t\t\t\t<tr>\n"
        "\t\t\t\t\t\t\t\t\t<td class=\"left\">Maximum</td>\n"
        "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
        "\t\t\t\t\t\t\t\t</tr>\n",
        data.max );

      ++i;
      os << "\t\t\t\t\t\t\t\t<tr";
      if ( !( i & 1 ) )
      {
        os << " class=\"odd\"";
      }
      os << ">\n";
      os.printf(
        "\t\t\t\t\t\t\t\t<tr>\n"
        "\t\t\t\t\t\t\t\t\t<td class=\"left\">Spread ( max - min )</td>\n"
        "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
        "\t\t\t\t\t\t\t\t</tr>\n",
        data.max - data.min );

      ++i;
      os << "\t\t\t\t\t\t\t\t<tr";
      if ( !( i & 1 ) )
      {
        os << " class=\"odd\"";
      }
      os << ">\n";
      os.printf(
        "\t\t\t\t\t\t\t\t<tr>\n"
        "\t\t\t\t\t\t\t\t\t<td class=\"left\">Range [ ( max - min ) / 2 * 100%% ]</td>\n"
        "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f%%</td>\n"
        "\t\t\t\t\t\t\t\t</tr>\n",
        data.mean ? ( ( data.max - data.min ) / 2 ) * 100 / data.mean : 0 );

      if ( !data.simple && data.variance_analyzed() )
      {
        ++i;
        os << "\t\t\t\t\t\t\t\t<tr";
        if ( !( i & 1 ) )
        {
          os << " class=\"odd\"";
        }
        os << ">\n";
        os.printf(
          "\t\t\t\t\t\t\t\t<tr>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"left\">Standard Deviation</td>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.4f</td>\n"
          "\t\t\t\t\t\t\t\t</tr>\n",
          data.std_dev );

        ++i;
        os << "\t\t\t\t\t\t\t\t<tr";
        if ( !( i & 1 ) )
        {
          os << " class=\"odd\"";
        }
        os << ">\n";
        os.printf(
          "\t\t\t\t\t\t\t\t<tr>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"left\">5th Percentile</td>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
          "\t\t\t\t\t\t\t\t</tr>\n",
          data.percentile( 0.05 ) );

        ++i;
        os << "\t\t\t\t\t\t\t\t<tr";
        if ( !( i & 1 ) )
        {
          os << " class=\"odd\"";
        }
        os << ">\n";
        os.printf(
          "\t\t\t\t\t\t\t\t<tr>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"left\">95th Percentile</td>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
          "\t\t\t\t\t\t\t\t</tr>\n",
          data.percentile( 0.95 ) );

        ++i;
        os << "\t\t\t\t\t\t\t\t<tr";
        if ( !( i & 1 ) )
        {
          os << " class=\"odd\"";
        }
        os << ">\n";
        os.printf(
          "\t\t\t\t\t\t\t\t<tr>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"left\">( 95th Percentile - 5th Percentile )</td>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.2f</td>\n"
          "\t\t\t\t\t\t\t\t</tr>\n",
          data.percentile( 0.95 ) - data.percentile( 0.05 ) );

        ++i;
        os << "\t\t\t\t\t\t\t\t<tr";
        if ( !( i & 1 ) )
        {
          os << " class=\"odd\"";
        }
        os << ">\n";
        os.printf(
          "\t\t\t\t\t\t\t\t<tr>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"left\"><b>Mean Distribution</b></td>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"right\"></td>\n"
          "\t\t\t\t\t\t\t\t</tr>\n" );

        ++i;
        os << "\t\t\t\t\t\t\t\t<tr";
        if ( !( i & 1 ) )
        {
          os << " class=\"odd\"";
        }
        os << ">\n";
        os.printf(
          "\t\t\t\t\t\t\t\t<tr>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"left\">Standard Deviation</td>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"right\">%.4f</td>\n"
          "\t\t\t\t\t\t\t\t</tr>\n",
          data.mean_std_dev );

        ++i;
        double mean_error = data.mean_std_dev * sim -> confidence_estimator;
        ++i;
        os << "\t\t\t\t\t\t\t\t<tr";
        if ( !( i & 1 ) )
        {
          os << " class=\"odd\"";
        }
        os << ">\n";
        os.printf(
          "\t\t\t\t\t\t\t\t<tr>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"left\">%.2f%% Confidence Intervall</td>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"right\">( %.2f - %.2f )</td>\n"
          "\t\t\t\t\t\t\t\t</tr>\n",
          sim -> confidence * 100.0,
          data.mean - mean_error,
          data.mean + mean_error );

        ++i;
        os << "\t\t\t\t\t\t\t\t<tr";
        if ( !( i & 1 ) )
        {
          os << " class=\"odd\"";
        }
        os << ">\n";
        os.printf(
          "\t\t\t\t\t\t\t\t<tr>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"left\">Normalized %.2f%% Confidence Intervall</td>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"right\">( %.2f%% - %.2f%% )</td>\n"
          "\t\t\t\t\t\t\t\t</tr>\n",
          sim -> confidence * 100.0,
          data.mean ? 100 - mean_error * 100 / data.mean : 0,
          data.mean ? 100 + mean_error * 100 / data.mean : 0 );



        ++i;
        os << "\t\t\t\t\t\t\t\t<tr";
        if ( !( i & 1 ) )
        {
          os << " class=\"odd\"";
        }
        os << ">\n";
        os << "\t\t\t\t\t\t\t\t<tr>\n"
           << "\t\t\t\t\t\t\t\t\t<td class=\"left\"><b>Approx. Iterations needed for ( always use n>=50 )</b></td>\n"
           << "\t\t\t\t\t\t\t\t\t<td class=\"right\"></td>\n"
           << "\t\t\t\t\t\t\t\t</tr>\n";

        os.printf(
          "\t\t\t\t\t\t\t\t<tr>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"left\">1%% Error</td>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"right\">%i</td>\n"
          "\t\t\t\t\t\t\t\t</tr>\n",
          ( int ) ( data.mean ? ( ( mean_error * mean_error * ( ( float ) data.size() ) / ( 0.01 * data.mean * 0.01 * data.mean ) ) ) : 0 ) );

        ++i;
        os << "\t\t\t\t\t\t\t\t<tr";
        if ( !( i & 1 ) )
        {
          os << " class=\"odd\"";
        }
        os << ">\n";
        os.printf(
          "\t\t\t\t\t\t\t\t<tr>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"left\">0.1%% Error</td>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"right\">%i</td>\n"
          "\t\t\t\t\t\t\t\t</tr>\n",
          ( int ) ( data.mean ? ( ( mean_error * mean_error * ( ( float ) data.size() ) / ( 0.001 * data.mean * 0.001 * data.mean ) ) ) : 0 ) );

        ++i;
        os << "\t\t\t\t\t\t\t\t<tr";
        if ( !( i & 1 ) )
        {
          os << " class=\"odd\"";
        }
        os << ">\n";
        os.printf(
          "\t\t\t\t\t\t\t\t<tr>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"left\">0.1 Scale Factor Error with Delta=300</td>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"right\">%i</td>\n"
          "\t\t\t\t\t\t\t\t</tr>\n",
          ( int ) ( 2.0 * mean_error * mean_error * ( ( float ) data.size() ) / ( 30 * 30 ) ) );

        ++i;
        os << "\t\t\t\t\t\t\t\t<tr";
        if ( !( i & 1 ) )
        {
          os << " class=\"odd\"";
        }
        os << ">\n";
        os.printf(
          "\t\t\t\t\t\t\t\t<tr>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"left\">0.05 Scale Factor Error with Delta=300</td>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"right\">%i</td>\n"
          "\t\t\t\t\t\t\t\t</tr>\n",
          ( int ) ( 2.0 * mean_error * mean_error * ( ( float ) data.size() ) / ( 15 * 15 ) ) );

        ++i;
        os << "\t\t\t\t\t\t\t\t<tr";
        if ( !( i & 1 ) )
        {
          os << " class=\"odd\"";
        }
        os << ">\n";
        os.printf(
          "\t\t\t\t\t\t\t\t<tr>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"left\">0.01 Scale Factor Error with Delta=300</td>\n"
          "\t\t\t\t\t\t\t\t\t<td class=\"right\">%i</td>\n"
          "\t\t\t\t\t\t\t\t</tr>\n",
          ( int ) (  2.0 * mean_error * mean_error * ( ( float ) data.size() ) / ( 3 * 3 ) ) );
      }
    }

  }
  os << "\t\t\t\t\t\t\t\t</table>\n";

  if ( data.distribution_created() )
  {
    std::string dist_chart = chart::distribution( sim -> print_styles, data.distribution, name, data.mean, data.min, data.max );

    os.printf(
      "\t\t\t\t\t<img src=\"%s\" alt=\"Distribution Chart\" />\n",
      dist_chart.c_str() );
  }
  os <<  "\t\t\t\t\t\t</div>\n"
     <<  "\t\t\t\t\t</div>\n";

}

void report::generate_player_buff_lists( player_t*  p, player_t::report_information_t& ri )
{
  if ( ri.buff_lists_generated )
    return;

  // Append p -> buff_list to ri.buff_list
  ri.buff_list.insert( ri.buff_list.end(), p -> buff_list.begin(), p -> buff_list.end() );

  for ( size_t i = 0; i < p -> pet_list.size(); ++i )
  {
    pet_t* pet = p -> pet_list[ i ];
    // Append pet -> buff_list to ri.buff_list
    ri.buff_list.insert( ri.buff_list.end(), pet -> buff_list.begin(), pet -> buff_list.end() );
  }

  // Append p -> sim -> buff_list to ri.buff_list
  ri.buff_list.insert( ri.buff_list.end(), p -> sim -> buff_list.begin(), p -> sim -> buff_list.end() );

  // Filter out non-dynamic buffs, copy them into ri.dynamic_buffs and sort
  //range::remove_copy_if( ri.buff_list, back_inserter( ri.dynamic_buffs ), buff_is_dynamic );
  range::remove_copy_if( ri.buff_list, back_inserter( ri.dynamic_buffs ), buff_is_dynamic() );
  range::sort( ri.dynamic_buffs, buff_comp() );

  // Filter out non-constant buffs, copy them into ri.constant_buffs and sort
  range::remove_copy_if( ri.buff_list, back_inserter( ri.constant_buffs ), buff_is_constant() );
  range::sort( ri.constant_buffs, buff_comp() );

  ri.buff_lists_generated = true;
}

void report::generate_player_charts( player_t* p, player_t::report_information_t& ri )
{
  if ( ri.charts_generated )
    return;

  // Pet Chart Adjustment ===================================================
  size_t max_buckets = static_cast<size_t>( p -> fight_length.max );

  // Make the pet graphs the same length as owner's
  if ( p -> is_pet() )
  {
    player_t* o = p -> cast_pet() -> owner;
    max_buckets = static_cast<size_t>( o -> fight_length.max );
  }


  // Stats Charts
  std::vector<stats_t*> stats_list;

  // Append p -> stats_list to stats_list
  stats_list.insert( stats_list.end(), p -> stats_list.begin(), p -> stats_list.end() );

  for ( size_t i = 0; i < p -> pet_list.size(); ++i )
  {
    pet_t* pet = p -> pet_list[ i ];
    // Append pet -> stats_list to stats_list
    stats_list.insert( stats_list.end(), pet -> stats_list.begin(), pet -> stats_list.end() );
  }

  if ( ! p -> is_pet() )
  {
    for ( size_t i = 0; i < stats_list.size(); i++ )
    {
      stats_t* s = stats_list[ i ];

      // Create Stats Timeline Chart
      s -> timeline_amount.resize( max_buckets );
      std::vector<double> timeline_aps;
      timeline_aps.reserve( s -> timeline_amount.size() );
      sliding_window_average<10>( s -> timeline_amount, std::back_inserter( timeline_aps ) );
      s -> timeline_aps_chart = chart::timeline( p, timeline_aps, s -> name_str + ( s -> type == STATS_DMG ? " DPS" : " HPS" ), s -> portion_aps.mean );
      s -> aps_distribution_chart = chart::distribution( p -> sim -> print_styles, s -> portion_aps.distribution, s -> name_str + ( s -> type == STATS_DMG ? " DPS" : " HPS" ),
                                                         s -> portion_aps.mean, s -> portion_aps.min, s -> portion_aps.max );
    }
  }
  // End Stats Charts

  // Player Charts
  ri.action_dpet_chart    = chart::action_dpet  ( p );
  ri.action_dmg_chart     = chart::aps_portion  ( p );
  ri.time_spent_chart     = chart::time_spent   ( p );
  ri.scaling_dps_chart    = chart::scaling_dps  ( p );
  ri.reforge_dps_chart    = chart::reforge_dps  ( p );
  ri.scale_factors_chart  = chart::scale_factors( p );

  std::string encoded_name;
  http::format( encoded_name, p -> name_str );

  {
    std::vector<double> timeline_dps;
    timeline_dps.reserve( p -> timeline_dmg.size() );
    sliding_window_average<10>( p -> timeline_dmg, std::back_inserter( timeline_dps ) );
    ri.timeline_dps_chart = chart::timeline( p, timeline_dps, encoded_name + " DPS", p -> dps.mean );
  }

  ri.timeline_dps_error_chart = chart::timeline_dps_error( p );
  ri.dps_error_chart = chart::dps_error         ( p );

  if ( p -> primary_role() == ROLE_HEAL )
  {
    ri.distribution_dps_chart = chart::distribution( p -> sim -> print_styles,
                                                     p -> hps.distribution, encoded_name + " HPS",
                                                     p -> hps.mean,
                                                     p -> hps.min,
                                                     p -> hps.max );
  }
  else
  {
    ri.distribution_dps_chart = chart::distribution( p -> sim -> print_styles,
                                                     p -> dps.distribution, encoded_name + " DPS",
                                                     p -> dps.mean,
                                                     p -> dps.min,
                                                     p -> dps.max );
  }

  ri.distribution_deaths_chart = chart::distribution( p -> sim -> print_styles,
                                                      p -> deaths.distribution, encoded_name + " Death",
                                                      p -> deaths.mean,
                                                      p -> deaths.min,
                                                      p -> deaths.max );

  // Resource Charts
  for ( size_t i = 0; i < p -> resource_timeline_count; ++i )
  {
    resource_e rt = p -> resource_timelines[ i ].type;
    ri.timeline_resource_chart[ rt ] =
      chart::timeline( p,
                       p -> resource_timelines[ i ].timeline,
                       encoded_name + ' ' + util::inverse_tokenize( util::resource_type_string( rt ) ),
                       0,
                       chart::resource_color( rt ) );
    ri.gains_chart[ rt ] = chart::gains( p, rt );
  }

  // Scaling charts
  if ( ! ( ( p -> sim -> scaling -> num_scaling_stats <= 0 ) || p -> quiet || p -> is_pet() || p -> is_enemy() || p -> is_add() || p -> type == HEALING_ENEMY ) )
  {
#if LOOTRANK_ENABLED == 1
    ri.gear_weights_lootrank_link    = chart::gear_weights_lootrank   ( p );
#endif
    ri.gear_weights_wowhead_link     = chart::gear_weights_wowhead    ( p );
    ri.gear_weights_wowreforge_link  = chart::gear_weights_wowreforge ( p );
    ri.gear_weights_pawn_std_string  = chart::gear_weights_pawn       ( p, true  );
    ri.gear_weights_pawn_alt_string  = chart::gear_weights_pawn       ( p, false );
  }

  // Create html profile str
  p -> create_profile( ri.html_profile_str, SAVE_ALL, true );

  ri.charts_generated = true;
}

void report::generate_sim_report_information( sim_t* s , sim_t::report_information_t& ri )
{
  if ( ri.charts_generated )
    return;



  chart::raid_aps     ( ri.dps_charts, s, s -> players_by_dps, true );
  chart::raid_aps     ( ri.hps_charts, s, s -> players_by_hps, false );
  chart::raid_dpet    ( ri.dpet_charts, s );
  chart::raid_gear    ( ri.gear_charts, s );
  ri.timeline_chart = chart::distribution( s -> print_styles,
                                           s -> simulation_length.distribution, "Timeline",
                                           s -> simulation_length.mean,
                                           s -> simulation_length.min,
                                           s -> simulation_length.max );

  ri.charts_generated = true;
}


