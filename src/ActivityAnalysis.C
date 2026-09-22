#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "RtypesCore.h"

using namespace std;

// Functions defined elsewhere
bool get_run_timing(
    int run_number,
    int phase,
    double& run_start_unix,
    double& run_duration_seconds);

int extract_run_number(const string& filename);
int identify_phase(int run_number);

Long64_t process_single_run(
    const string& filename,
    bool is_simulation,
    int simulation_phase);


// Stores the calculated activity for an individual run
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

ActivityResult calculate_activity(
    const string& filename,
    double efficiency)
{
    ActivityResult result;

    const double detector_volume = 15.4;  // m^3

    result.run_number = extract_run_number(filename);
    result.phase = identify_phase(result.run_number);

    if (result.run_number < 0 || result.phase < 0) {
        cout << "Error: Could not identify run or phase." << endl;
        return result;
    }

    if (!get_run_timing(
            result.run_number,
            result.phase,
            result.run_start_unix,
            result.run_duration_seconds)) {

        cout << "Error: Could not obtain timing information for run "
             << result.run_number << endl;

        return result;
    }

    result.selected_events =
        process_single_run(filename, false, -1);

    if (result.selected_events < 0 ||
        result.run_duration_seconds <= 0.0 ||
        efficiency <= 0.0) {

        return result;
    }

    result.efficiency = efficiency;

    result.activity_mBq_m3 =
        (static_cast<double>(result.selected_events) /
         (result.run_duration_seconds *
          detector_volume *
          efficiency)) * 1000.0;

    result.activity_error_mBq_m3 =
        (sqrt(static_cast<double>(result.selected_events)) /
         (result.run_duration_seconds *
          detector_volume *
          efficiency)) * 1000.0;

    result.valid = true;

    return result;
}

// Selection efficiencies from simulation
const double phase0_efficiency = 0.03191;
const double phase3_efficiency = 0.03366;


void CUTzz(
    const string& file_list = "input_files.txt",
    int requested_phase = 0)
{
    if (requested_phase != 0 && requested_phase != 3) {
        cout << "Error: requested_phase must be 0 or 3." << endl;
        return;
    }

    const double efficiency =
        (requested_phase == 0)
        ? phase0_efficiency
        : phase3_efficiency;

    ifstream input(file_list);

    if (!input.is_open()) {
        cout << "Error: Could not open file list: "
             << file_list << endl;
        return;
    }

    string filename;

    while (getline(input, filename)) {

        if (filename.empty())
            continue;

        const int run_number = extract_run_number(filename);
        const int phase = identify_phase(run_number);

        if (phase != requested_phase)
            continue;

        ActivityResult result =
            calculate_activity(filename, efficiency);

        if (!result.valid)
            continue;

        cout << "\nRun " << result.run_number
             << " | Selected events: " << result.selected_events
             << " | Efficiency: " << result.efficiency
             << " | Activity: " << result.activity_mBq_m3
             << " +/- " << result.activity_error_mBq_m3
             << " mBq/m^3" << endl;
    }
}

void calculate_simulation_efficiencies(
    const vector<string>& phase0_files,
    const vector<string>& phase3_files)
{
    const Long64_t events_per_file = 1000000;

    Long64_t phase0_selected = 0;
    Long64_t phase3_selected = 0;

    // Phase 0 simulation
    for (const string& filename : phase0_files) {

        Long64_t selected =
            process_single_run(filename, true, 0);

        if (selected >= 0)
            phase0_selected += selected;
    }

    // Phase 3 simulation
    for (const string& filename : phase3_files) {

        Long64_t selected =
            process_single_run(filename, true, 3);

        if (selected >= 0)
            phase3_selected += selected;
    }

    const Long64_t phase0_generated =
        events_per_file * phase0_files.size();

    const Long64_t phase3_generated =
        events_per_file * phase3_files.size();

    const double calculated_phase0_efficiency =
        (phase0_generated > 0)
        ? static_cast<double>(phase0_selected) / phase0_generated
        : 0.0;

    const double calculated_phase3_efficiency =
        (phase3_generated > 0)
        ? static_cast<double>(phase3_selected) / phase3_generated
        : 0.0;

    cout << "\nSimulation efficiency summary" << endl;

    cout << "Phase 0: "
         << calculated_phase0_efficiency
         << " (" << calculated_phase0_efficiency * 100.0
         << "%)" << endl;

    cout << "Phase 3: "
         << calculated_phase3_efficiency
         << " (" << calculated_phase3_efficiency * 100.0
         << "%)" << endl;
}
