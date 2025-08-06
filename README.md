# DCC Model Validation with GENIE

This project validates the implementation of the DCC (Dynamical Coupled-Channels) model in GENIE by comparing GENIE-generated single-pion electroproduction cross sections to experimental data from Egiyan et al. (2006).

## Description

This repository contains a set of ROOT-based C++ programs for computing and visualizing the exclusive electroproduction cross sections of the channel:

> **e + p → e′ + π⁺ + n**

The GENIE framework is used to generate and analyze events using the DCC model, and the outputs are compared to experimental results from Egiyan et al.

## Structure

- `single_pion_main.cpp`: Main program to read Egiyan data, compute cross sections from GENIE events, and compare.
- `single_pion.hpp`: Definitions for managing experimental cross section data.
- `SPP_event.hpp`: Functions for filtering single-pion production events from GENIE GHEP files.
- `Xsec_sim.hpp`: Responsible for computing the cross section from GENIE spline and events.
- `Utils.hpp`: Utility functions for I/O, ROOT operations, and cross section binning.
- `res/`: Directory where I chose to put the necessary files for the comparison (e.g. `Egyan-data`,`genie-event-files`,...).

## Features

- Reads digitized Egiyan 2006 data points from a CSV file.
- Filters GENIE events for exactly one π⁺ and one neutron in the final state.
- Computes differential cross sections using GENIE splines and kinematics.
- Normalizes GENIE predictions to match Egiyan-style binning.
- Produces comparison plots in PDF format.
## Usage
> **$ ./Single_pion_production_cmp --input_file path_to_egiyan_data.csv --event_file path_to_genie_events.ghep.root --xsec_file path_to_genie_xsec_spline.root**

The output will be  PDF files containing graphs for visual inspection and validation.
