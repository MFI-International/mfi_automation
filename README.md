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

# API

The program has the below set of insructions to communicate between client and server. Pease not e that **these are subject to change and additions as needed**

## General Usagge

1. The general syntax of the protocol is `[CMD]:[VAL],...[VAL]`, where `[CMD]` is the command and `[VAL]` are the values attributed to that command. There might be more than one value required for a given command
1. Commands that are initiated from the client are titled `(A->S)`
1. Commands that are initiated from the server are titled `(S->A)`

## Commands

---

### **Login `(A->S)`**

#### Request:

```
LGN:[userID],[machineID]
```

- **userID** `(uint16_t)`: The unique ID of each user.
- **machineID** `(uint16_t)`: The unique ID of each machine.

#### Response: ( Successfull / Unsuccessfull )

```
LGN:[userName],[target],[completed]
LGN:ERR,[errorCode]
```

- **userName** `(string)`: The name of the user associated with **userID**.
- **target** `(uint16_t)`: The starting target value for the user associated with **userID**.
- **completed** `(uint16_t)`: The starting completed value for the user associated with **userID**.
- **errorCode** `(uint8_t)`: Code given on error related to the given **userID**.
  - `0` : **userID** not found.
  - `1` : **userID** not certified for machine
  - `2` : Internal Error.

---

### **Logout `(S->A)`**

#### Request:

```
LGO:[]
```

#### Response: ( Successfull / Unsuccessfull )

```
LGN:[]
LGN:ERR,[errorCode]
```

- **errorCode** `(uint8_t)`: Code given on error related to logging out of the related machine.

  - `0` : no one is logged into the machine.
  - `1` : Internal Error.

---

### **Set Target `(S->A)`**

#### Request:

```
TAR:[target]
```

- **target** `(uint16_t)`: The target value to be set to the machine.

#### Response: ( Successfull / Unsuccessfull )

```
TAR:[target]
Tar:ERR,[errorCode]
```

- **target** `(uint16_t)`: The target value set to the machine.
- **errorCode** `(uint8_t)`: Code given on error related to the given **target** value.
  - `0` : Internal Error.

---

### **Set Completed `(S->A)`**

#### Request:

```
CMP:[completed]
```

- **completed** `(uint16_t)`: The completed value to be set to the machine.

#### Response: ( Successfull / Unsuccessfull )

```
CMP:[completed]
CMP:ERR,[errorCode]
```

- **completed** `(uint16_t)`: The completed value set to the machine.
- **errorCode** `(uint8_t)`: Code given on error related to the given **completed** value.
  - `0` : Internal Error.

---

### **State Change `(A->S)`**

#### Request:

```
SCH:[level]
```

- **level** `(uint16_t)`: The level the analog to digital converter is now reading. _note that the levels ranges from 0 to the max level given in the `STL` command_

#### Response: ( Successfull / Unsuccessfull )

```
SCH:[level]
SCH:ERR,[errorCode]
```

- **level** `uint8_t`: The new **level** recieved by the server.
- **errorCode** `(uint8_t)`: Code given on error related to the given **level**.
  - `0` : **level** value given not found between 0 and **levels** given in `STL` command.
  - `1` : Internal Error.

---

### **Set Number of Transition Levels `(S->A)`**

#### Request:

```
STL:[levels]
```

- **levels** `(uint8_t)`: The max number of equally spaced levels the analog to digital converter will now use to calcualte when to trigger the `SCH` command. Ranges from 0 to 100. The formula for calculation of states is `state = ceil([currentVoltage] * [levels]) / [maxVoltage] `

#### Response: ( Successfull / Unsuccessfull )

```
STL:[levels]
RST:ERR,[errorCode]
```

- **levels** `(uint8_t)`: The number of state transition levels the machine is now broadcasting with.
- **errorCode** `(uint8_t)`: Code given on error related to the **levels** value given.
  - `0` : Levels value not in range.
  - `1` : Internal Error.

---

### **Reset Machine `(S->A)`**

#### Request:

```
RST:[]
```

#### Response: ( Successfull / Unsuccessfull )

```
RST:[]
RST:ERR,[errorCode]
```

- **errorCode** `(uint8_t)`: Code given on error related to resetting the related machine.
  - `0` : Internal Error.

---
