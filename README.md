# MFI Autmation Project

This repo contains the source code and documentation powering the real time **data aquisition** and **contol** project for MFI International's textile services.

# Setup

## Downloading Visual Studio Code

### This project is best utilized using Visual Studio Code

1. Download [Visual Studio](https://code.visualstudio.com/Download)
1. Install the [Arduino Extension](https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino)
1. Install the [C/C++ Extension Pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)

## Installing and Using Version Control (Git)

### Installing Git:

1. For Windows download it [here](https://git-scm.com/download/win)
1. For Linux systems, download it using the command `apt-get install git`

### Initalizing this repo to your computer:

1. `git init` initializes git in the folder the command is run in.
1. `git config --global user.name "[First_Name Last_Name]"` sets the author's name for your computer.
1. `git config --global user.email "[email_address]` sets the author's email address for your computer. **Make sure this matches your github account**
1. `git clone https://github.com/MFI-International/mfi_automation.git` clones the repo onto your folder

### Pulling the latest remote repository to local device

1. `git pull`

### Pushing your changes to the remote repo

1. `git add .` adds all your changes to the split branch
1. `git commit -m '[message]'` commits the changed branch to you local main branch
1. `git push -u origin master` pushes your main branch to the remote repo

## Installing the Required Libraries

The below libraries are necessary for running this project as is, to install these using VSCode, type <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> and type and click `Arduino: Library Manager` in the dialouge box. There you can search up the below libraries and install them.

### ADS1115 Analog to Digital Converter

1. `Adafruit ADS1X15` by Adafruit
1. `Adafruit BusIO` by Adafruit

### RA8875 TFT Screen Driver Board

1. `Adafruit RA8875` by Adafruit
1. `Adafruit GFX Library` by Adafruit

## Setting up the Adruino Itself

In order to run the code 'as is' please follow the schematic below.

![This is an image](https://i.imgur.com/hlbVIjG.png)

# Implementation

## Scheduling

The program schedules between HTTP client request bursts and sensor reading bursts. The client request bursts have priority over sensor readings since they are not cyclical. Please note that this scheduling algorithm only works if the HTTP requests are no less than 240ms apart (the time it takes to pull the new sensor readings and serve it to the client). The next goal is to build context changes so there can be a limit on the time quantum for a process and drop the response time for the processes that need it (GUI etc...)

```
if( httpRequestExists == true ){
    serveDataToClient();
    // this takes about 80ms to complete
}else{
    readADCValues();
    // this tkaes aout 160ms to complete
}
```
