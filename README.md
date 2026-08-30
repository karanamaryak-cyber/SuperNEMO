# Measurement of Off-Plane Radon Activity Using the 1e1γ Channel


This repository contains a simplified ROOT implementation of the event-selection logic used to study off-plane radon backgrounds in the SuperNEMO Demonstrator through the `1e1γ` decay channel of `²¹⁴Bi`.

The code is a reduced version of the full analysis macro. It retains the ROOT tree reading, event-selection cuts, timing calculation, simulation efficiency calculation, radon activity calculation, and a simple sequential cut-flow summary. Plotting, diagnostic studies, and activity-comparison routines are not included.

## Analysis sequence

Events are processed in the following order:

1. **Photon-energy pre-cut**

   * Phase 0: `Eγ > 0.300 MeV`
   * Phase 3: `Eγ > 0.050 MeV`

2. **Topology selection**

   * Exactly one reconstructed electron
   * Exactly one reconstructed gamma
   * Exactly two entries in the particle-ID vector

3. **Calorimeter TDC cut**

   * Electron and gamma must both satisfy `−20 ns ≤ calo_tdc ≤ 300 ns`

4. **Geometry selection**

   * Main-Wall optical modules only: `0 ≤ om_number < 520`
   * Electron vertex: `77 mm < |x| < 337 mm`
   * Electron vertex: `−2000 mm < y < 1900 mm`

5. **Timing selection**

   * The electron velocity is calculated relativistically from its reconstructed kinetic energy.
   * Electron and gamma times of flight are subtracted from their measured calorimeter times.
   * The event must satisfy `Δt < 12 ns`, where:

     `Δt = |(t_e,calo − L_e/v_e) − (t_γ,calo − L_γ/c)|`

6. **Total-energy selection**

   * The reconstructed electron-plus-gamma energy must satisfy `E_total < 3.000 MeV`.

The cut flow printed for each file gives the number of events remaining after every sequential selection.

## ROOT tree input

The macro reads a TTree named `Result_tree`. The required branches include:

* `pid`, `energy`, `electron_number`, and `gamma_number`
* `calo_tdc` and `om_number`
* Data vertex branches: `first_vertex_x/y/z`
* Simulation vertex branches: `vertex_track_first_end_x/y/z`
* `vertex_extrapolation_calo_x/y/z`
* `gamma_om_x/y/z`

The particle-ID convention used by the analysis is:

* Gamma: `pid = 0`
* Electron: `pid = 1`

## Radon activity

For each data run, the radon activity is calculated as:

`A = N_selected / (t × V × ε)`

where:

* `N_selected` is the number of events surviving all cuts.
* `t` is the run duration in seconds.
* `V = 15.4 m³` is the tracker volume.
* `ε` is the phase-dependent efficiency reported in the submitted thesis:

  * Phase 0: `ε = 0.03191` (`3.191%`)
  * Phase 3: `ε = 0.03366` (`3.366%`)

The result is converted from `Bq/m³` to `mBq/m³`.

## Running the data analysis

Start ROOT:

```bash
root -l
```

Load and compile the macro:

```cpp
.L cutzz.C+
```

For Phase 0:

```cpp
CUTzz("input_files.txt", 0)
```

For Phase 3:

```cpp
CUTzz("input_files.txt", 3)
```

`input_files.txt` must contain one reconstructed ROOT-file path per line. For each valid run, the macro prints the run duration, selected-event count, efficiency, and activity directly in the ROOT terminal.

## Calculating simulation efficiencies

After loading the macro, run:

```cpp
calculate_simulation_efficiencies("simulation_input_files.txt")
```

The input list must contain exactly 20 simulation ROOT files. The calculation assumes `1,000,000` generated events per file and applies both the Phase 0 and Phase 3 selections to the same simulation sample.

The efficiencies printed by this diagnostic function are not passed automatically to the activity calculation. The activity calculation uses the fixed efficiencies reported in the submitted thesis, as listed above.

## Scope

This macro presents the core selection and activity-calculation logic. It does not contain the plotting, background-comparison, diagnostic CSV, or specialised investigation routines from the complete analysis code.


The efficiencies printed by this diagnostic function are not passed automatically to the activity calculation. The activity calculation uses the fixed efficiencies reported in the submitted thesis, listed above.

Scope

This macro presents the core selection and activity-calculation logic. It does not contain the plotting, background-comparison, diagnostic CSV, or specialised investigation routines from the complete analysis code.
