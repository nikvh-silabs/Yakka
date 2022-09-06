#include "blueprint_database.hpp"
#include "utilities.hpp"
#include "inja.hpp"
// #include <filesystem>
// #include <fstream>
#include <regex>

// namespace fs = std::filesystem;

namespace yakka
{
    #if 0
    void blueprint_database::generate_task_database(std::vector<std::string> command_list)
    {
        std::multimap<std::string, blueprint_match > task_database;
        std::vector<std::string> new_targets;

        while (!command_list.empty())
        {
            for (const auto& t: command_list)
            {
                // Add to processed targets and check if it's already been processed
                // if (processed_targets.insert(t).second == false)
                //     continue;

                // Do not add to task database if it's a data dependency. There is special processing of these.
                if (t.front() == ':')
                    continue;

                // Check if target is not in the database. Note task_database is a multimap
                if (task_database.find(t) == task_database.end())
                {
                    process_blueprint_target(t);
                }
                auto tasks = task_database.equal_range(t);

                std::for_each(tasks.first, tasks.second, [&new_targets](auto& i) {
                    new_targets.insert(new_targets.end(), i.second.dependencies.begin(), i.second.dependencies.end());
                });
            }

            command_list.clear();
            command_list.swap(new_targets);
        }    
    }

    void blueprint_database::process_blueprint_target( const std::string target, nlohmann::json& data )
    {
        bool blueprint_match_found = false;

        for ( auto& blueprint : blueprints )
        {
            blueprint_match match;

            // Check if rule is a regex, otherwise do a string comparison
            if ( blueprint.second.regex )
            {
                std::smatch s;
                if (!std::regex_match(target, s, std::regex { blueprint.second.regex.value() } ) )
                    continue;

                // arg_count starts at 0 as the first match is the entire string
                int arg_count = 0;
                for ( auto& regex_match : s )
                {
                    match.regex_matches[arg_count] = regex_match.str( );
                    ++arg_count;
                }
            }
            else
            {
                if (target != blueprint.first )
                    continue;

                match.regex_matches[0] = target;
            }

            // Found a match. Create a blueprint match object
            blueprint_match_found = true;
            match.blueprint = blueprint.second;

            inja::Environment local_inja_env;
            local_inja_env.add_callback("$", 1, [&match](const inja::Arguments& args) {
                        return match.regex_matches[ args[0]->get<int>() ];
                    });

            local_inja_env.add_callback("curdir", 0, [&match](const inja::Arguments& args) { return match.blueprint["yakka_parent_path"].Scalar();});
            local_inja_env.add_callback("notdir", 1, [](inja::Arguments& args) { return std::filesystem::path{args.at(0)->get<std::string>()}.filename();});
            local_inja_env.add_callback("extension", 1, [](inja::Arguments& args) { return std::filesystem::path{args.at(0)->get<std::string>()}.extension();});
            local_inja_env.add_callback("render", 1, [&](const inja::Arguments& args) { return local_inja_env.render(args[0]->get<std::string>(), data);});
            local_inja_env.add_callback("aggregate", 1, [&](const inja::Arguments& args) {
                YAML::Node aggregate;
                const std::string path = args[0]->get<std::string>();
                // Loop through components, check if object path exists, if so add it to the aggregate
                for (const auto& c: this->project_summary["components"])
                {
                    auto v = yaml_path(c.second, path);
                    if (!v)
                        continue;
                    
                    if (v.IsMap())
                        for (auto i: v)
                            aggregate[i.first] = i.second; //local_inja_env.render(i.second.as<std::string>(), this->project_summary);
                    else if (v.IsSequence())
                        for (auto i: v)
                            aggregate.push_back(local_inja_env.render(i.as<std::string>(), data));
                    else
                        aggregate.push_back(local_inja_env.render(v.as<std::string>(), data));
                }
                if (aggregate.IsNull())
                    return nlohmann::json();
                else
                    return aggregate.as<nlohmann::json>();
                });

            // Run template engine on dependencies
            for ( auto d : blueprint.second["depends"] )
            {
                // Check for special dependency_file condition
                if ( d.IsMap( ) )
                {
                    if ( d["dependency_file"] )
                    {
                        const std::string generated_dependency_file = try_render(local_inja_env,  d.begin()->second.Scalar(), data, log );
                        auto dependencies = parse_gcc_dependency_file(generated_dependency_file);
                        match.dependencies.insert( std::end( match.dependencies ), std::begin( dependencies ), std::end( dependencies ) );
                        continue;
                    }
                    else if (d["data"] )
                    {
                        for (const auto& data_item: d["data"])
                        {
                            std::string data_name = try_render(local_inja_env, data_item.as<std::string>(), data, log);
                            if (data_name.front() != data_dependency_identifier)
                                data_name.insert(0,1, data_dependency_identifier);
                            match.dependencies.push_back(data_name);
                        }
                        continue;
                    }
                }
                // Verify validity of dependency
                else if ( !d.IsScalar( ) )
                {
                    log->error("Dependencies should be Scalar");
                    return;
                }

                std::string depend_string = d.Scalar( );

                // Generate full dependency string by applying template engine
                std::string generated_depend;
                try
                {
                    generated_depend = local_inja_env.render( depend_string, data );
                }
                catch ( std::exception& e )
                {
                    log->error("Couldn't apply template: '{}'", depend_string);
                    return;
                }

                // Check if the input was a YAML array construct
                if ( generated_depend.front( ) == '[' && generated_depend.back( ) == ']' )
                {
                    // Load the generated dependency string as YAML and push each item individually
                    try {
                        auto generated_node = YAML::Load( generated_depend );
                        for ( auto i : generated_node )
                            match.dependencies.push_back( i.Scalar( ) );
                    } catch ( std::exception& e ) {
                        std::cerr << "Failed to parse dependency: " << depend_string << "\n";
                    }
                }
                else
                {
                    match.dependencies.push_back( generated_depend );
                }
            }

            task_database.insert(std::make_pair(target, std::move(match)));
        }

        if (!blueprint_match_found)
        {
            if (!fs::exists( target ))
                log->info("No blueprint for '{}'", target);
            // task_database.insert(std::make_pair(target, std::make_shared<blueprint_node>(target)));
        }
    }
    #endif
}