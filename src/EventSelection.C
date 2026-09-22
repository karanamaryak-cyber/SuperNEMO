#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "TChain.h"
#include "TMath.h"

using namespace std;

// Helper functions defined in RunUtilities.C
int extract_run_number(const string& filename);
int identify_phase(int run_number);

Long64_t process_single_run(
    const string& filename,
    bool is_simulation = false,
    int simulation_phase = -1)
{
    TChain tree("Result_tree");

    if (tree.Add(filename.c_str()) <= 0) {
        cout << "Warning: Could not add file: "
             << filename << endl;
        return -1;
    }

    int current_run_number = -1;
    int current_phase = -1;

    if (is_simulation) {

        current_phase = simulation_phase;

        if (current_phase != 0 && current_phase != 3) {
            cout << "Error: Simulation must use Phase 0 or Phase 3 cuts."
                 << endl;
            return -1;
        }
    }
    else {

        current_run_number = extract_run_number(filename);
        current_phase = identify_phase(current_run_number);

        if (current_run_number < 0 || current_phase < 0) {
            cout << "Error: Could not identify the run or phase for "
                 << filename << endl;
            return -1;
        }
    }

    // ROOT branch variables
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

    // Connect variables to branches in Result_tree
    tree.SetBranchAddress("pid", &pid);
    tree.SetBranchAddress("energy", &energy);
    tree.SetBranchAddress("electron_number", &electron_number);
    tree.SetBranchAddress("gamma_number", &gamma_number);
    tree.SetBranchAddress("calo_tdc", &calo_tdc);
    tree.SetBranchAddress("om_number", &om_number);

    if (is_simulation) {
        tree.SetBranchAddress(
            "vertex_track_first_end_x",
            &first_vertex_end_x);

        tree.SetBranchAddress(
            "vertex_track_first_end_y",
            &first_vertex_end_y);

        tree.SetBranchAddress(
            "vertex_track_first_end_z",
            &first_vertex_end_z);
    }
    else {
        tree.SetBranchAddress("first_vertex_x", &first_vertex_end_x);
        tree.SetBranchAddress("first_vertex_y", &first_vertex_end_y);
        tree.SetBranchAddress("first_vertex_z", &first_vertex_end_z);
    }

    tree.SetBranchAddress(
        "vertex_extrapolation_calo_x",
        &vertex_extrapolation_calo_x);

    tree.SetBranchAddress(
        "vertex_extrapolation_calo_y",
        &vertex_extrapolation_calo_y);

    tree.SetBranchAddress(
        "vertex_extrapolation_calo_z",
        &vertex_extrapolation_calo_z);

    tree.SetBranchAddress("gamma_om_x", &gamma_om_x);
    tree.SetBranchAddress("gamma_om_y", &gamma_om_y);
    tree.SetBranchAddress("gamma_om_z", &gamma_om_z);

    // Phase-dependent photon energy pre-cut
    double photon_pre_cut = 0.0;

    if (current_phase == 0)
        photon_pre_cut = 0.300;
    else if (current_phase == 3)
        photon_pre_cut = 0.050;
    else
        return -1;

    // Particle IDs and physical constants
    const int gamma_Pid = 0;
    const int electron_Pid = 1;

    const double c_light = 299.792458;   // mm/ns
    const double electron_mass = 0.511;  // MeV

    // Selection cuts
    const double timing_cut = 12.0;       // ns
    const double total_energy_cut = 3.000; // MeV

    const double calo_tdc_min = -20.0;    // ns
    const double calo_tdc_max = 300.0;    // ns

    // Cut-flow counters
    Long64_t total_events_processed = 0;
    Long64_t passed_pre_cut = 0;
    Long64_t passed_topology = 0;
    Long64_t passed_calo_tdc = 0;
    Long64_t passed_geometry = 0;
    Long64_t passed_timing = 0;
    Long64_t passed_energy = 0;

    const Long64_t number_of_entries = tree.GetEntries();

    for (Long64_t entry = 0; entry < number_of_entries; ++entry) {

        tree.GetEntry(entry);
        ++total_events_processed;

        bool event_survives_pre_cut = false;
        bool event_survives_topology_cut = false;
        bool event_survives_calo_tdc_cut = false;
        bool event_survives_geometry_cut = false;
        bool event_survives_timing_cut = false;
        bool event_survives_energy_cut = false;

        // ==================================================
        // 1. Photon-energy pre-cut
        // ==================================================
        for (size_t j = 0; j < pid->size(); ++j) {

            if (pid->at(j) == gamma_Pid &&
                energy->at(j) > photon_pre_cut) {

                event_survives_pre_cut = true;
                break;
            }
        }

        // ==================================================
        // 2. 1e1gamma topology cut
        // ==================================================
        if (event_survives_pre_cut &&
            electron_number == 1 &&
            gamma_number == 1 &&
            pid->size() == 2) {

            event_survives_topology_cut = true;
        }

        // ==================================================
        // 3. Calorimeter TDC cut
        // ==================================================
        if (event_survives_topology_cut) {

            bool valid_tdc = true;

            for (size_t j = 0; j < calo_tdc->size(); ++j) {

                if (calo_tdc->at(j) < calo_tdc_min ||
                    calo_tdc->at(j) > calo_tdc_max) {

                    valid_tdc = false;
                    break;
                }
            }

            if (valid_tdc)
                event_survives_calo_tdc_cut = true;
        }


        // ==================================================
        // 4. Geometry cut
        // ==================================================
        if (event_survives_calo_tdc_cut) {

            const double vertex_x = first_vertex_end_x->at(0);
            const double vertex_y = first_vertex_end_y->at(0);

            const bool valid_x =
                TMath::Abs(vertex_x) > 77.0 &&
                TMath::Abs(vertex_x) < 337.0;

            const bool valid_y =
                vertex_y > -2000.0 &&
                vertex_y < 1900.0;

            bool valid_om = true;

            for (size_t j = 0; j < om_number->size(); ++j) {

                if (om_number->at(j) < 0 ||
                    om_number->at(j) > 519) {

                    valid_om = false;
                    break;
                }
            }

            if (valid_x && valid_y && valid_om)
                event_survives_geometry_cut = true;
        }

        // ==================================================
        // 5. Time-of-flight / timing cut
        // ==================================================
        if (event_survives_geometry_cut) {

            int electron_index = -1;
            int gamma_index = -1;

            for (size_t j = 0; j < pid->size(); ++j) {

                if (pid->at(j) == electron_Pid)
                    electron_index = static_cast<int>(j);

                if (pid->at(j) == gamma_Pid)
                    gamma_index = static_cast<int>(j);
            }

            if (electron_index >= 0 && gamma_index >= 0) {

                const double electron_energy =
                    energy->at(electron_index);

                const double total_electron_energy =
                    electron_energy + electron_mass;

                const double momentum =
                    TMath::Sqrt(
                        total_electron_energy * total_electron_energy -
                        electron_mass * electron_mass);

                const double beta =
                    momentum / total_electron_energy;

                if (beta > 0.0) {

                    const double dx_e =
                        first_vertex_end_x->at(0) -
                        vertex_extrapolation_calo_x->at(0);

                    const double dy_e =
                        first_vertex_end_y->at(0) -
                        vertex_extrapolation_calo_y->at(0);

                    const double dz_e =
                        first_vertex_end_z->at(0) -
                        vertex_extrapolation_calo_z->at(0);

                    const double Le =
                        TMath::Sqrt(
                            dx_e * dx_e +
                            dy_e * dy_e +
                            dz_e * dz_e);

                    const double gamma_x =
                        gamma_om_x->at(gamma_index);

                    const double gamma_y =
                        gamma_om_y->at(gamma_index);

                    const double gamma_z =
                        gamma_om_z->at(gamma_index);

                    if (gamma_x != -114514 &&
                        gamma_y != -114514 &&
                        gamma_z != -114514) {

                        const double dx_g =
                            first_vertex_end_x->at(0) - gamma_x;

                        const double dy_g =
                            first_vertex_end_y->at(0) - gamma_y;

                        const double dz_g =
                            first_vertex_end_z->at(0) - gamma_z;

                        const double Lg =
                            TMath::Sqrt(
                                dx_g * dx_g +
                                dy_g * dy_g +
                                dz_g * dz_g);

                        const double electron_TOF =
                            Le / (beta * c_light);

                        const double gamma_TOF =
                            Lg / c_light;

                        const double te =
                            calo_tdc->at(electron_index) -
                            electron_TOF;

                        const double tg =
                            calo_tdc->at(gamma_index) -
                            gamma_TOF;

                        const double delta_t =
                            TMath::Abs(te - tg);

                        if (delta_t < timing_cut)
                            event_survives_timing_cut = true;
                    }
                }
            }
        }

        // ==================================================
        // 6. Total-energy cut
        // ==================================================
        if (event_survives_timing_cut) {

            double E_total = 0.0;

            for (size_t j = 0; j < pid->size(); ++j) {

                if (pid->at(j) == electron_Pid ||
                    pid->at(j) == gamma_Pid) {

                    E_total += energy->at(j);
                }
            }

            if (E_total < total_energy_cut)
                event_survives_energy_cut = true;
        }


        // ==================================================
        // Sequential cut flow
        // ==================================================
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


    // ======================================================
    // Cut-flow summary
    // ======================================================
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
