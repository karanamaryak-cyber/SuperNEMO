# Measurement of Off-Plane Radon Activity Using the 1e1γ Channel

This repository contains a modularised version of the ROOT/C++ analysis developed for my MSc dissertation in Particle and Nuclear Physics at the University of Edinburgh.

The project investigated whether the **1e1γ decay channel of ²¹⁴Bi** could provide an independent method for measuring **²²²Rn activity within the SuperNEMO Demonstrator tracker**. Radon and its progeny are important backgrounds in searches for neutrinoless double-beta decay (0νββ), making their accurate measurement and mitigation essential.

The analysis was performed using **Phase 0 and Phase 3 SuperNEMO data** and was compared with the established **1e1α (BiPo) radon analysis**.

## Physics motivation

SuperNEMO searches for neutrinoless double-beta decay using a tracker-calorimeter detector capable of reconstructing particle trajectories, energies and timing information.

Radon contamination is an important background because its progeny, particularly **²¹⁴Bi**, can produce event topologies similar to those expected from 0νββ.

The established BiPo analysis identifies ²¹⁴Bi through the characteristic 1e1α signature, consisting of an electron followed by an alpha particle originating from the same location. This project investigated an alternative channel in which ²¹⁴Bi undergoes beta decay to an excited state of ²¹⁴Po, followed by gamma emission, producing a **one-electron–one-gamma (1e1γ)** topology.

## Analysis

The analysis was applied to **502 experimental data runs** together with simulated ²¹⁴Bi decays on the tracker wires.

Candidate 1e1γ events were identified through a sequential selection based on the reconstructed event topology, detector geometry, calorimeter timing and reconstructed energy.

The principal selection stages were:

1. **Photon-energy pre-cut**
   - Phase 0: `Eγ > 0.300 MeV`
   - Phase 3: `Eγ > 0.050 MeV`

2. **1e1γ topology**
   - Exactly one reconstructed electron
   - Exactly one reconstructed gamma

3. **Calorimeter timing**
   - `−20 ns ≤ calo_tdc ≤ 300 ns`

4. **Geometry selection**
   - Main-Wall optical modules
   - `77 mm < |x| < 337 mm`
   - `−2000 mm < y < 1900 mm`

5. **Time-of-flight selection**
   - Electron and gamma propagation times are reconstructed from their detector geometry and measured calorimeter times.
   - Candidate events satisfy:

     `Δt < 12 ns`

6. **Total-energy selection**
   - `E_e + E_γ < 3.000 MeV`

The code reports a sequential cut flow showing the number of events surviving each stage of the selection.

## Time-of-flight reconstruction

The electron velocity is calculated relativistically from its reconstructed kinetic energy. The electron and gamma flight times are then removed from their measured calorimeter times.

The timing variable used for the selection is

`Δt = |(t_e,calo − L_e/v_e) − (t_γ,calo − L_γ/c)|`

where `L_e` and `L_γ` are the reconstructed electron and gamma path lengths.

Events with `Δt < 12 ns` are retained.

## Radon activity

For each data run, the ²²²Rn activity is calculated from the number of selected 1e1γ events:

`A = N_selected / (t × V × ε)`

where:

- `N_selected` is the number of events surviving the complete selection,
- `t` is the run duration,
- `V = 15.4 m³` is the tracker volume,
- `ε` is the phase-dependent selection efficiency.

The efficiencies obtained for the analysis were:

- **Phase 0:** `ε = 3.191%`
- **Phase 3:** `ε = 3.366%`

Activity is reported in `mBq/m³`.

## Comparison with the BiPo analysis

The calculated 1e1γ activities were compared with independent measurements obtained using the established **1e1α/BiPo channel**.

Two complementary comparisons were performed:

- the evolution of the measured radon activity with time;
- the correlation between the 1e1γ and BiPo activity measurements.

The activity correlation was fitted using

`A_1e1γ = a A_1e1α + b`

to quantify the relationship between the two measurements.

## Main result

The **1e1γ analysis reproduced the overall variation in radon activity with time observed by the BiPo analysis**, demonstrating that the channel is sensitive to changes in the radon contamination within the tracker.

However, the 1e1γ method consistently measured substantially higher activities than the BiPo analysis. This indicates that additional background processes can produce the same electron-gamma topology and survive the 1e1γ event selection.

The study therefore demonstrated the sensitivity of the 1e1γ channel to radon variations while also identifying **background contamination as the principal limitation to using the channel as an independent precision measurement of radon activity**.

## Repository structure

```text
SuperNEMO/
│
├── src/
│   ├── EventSelection.C
│   ├── ActivityAnalysis.C
│   ├── RunUtilities.C
│   └── Plotting.C
│
├── plots/
│   ├── phase0/
│   └── phase3/
│
├── README.md
└── .gitignore
```

### `EventSelection.C`

Implements the sequential 1e1γ event-selection pipeline, including topology, calorimeter timing, geometry, time-of-flight and total-energy requirements.

### `ActivityAnalysis.C`

Calculates the selected-event efficiency and run-by-run radon activity.

### `RunUtilities.C`

Contains utilities for extracting run information, identifying the detector phase and retrieving run timing information.

### `Plotting.C`

Contains ROOT routines for visualising the principal selection variables and final analysis results, including:

- photon-energy distributions;
- calorimeter timing;
- reconstructed vertex distributions;
- time-of-flight corrected `Δt`;
- total electron-plus-gamma energy;
- 1e1γ/BiPo activity comparison;
- activity correlation and linear fit.

### `plots/`

Contains selected Phase 0 and Phase 3 figures illustrating the event selection and final activity comparisons.

## ROOT input

The analysis reads reconstructed events from a ROOT `TTree` named:

```text
Result_tree
```

Principal branches used include:

- `pid`
- `energy`
- `electron_number`
- `gamma_number`
- `calo_tdc`
- `om_number`
- `first_vertex_x/y/z`
- `vertex_track_first_end_x/y/z`
- `vertex_extrapolation_calo_x/y/z`
- `gamma_om_x/y/z`

The particle-ID convention is:

- `pid = 0` → gamma
- `pid = 1` → electron

## Software

The analysis was developed in **C++ using CERN ROOT** and uses ROOT functionality including:

- `TChain`
- `TH1D` / `TH2D`
- `TGraphErrors`
- `TMultiGraph`
- `TF1`
- `TCanvas`

## Scope of this repository

This repository is intended to provide a **representative and readable implementation of the core analysis**, rather than reproduce every diagnostic and specialised study contained in the complete dissertation analysis framework.

The original analysis was developed using SuperNEMO collaboration data and computing infrastructure. Consequently, the repository does not distribute experimental ROOT datasets or collaboration-internal data products.

## Dissertation

**Measurement of Off-Plane Radon Activity Using the 1e1γ Channel**  
Aryak Karanam  
MSc Particle and Nuclear Physics  
University of Edinburgh, 2026

Supervised by **Dr Cheryl Patrick** and **Dr Xalbat Aguerre**.
