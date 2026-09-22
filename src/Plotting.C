#include <iostream>
#include <string>
#include <vector>

#include "TCanvas.h"
#include "TChain.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLegend.h"
#include "TLine.h"
#include "TMath.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TF1.h"

using namespace std;

void plot_photon_energy(
    const string& filename,
    int phase)
{
    TChain tree("Result_tree");

    if (tree.Add(filename.c_str()) <= 0) {
        cout << "Could not open: " << filename << endl;
        return;
    }

    vector<int>* pid = nullptr;
    vector<double>* energy = nullptr;

    tree.SetBranchAddress("pid", &pid);
    tree.SetBranchAddress("energy", &energy);

    const int gamma_Pid = 0;

    const double photon_pre_cut =
        (phase == 0) ? 0.300 : 0.050;

    TH1D* hPhotonEnergy =
        new TH1D(
            "hPhotonEnergy",
            "Photon Energy;Photon Energy [MeV];Events",
            100, 0.0, 3.0);

    const Long64_t entries = tree.GetEntries();

    for (Long64_t entry = 0; entry < entries; ++entry) {

        tree.GetEntry(entry);

        for (size_t j = 0; j < pid->size(); ++j) {

            if (pid->at(j) == gamma_Pid)
                hPhotonEnergy->Fill(energy->at(j));
        }
    }

    TCanvas* canvas =
        new TCanvas("cPhotonEnergy", "Photon Energy", 800, 600);

    hPhotonEnergy->Draw();

    TLine* cutLine =
        new TLine(
            photon_pre_cut,
            0.0,
            photon_pre_cut,
            hPhotonEnergy->GetMaximum());

    cutLine->SetLineStyle(2);
    cutLine->SetLineWidth(2);
    cutLine->Draw("same");

    canvas->SaveAs(
        phase == 0
        ? "photon_energy_phase0.png"
        : "photon_energy_phase3.png");
}

void plot_calo_tdc(const string& filename)
{
    TChain tree("Result_tree");

    if (tree.Add(filename.c_str()) <= 0) {
        cout << "Could not open: " << filename << endl;
        return;
    }

    vector<double>* calo_tdc = nullptr;
    tree.SetBranchAddress("calo_tdc", &calo_tdc);

    TH1D* hCaloTDC =
        new TH1D(
            "hCaloTDC",
            "Calorimeter TDC;Calorimeter TDC [ns];Entries",
            160, -50.0, 350.0);

    const Long64_t entries = tree.GetEntries();

    for (Long64_t entry = 0; entry < entries; ++entry) {

        tree.GetEntry(entry);

        for (double tdc : *calo_tdc)
            hCaloTDC->Fill(tdc);
    }

    TCanvas* canvas =
        new TCanvas("cCaloTDC", "Calorimeter TDC", 800, 600);

    hCaloTDC->Draw();

    // TDC acceptance window: -20 ns to 300 ns
    TLine* lowerCut =
        new TLine(-20.0, 0.0, -20.0, hCaloTDC->GetMaximum());

    TLine* upperCut =
        new TLine(300.0, 0.0, 300.0, hCaloTDC->GetMaximum());

    lowerCut->SetLineStyle(2);
    upperCut->SetLineStyle(2);

    lowerCut->SetLineWidth(2);
    upperCut->SetLineWidth(2);

    lowerCut->Draw("same");
    upperCut->Draw("same");

    canvas->SaveAs("calorimeter_tdc.png");
}

void plot_vertex_geometry(const string& filename)
{
    TChain tree("Result_tree");

    if (tree.Add(filename.c_str()) <= 0) {
        cout << "Could not open: " << filename << endl;
        return;
    }

    vector<double>* first_vertex_x = nullptr;
    vector<double>* first_vertex_y = nullptr;

    tree.SetBranchAddress("first_vertex_x", &first_vertex_x);
    tree.SetBranchAddress("first_vertex_y", &first_vertex_y);

    TH2D* hVertex =
        new TH2D(
            "hVertex",
            "Reconstructed Vertex Position;Vertex x [mm];Vertex y [mm]",
            100, -400.0, 400.0,
            100, -2200.0, 2200.0);

    const Long64_t entries = tree.GetEntries();

    for (Long64_t entry = 0; entry < entries; ++entry) {

        tree.GetEntry(entry);

        if (!first_vertex_x->empty() &&
            !first_vertex_y->empty()) {

            hVertex->Fill(
                first_vertex_x->at(0),
                first_vertex_y->at(0));
        }
    }

    TCanvas* canvas =
        new TCanvas("cVertex", "Vertex Geometry", 900, 700);

    hVertex->Draw("COLZ");

    canvas->SaveAs("vertex_geometry.png");
}

void plot_timing_difference(const string& filename)
{
    TChain tree("Result_tree");

    if (tree.Add(filename.c_str()) <= 0) {
        cout << "Could not open: " << filename << endl;
        return;
    }

    vector<int>* pid = nullptr;
    vector<double>* energy = nullptr;
    vector<double>* calo_tdc = nullptr;

    vector<double>* first_vertex_x = nullptr;
    vector<double>* first_vertex_y = nullptr;
    vector<double>* first_vertex_z = nullptr;

    vector<double>* vertex_extrapolation_calo_x = nullptr;
    vector<double>* vertex_extrapolation_calo_y = nullptr;
    vector<double>* vertex_extrapolation_calo_z = nullptr;

    vector<double>* gamma_om_x = nullptr;
    vector<double>* gamma_om_y = nullptr;
    vector<double>* gamma_om_z = nullptr;

    tree.SetBranchAddress("pid", &pid);
    tree.SetBranchAddress("energy", &energy);
    tree.SetBranchAddress("calo_tdc", &calo_tdc);

    tree.SetBranchAddress("first_vertex_x", &first_vertex_x);
    tree.SetBranchAddress("first_vertex_y", &first_vertex_y);
    tree.SetBranchAddress("first_vertex_z", &first_vertex_z);

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

    TH1D* hDeltaT =
        new TH1D(
            "hDeltaT",
            "TOF-Corrected Timing Difference;|#Delta t| [ns];Events",
            100, 0.0, 50.0);

    const double c_light = 299.792458;
    const double electron_mass = 0.511;

    const Long64_t entries = tree.GetEntries();

    for (Long64_t entry = 0; entry < entries; ++entry) {

        tree.GetEntry(entry);

        int electron_index = -1;
        int gamma_index = -1;

        for (size_t j = 0; j < pid->size(); ++j) {
            if (pid->at(j) == 1)
                electron_index = j;

            if (pid->at(j) == 0)
                gamma_index = j;
        }

        if (electron_index < 0 || gamma_index < 0)
            continue;

        const double total_electron_energy =
            energy->at(electron_index) + electron_mass;

        const double momentum =
            TMath::Sqrt(
                total_electron_energy * total_electron_energy -
                electron_mass * electron_mass);

        const double beta = momentum / total_electron_energy;

        if (beta <= 0.0)
            continue;

        const double Le = TMath::Sqrt(
            TMath::Power(first_vertex_x->at(0) -
                         vertex_extrapolation_calo_x->at(0), 2) +
            TMath::Power(first_vertex_y->at(0) -
                         vertex_extrapolation_calo_y->at(0), 2) +
            TMath::Power(first_vertex_z->at(0) -
                         vertex_extrapolation_calo_z->at(0), 2));

          if (gamma_om_x->at(gamma_index) == -114514 ||
              gamma_om_y->at(gamma_index) == -114514 ||
              gamma_om_z->at(gamma_index) == -114514)
              continue;

        const double Lg = TMath::Sqrt(
            TMath::Power(first_vertex_x->at(0) -
                         gamma_om_x->at(gamma_index), 2) +
            TMath::Power(first_vertex_y->at(0) -
                         gamma_om_y->at(gamma_index), 2) +
            TMath::Power(first_vertex_z->at(0) -
                         gamma_om_z->at(gamma_index), 2));

        const double te =
            calo_tdc->at(electron_index) -
            Le / (beta * c_light);

        const double tg =
            calo_tdc->at(gamma_index) -
            Lg / c_light;

        hDeltaT->Fill(TMath::Abs(te - tg));
    }

    TCanvas* canvas =
        new TCanvas("cDeltaT", "Timing Difference", 800, 600);

    hDeltaT->Draw();

    TLine* timingCut =
        new TLine(12.0, 0.0, 12.0, hDeltaT->GetMaximum());

    timingCut->SetLineStyle(2);
    timingCut->SetLineWidth(2);
    timingCut->Draw("same");

    canvas->SaveAs("timing_difference.png");
}

void plot_total_energy(const string& filename)
{
    TChain tree("Result_tree");

    if (tree.Add(filename.c_str()) <= 0) {
        cout << "Could not open: " << filename << endl;
        return;
    }

    vector<int>* pid = nullptr;
    vector<double>* energy = nullptr;

    tree.SetBranchAddress("pid", &pid);
    tree.SetBranchAddress("energy", &energy);

    TH1D* hTotalEnergy =
        new TH1D(
            "hTotalEnergy",
            "Total Electron + Photon Energy;Total Energy [MeV];Events",
            120, 0.0, 6.0);

    const Long64_t entries = tree.GetEntries();

    for (Long64_t entry = 0; entry < entries; ++entry) {

        tree.GetEntry(entry);

        double total_energy = 0.0;
        bool has_electron = false;
        bool has_gamma = false;

        for (size_t j = 0; j < pid->size(); ++j) {

            if (pid->at(j) == 1) {
                total_energy += energy->at(j);
                has_electron = true;
            }

            if (pid->at(j) == 0) {
                total_energy += energy->at(j);
                has_gamma = true;
            }
        }

        if (has_electron && has_gamma)
            hTotalEnergy->Fill(total_energy);
    }

    TCanvas* canvas =
        new TCanvas("cTotalEnergy", "Total Energy", 800, 600);

    hTotalEnergy->Draw();

    TLine* energyCut =
        new TLine(
            3.0, 0.0,
            3.0, hTotalEnergy->GetMaximum());

    energyCut->SetLineStyle(2);
    energyCut->SetLineWidth(2);
    energyCut->Draw("same");

    canvas->SaveAs("total_energy.png");
}

void plot_activity_comparison(
    const vector<double>& run_times,
    const vector<double>& gamma_activities,
    const vector<double>& gamma_errors,
    const vector<double>& bipo_activities,
    const vector<double>& bipo_errors,
    int phase)
{
    const int n = run_times.size();

    if (n == 0 ||
        gamma_activities.size() != n ||
        gamma_errors.size() != n ||
        bipo_activities.size() != n ||
        bipo_errors.size() != n) {

        cout << "Invalid activity comparison data." << endl;
        return;
    }

    vector<double> zero_x_errors(n, 0.0);

    TGraphErrors* gamma_graph =
        new TGraphErrors(
            n,
            run_times.data(),
            gamma_activities.data(),
            zero_x_errors.data(),
            gamma_errors.data());

    TGraphErrors* bipo_graph =
        new TGraphErrors(
            n,
            run_times.data(),
            bipo_activities.data(),
            zero_x_errors.data(),
            bipo_errors.data());

    gamma_graph->SetMarkerStyle(20);
    gamma_graph->SetMarkerColor(kBlue + 1);
    gamma_graph->SetLineColor(kBlue + 1);

    bipo_graph->SetMarkerStyle(21);
    bipo_graph->SetMarkerColor(kRed + 1);
    bipo_graph->SetLineColor(kRed + 1);

    TMultiGraph* comparison =
        new TMultiGraph();

    comparison->Add(gamma_graph, "P");
    comparison->Add(bipo_graph, "P");

    string title =
        "Phase " + to_string(phase) +
        " radon activity comparison;Date;Activity (mBq/m^{3})";

    comparison->SetTitle(title.c_str());

    TCanvas* canvas =
        new TCanvas(
            "cActivityComparison",
            "Radon Activity Comparison",
            1000, 700);

    comparison->Draw("AP");

    comparison->GetXaxis()->SetTimeDisplay(1);
    comparison->GetXaxis()->SetTimeOffset(0, "gmt");
    comparison->GetXaxis()->SetTimeFormat("%d %b");

    TLegend* legend =
        new TLegend(0.20, 0.82, 0.80, 0.90);

    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->SetNColumns(2);

    legend->AddEntry(
        gamma_graph,
        "1e1#gamma (this analysis)",
        "pe");

    legend->AddEntry(
        bipo_graph,
        "1e1#alpha / BiPo",
        "pe");

    legend->Draw();

    string output =
        "phase" + to_string(phase) +
        "_activity_comparison.png";

    canvas->SaveAs(output.c_str());
}

void plot_activity_correlation(
    const vector<double>& gamma_activities,
    const vector<double>& gamma_errors,
    const vector<double>& bipo_activities,
    const vector<double>& bipo_errors,
    int phase)
{
    const int n = gamma_activities.size();

    if (n == 0 ||
        gamma_errors.size() != n ||
        bipo_activities.size() != n ||
        bipo_errors.size() != n) {

        cout << "Invalid activity correlation data." << endl;
        return;
    }

    TGraphErrors* graph =
        new TGraphErrors(
            n,
            bipo_activities.data(),
            gamma_activities.data(),
            bipo_errors.data(),
            gamma_errors.data());

    graph->SetMarkerStyle(20);
    graph->SetMarkerSize(0.75);
    graph->SetMarkerColor(kBlue + 1);
    graph->SetLineColor(kBlue + 1);

    string title =
        "Phase " + to_string(phase) +
        " activity correlation;"
        "1e1#alpha / BiPo activity (mBq/m^{3});"
        "1e1#gamma activity (mBq/m^{3})";

    graph->SetTitle(title.c_str());

    TCanvas* canvas =
        new TCanvas(
            "cActivityCorrelation",
            "Activity Correlation",
            900, 700);

    graph->Draw("AP");

    TF1* fit =
        new TF1(
            "activityFit",
            "[0]*x+[1]",
            graph->GetXaxis()->GetXmin(),
            graph->GetXaxis()->GetXmax());

    graph->Fit(fit, "Q");

    fit->SetLineColor(kRed + 1);
    fit->SetLineWidth(2);
    fit->Draw("same");

    cout << "\nPhase " << phase
         << " activity fit:" << endl;

    cout << "A_1e1gamma = a * A_1e1alpha + b" << endl;

    cout << "a = " << fit->GetParameter(0)
         << " +/- " << fit->GetParError(0) << endl;

    cout << "b = " << fit->GetParameter(1)
         << " +/- " << fit->GetParError(1)
         << " mBq/m^3" << endl;

    string output =
        "phase" + to_string(phase) +
        "_activity_correlation.png";

    canvas->SaveAs(output.c_str());
}

void generate_selection_plots(
    const string& filename,
    int phase)
{
    cout << "\nGenerating selection plots..." << endl;

    plot_photon_energy(filename, phase);
    plot_calo_tdc(filename);
    plot_vertex_geometry(filename);
    plot_timing_difference(filename);
    plot_total_energy(filename);

    cout << "Selection plots generated." << endl;
}
