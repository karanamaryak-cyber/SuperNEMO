#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

// ==========================================================
// Retrieve run start time and duration
// ==========================================================
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
        cout << "Error: Could not open timing file: "
             << timing_file_path << endl;
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
                cout << "Warning: Invalid duration for run "
                     << run_number << endl;
                return false;
            }

            run_start_unix = stored_start_unix;
            run_duration_seconds = stored_duration_seconds;

            return true;
        }
    }

    cout << "Warning: No timing information found for run "
         << run_number << " in Phase " << phase << endl;

    return false;
}


// ==========================================================
// Extract run number from SuperNEMO filename
// ==========================================================
int extract_run_number(const string& filename)
{
    const string marker = "run-";

    size_t start = filename.find(marker);

    if (start == string::npos)
        return -1;

    start += marker.size();

    const size_t end =
        filename.find_first_not_of("0123456789", start);

    if (end == start)
        return -1;

    return stoi(filename.substr(start, end - start));
}


// ==========================================================
// Identify commissioning phase from run number
// ==========================================================
int identify_phase(int run_number)
{
    if (run_number >= 1546 && run_number <= 1798)
        return 0;

    if (run_number >= 2011 && run_number <= 2183)
        return 1;

    if (run_number >= 2683 && run_number <= 3467)
        return 2;

    if (run_number >= 3470)
        return 3;

    return -1;
}
