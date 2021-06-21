# DAQ_box
<!--- 
One paragraph statement about the project.
--->
Repo for McMaster Baja Racing's data aquisition box.
```
 ───DAQ_box
    │   LICENSE
    │   README.md
    │
    ├───daq_system
    │   ├───daq_system_DEBUG
    │   │       daq_system_DEBUG.ino
    │   │
    │   └───daq_system_RELEASE
    │           daq_system_RELEASE.ino
    │
    ├───HUD
    │   ├───HUD_10LED_RPM
    │   │       HUD_10LED_RPM.ino
    │   │
    │   └───neopixel_colour_tuning
    │           neopixel_colour_tuning.ino
    │
    └───IMU
        └───restore_offsets
                restore_offsets.ino
```
## Built With

- Arduino
- Python

## Live Demo
<!--- 
Add a link to a video of a demo
--->

Demo coming soon

## Getting Started
<!---
Provide steps on how to get this project up and running locally  for a new member
--->

### Prerequisites
The following will be required
- git
- Arduino or or `platformIO` for VSCode (something that can compile and flash arduino code)
- python3

### Setup

1. Install `git` using instructions found [here](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git).
2. Install `Arduino` or `platformIO` for VSCode (something that can compile and flash arduino code).
3. Clone this repo by running `git clone https://github.com/McMaster-Baja-Racing/DAQ_box.git`

### Usage

### Run tests
No unit or integration tests yet.

### Deployment

The usual arduino deployment. Instructions found [here](https://chipwired.com/uploading-code-arduino/).
