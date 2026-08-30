#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "TChain.h"
#include "TMath.h"

using namespace std;

struct ActivityResult
{
    bool valid = false;
    int run_number = -1;
    int phase = -1;
    double run_start_unix = 0.0;
    double run_duration_seconds = 0.0;
    Long64_t selected_events = 0;
    double efficiency = 0.0;
    double activity_mBq_m3 = 0.0;
    double activity_error_mBq_m3 = 0.0;
};

bool get_run_timing(
    int run_number,
    int phase,
    double& run_start_unix,
    double& run_duration_seconds)
{
    string timing_file_path;

    if (phase == 0) {
        timing_file_path =
            "/sps/nemo/snemo/snemo_data/reco_data/"
            "UDD_betabeta_v1.list";
    }
    else if (phase == 3) {
        timing_file_path =
            "/sps/nemo/snemo/snemo_data/reco_data/"
            "UDD_betabeta_v2.update";
    }
    else {
        cout << "Error: Timing lookup is supported only for Phase 0 and Phase 3."
             << endl;
        return false;
    }

    ifstream timing_file(timing_file_path);
    if (!timing_file.is_open()) {
        cout << "Error: Could not open timing file: " << timing_file_path << endl;
        return false;
    }

    string line;
    while (getline(timing_file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        istringstream line_stream(line);
        int stored_run_number = -1;
        double stored_start_unix = 0.0;
        double stored_duration_seconds = 0.0;

        if (!(line_stream >> stored_run_number
                          >> stored_start_unix
                          >> stored_duration_seconds)) {
            continue;
        }

        if (stored_run_number == run_number) {
            if (stored_duration_seconds <= 0.0) {
                cout << "Warning: Invalid duration for run " << run_number << endl;
                return false;
            }

            run_start_unix = stored_start_unix;
            run_duration_seconds = stored_duration_seconds;
            return true;
        }
    }

    cout << "Warning: No timing information found for run " << run_number
         << " in Phase " << phase << endl;
    return false;
}

int extract_run_number(const string& filename)
{
    const string marker = "run-";
    size_t start = filename.find(marker);
    if (start == string::npos) return -1;

    start += marker.size();
    const size_t end = filename.find_first_not_of("0123456789", start);
    if (end == start) return -1;

    return stoi(filename.substr(start, end - start));
}

int identify_phase(int run_number)
{
    if (run_number >= 1546 && run_number <= 1798) return 0;
    if (run_number >= 2011 && run_number <= 2183) return 1;
    if (run_number >= 2683 && run_number <= 3467) return 2;
    if (run_number >= 3470) return 3;
    return -1;
}

Long64_t process_single_run(
    const string& filename,
    bool is_simulation = false,
    int simulation_phase = -1,
    ActivityResult* activity_result = nullptr)
{
    TChain tree("Result_tree");
    if (tree.Add(filename.c_str()) <= 0) {
        cout << "Warning: Could not add file: " << filename << endl;
        return -1;
    }

    int current_run_number = -1;
    int current_phase = -1;

    if (is_simulation) {
        current_phase = simulation_phase;
        if (current_phase != 0 && current_phase != 3) {
            cout << "Error: Simulation must use Phase 0 or Phase 3 cuts." << endl;
            return -1;
        }
    }
    else {
        current_run_number = extract_run_number(filename);
        current_phase = identify_phase(current_run_number);
        if (current_run_number < 0 || current_phase < 0) {
            cout << "Error: Could not identify the run or phase for " << filename
                 << endl;
            return -1;
        }
    }

    vector<int>* pid = nullptr;
    vector<double>* energy = nullptr;
    vector<int>* om_number = nullptr;
    vector<double>* calo_tdc = nullptr;
    vector<double>* first_vertex_end_x = nullptr;
    vector<double>* first_vertex_end_y = nullptr;
    vector<double>* first_vertex_end_z = nullptr;
    vector<double>* vertex_extrapolation_calo_x = nullptr;
    vector<double>* vertex_extrapolation_calo_y = nullptr;
    vector<double>* vertex_extrapolation_calo_z = nullptr;
    vector<double>* gamma_om_x = nullptr;
    vector<double>* gamma_om_y = nullptr;
    vector<double>* gamma_om_z = nullptr;
    int electron_number = 0;
    int gamma_number = 0;

    tree.SetBranchAddress("pid", &pid);
    tree.SetBranchAddress("energy", &energy);
    tree.SetBranchAddress("electron_number", &electron_number);
    tree.SetBranchAddress("gamma_number", &gamma_number);
    tree.SetBranchAddress("calo_tdc", &calo_tdc);
    tree.SetBranchAddress("om_number", &om_number);

    if (is_simulation) {
        tree.SetBranchAddress("vertex_track_first_end_x", &first_vertex_end_x);
        tree.SetBranchAddress("vertex_track_first_end_y", &first_vertex_end_y);
        tree.SetBranchAddress("vertex_track_first_end_z", &first_vertex_end_z);
    }
    else {
        tree.SetBranchAddress("first_vertex_x", &first_vertex_end_x);
        tree.SetBranchAddress("first_vertex_y", &first_vertex_end_y);
        tree.SetBranchAddress("first_vertex_z", &first_vertex_end_z);
    }

    tree.SetBranchAddress("vertex_extrapolation_calo_x",
                          &vertex_extrapolation_calo_x);
    tree.SetBranchAddress("vertex_extrapolation_calo_y",
                          &vertex_extrapolation_calo_y);
    tree.SetBranchAddress("vertex_extrapolation_calo_z",
                          &vertex_extrapolation_calo_z);
    tree.SetBranchAddress("gamma_om_x", &gamma_om_x);
    tree.SetBranchAddress("gamma_om_y", &gamma_om_y);
    tree.SetBranchAddress("gamma_om_z", &gamma_om_z);

    double photon_pre_cut = 0.0;
    if (current_phase == 0) photon_pre_cut = 0.300;
    else if (current_phase == 3) photon_pre_cut = 0.050;
    else return -1;

    const int gamma_Pid = 0;
    const int electron_Pid = 1;
    const double c_light = 299.792458;
    const double electron_mass = 0.511;
    const double timing_cut = 12.0;
    const double total_energy_cut = 3.000;
    const double calo_tdc_min = -20.0;
    const double electron_calo_tdc_max = 300.0;
    const double gamma_calo_tdc_max = 300.0;

    Long64_t total_events_processed = 0;
    Long64_t passed_pre_cut = 0;
    Long64_t passed_topology = 0;
    Long64_t passed_calo_tdc = 0;
    Long64_t passed_geometry = 0;
    Long64_t passed_timing = 0;
    Long64_t passed_energy = 0;

    const Long64_t total_events = tree.GetEntries();

    for (Long64_t i = 0; i < total_events; ++i) {
        tree.GetEntry(i);
        ++total_events_processed;

        if (!pid || !energy || pid->size() != energy->size()) {
            continue;
        }

        bool event_survives_pre_cut = false;
        bool event_survives_topology_cut = false;
        bool event_survives_calo_tdc_cut = false;
        bool event_survives_geometry_cut = false;
        bool event_survives_timing_cut = false;
        bool event_survives_energy_cut = false;

        // 1. Phase-dependent photon-energy pre-cut
        for (size_t j = 0; j < pid->size(); ++j) {
            if (pid->at(j) == gamma_Pid) {
                const double E_gamma = energy->at(j);
                if (E_gamma > photon_pre_cut) {
                    event_survives_pre_cut = true;
                }
            }
        }

        // 2. Exactly one electron and one gamma
        if (electron_number == 1 && gamma_number == 1 && pid->size() == 2) {
            event_survives_topology_cut = true;
        }

        // 3. Calorimeter TDC cut
        if (event_survives_topology_cut && calo_tdc &&
            calo_tdc->size() == pid->size()) {
            int electron_index = -1;
            int gamma_index = -1;

            for (size_t j = 0; j < pid->size(); ++j) {
                if (pid->at(j) == electron_Pid) electron_index = j;
                else if (pid->at(j) == gamma_Pid) gamma_index = j;
            }

            if (electron_index >= 0 && gamma_index >= 0) {
                const double electron_tdc = calo_tdc->at(electron_index);
                const double gamma_tdc = calo_tdc->at(gamma_index);

                const bool electron_passes =
                    electron_tdc >= calo_tdc_min &&
                    electron_tdc <= electron_calo_tdc_max;
                const bool gamma_passes =
                    gamma_tdc >= calo_tdc_min &&
                    gamma_tdc <= gamma_calo_tdc_max;

                if (electron_passes && gamma_passes) {
                    event_survives_calo_tdc_cut = true;
                }
            }
        }

        // 4. Geometry cuts
        if (first_vertex_end_x && first_vertex_end_y && om_number &&
            first_vertex_end_x->size() > 0 &&
            first_vertex_end_y->size() > 0 && om_number->size() > 0) {
            const double x = first_vertex_end_x->at(0);
            const double y = first_vertex_end_y->at(0);
            const bool x_pass =
                TMath::Abs(x) > 77.0 && TMath::Abs(x) < 337.0;
            const bool y_pass = y > -2000.0 && y < 1900.0;
            bool main_wall_pass = true;

            for (size_t j = 0; j < om_number->size(); ++j) {
                if (om_number->at(j) < 0 || om_number->at(j) >= 520) {
                    main_wall_pass = false;
                    break;
                }
            }

            if (x_pass && y_pass && main_wall_pass) {
                event_survives_geometry_cut = true;
            }
        }

        // 5. Time-of-flight calculation and timing cut
        if (event_survives_topology_cut && calo_tdc &&
            first_vertex_end_x && first_vertex_end_y && first_vertex_end_z &&
            vertex_extrapolation_calo_x && vertex_extrapolation_calo_y &&
            vertex_extrapolation_calo_z && gamma_om_x && gamma_om_y && gamma_om_z &&
            calo_tdc->size() == pid->size() &&
            energy->size() == pid->size() &&
            first_vertex_end_x->size() > 0 &&
            first_vertex_end_y->size() > 0 &&
            first_vertex_end_z->size() > 0 &&
            vertex_extrapolation_calo_x->size() > 0 &&
            vertex_extrapolation_calo_y->size() > 0 &&
            vertex_extrapolation_calo_z->size() > 0 &&
            gamma_om_x->size() > 0 && gamma_om_y->size() > 0 &&
            gamma_om_z->size() > 0) {
            int electron_index = -1;
            int gamma_index = -1;

            for (size_t j = 0; j < pid->size(); ++j) {
                if (pid->at(j) == electron_Pid) electron_index = j;
                if (pid->at(j) == gamma_Pid) gamma_index = j;
            }

            if (electron_index >= 0 && gamma_index >= 0 &&
                gamma_index < static_cast<int>(gamma_om_x->size()) &&
                gamma_index < static_cast<int>(gamma_om_y->size()) &&
                gamma_index < static_cast<int>(gamma_om_z->size())) {
                const double E_electron = energy->at(electron_index);

                if (E_electron > 0) {
                    const double gamma_rel = 1.0 + E_electron / electron_mass;
                    const double beta =
                        TMath::Sqrt(1.0 - 1.0 / (gamma_rel * gamma_rel));

                    if (beta > 0) {
                        const double dx_e = first_vertex_end_x->at(0) -
                                            vertex_extrapolation_calo_x->at(0);
                        const double dy_e = first_vertex_end_y->at(0) -
                                            vertex_extrapolation_calo_y->at(0);
                        const double dz_e = first_vertex_end_z->at(0) -
                                            vertex_extrapolation_calo_z->at(0);
                        const double Le =
                            TMath::Sqrt(dx_e * dx_e + dy_e * dy_e + dz_e * dz_e);

                        const double gamma_x = gamma_om_x->at(gamma_index);
                        const double gamma_y = gamma_om_y->at(gamma_index);
                        const double gamma_z = gamma_om_z->at(gamma_index);

                        if (gamma_x != -114514 && gamma_y != -114514 &&
                            gamma_z != -114514) {
                            const double dx_g = first_vertex_end_x->at(0) - gamma_x;
                            const double dy_g = first_vertex_end_y->at(0) - gamma_y;
                            const double dz_g = first_vertex_end_z->at(0) - gamma_z;
                            const double Lg = TMath::Sqrt(
                                dx_g * dx_g + dy_g * dy_g + dz_g * dz_g);
                            const double electron_TOF = Le / (beta * c_light);
                            const double gamma_TOF = Lg / c_light;
                            const double te = calo_tdc->at(electron_index) -
                                              electron_TOF;
                            const double tg = calo_tdc->at(gamma_index) - gamma_TOF;
                            const double delta_t = TMath::Abs(te - tg);

                            if (delta_t < timing_cut) {
                                event_survives_timing_cut = true;
                            }
                        }
                    }
                }
            }
        }

        // 6. Total-energy cut
        if (event_survives_topology_cut) {
            double E_total = 0.0;
            for (size_t j = 0; j < pid->size(); ++j) {
                if (pid->at(j) == electron_Pid || pid->at(j) == gamma_Pid) {
                    E_total += energy->at(j);
                }
            }
            if (E_total < total_energy_cut) {
                event_survives_energy_cut = true;
            }
        }

        // Sequential cut flow: pre-cut -> topology -> TDC -> geometry -> timing -> energy
        if (event_survives_pre_cut) {
            ++passed_pre_cut;
            if (event_survives_topology_cut) {
                ++passed_topology;
                if (event_survives_calo_tdc_cut) {
                    ++passed_calo_tdc;
                    if (event_survives_geometry_cut) {
                        ++passed_geometry;
                        if (event_survives_timing_cut) {
                            ++passed_timing;
                            if (event_survives_energy_cut) {
                                ++passed_energy;
                            }
                        }
                    }
                }
            }
        }
    }

    // 7. Efficiency and activity calculation
    if (!is_simulation && activity_result &&
        (current_phase == 0 || current_phase == 3)) {
        const double detector_volume_m3 = 15.4;
        const double Bq_to_mBq = 1000.0;
        const double selection_efficiency =
    current_phase == 0 ? 0.03191 : 0.03366;
        double run_start_unix = 0.0;
        double run_duration_seconds = 0.0;

        if (get_run_timing(current_run_number, current_phase,
                           run_start_unix, run_duration_seconds)) {
            activity_result->valid = true;
            activity_result->run_number = current_run_number;
            activity_result->phase = current_phase;
            activity_result->run_start_unix = run_start_unix;
            activity_result->run_duration_seconds = run_duration_seconds;
            activity_result->selected_events = passed_energy;
            activity_result->efficiency = selection_efficiency;
            activity_result->activity_mBq_m3 =
                static_cast<double>(passed_energy) /
                (run_duration_seconds * detector_volume_m3 * selection_efficiency) *
                Bq_to_mBq;
            activity_result->activity_error_mBq_m3 =
                std::sqrt(static_cast<double>(passed_energy)) /
                (run_duration_seconds * detector_volume_m3 * selection_efficiency) *
                Bq_to_mBq;
        }
    }

    // 8. Simple cut-flow summary
    cout << "\nCut-flow summary for " << filename << '\n'
         << "Total events:       " << total_events_processed << '\n'
         << "After pre-cut:      " << passed_pre_cut << '\n'
         << "After topology:     " << passed_topology << '\n'
         << "After calo TDC:     " << passed_calo_tdc << '\n'
         << "After geometry:     " << passed_geometry << '\n'
         << "After timing:       " << passed_timing << '\n'
         << "After total energy: " << passed_energy << endl;

    return passed_energy;
}

void CUTzz(
    const char* file_list = "input_files.txt",
    int requested_phase = 0)
{
    if (requested_phase != 0 && requested_phase != 3) {
        cout << "Error: Only Phase 0 and Phase 3 are supported." << endl;
        return;
    }

    ifstream input_list(file_list);
    if (!input_list.is_open()) {
        cout << "Error: Cannot open input file list: " << file_list << endl;
        return;
    }

    string filename;
    while (getline(input_list, filename)) {
        if (filename.empty()) continue;

        const int run_number = extract_run_number(filename);
        if (identify_phase(run_number) != requested_phase) continue;

        ActivityResult result;
        process_single_run(filename, false, -1, &result);

        if (result.valid) {
            cout << "Activity result for run " << result.run_number << '\n'
                 << "Run duration: " << result.run_duration_seconds << " s\n"
                 << "Selected events: " << result.selected_events << '\n'
                 << "Efficiency used: " << result.efficiency << '\n'
                 << "Radon activity: " << result.activity_mBq_m3
                 << " mBq/m^3\n"
                 << endl;
        }
    }
}

void calculate_simulation_efficiencies(
    const char* simulation_file_list = "simulation_input_files.txt")
{
    ifstream input_list(simulation_file_list);
    if (!input_list.is_open()) {
        cout << "Error: Cannot open simulation input file list: "
             << simulation_file_list << endl;
        return;
    }

    vector<string> simulation_files;
    string filename;

    while (getline(input_list, filename)) {
        if (filename.empty()) continue;
        simulation_files.push_back(filename);
    }

    const Long64_t expected_number_of_files = 20;
    const Long64_t generated_events_per_file = 1000000LL;

    if (static_cast<Long64_t>(simulation_files.size()) !=
        expected_number_of_files) {
        cout << "Error: Expected exactly " << expected_number_of_files
             << " simulation files, but found " << simulation_files.size()
             << endl;
        return;
    }

    const Long64_t total_generated_events =
        expected_number_of_files * generated_events_per_file;
    Long64_t total_selected[2] = {0, 0};
    const int phases[2] = {0, 3};

    for (int phase_index = 0; phase_index < 2; ++phase_index) {
        Long64_t files_processed = 0;

        for (const string& simulation_file : simulation_files) {
            const Long64_t selected =
                process_single_run(simulation_file, true, phases[phase_index]);

            if (selected < 0) continue;
            total_selected[phase_index] += selected;
            ++files_processed;
        }

        if (files_processed != expected_number_of_files) {
            cout << "Error: Only " << files_processed << " of "
                 << expected_number_of_files << " simulation files were processed."
                 << endl;
            return;
        }
    }

    const double phase0_efficiency =
        static_cast<double>(total_selected[0]) /
        static_cast<double>(total_generated_events);
    const double phase3_efficiency =
        static_cast<double>(total_selected[1]) /
        static_cast<double>(total_generated_events);

    cout << "\nFinal simulation efficiency results\n"
         << "Total generated Bi-214 events: " << total_generated_events << '\n'
         << "Phase 0 selected events: " << total_selected[0] << '\n'
         << "Phase 0 efficiency: " << phase0_efficiency << '\n'
         << "Phase 0 efficiency (%): " << phase0_efficiency * 100.0 << "%\n"
         << "Phase 3 selected events: " << total_selected[1] << '\n'
         << "Phase 3 efficiency: " << phase3_efficiency << '\n'
         << "Phase 3 efficiency (%): " << phase3_efficiency * 100.0 << '%'
         << endl;
}
