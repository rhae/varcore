
# Motivation
Provide a software component for an ressource constrained system
(embedded) that manages variables.
Variables are defined at compile time.
Variables are defined in an CSV table.
A preprocessor creates the required structures.

# Variable core

- Data types: int16, int32, float, string und enum either as scalar values and vectors
- standard values
- access rights
  * user, admin
  * rd/rw source restriction
- storage types: Volatile, EEPROM
- number formats (optional)
- units (optional)

# Variable preprocessor
The preprocessor reads a CSV file.
The CSV file can contain #defines and #pragma instructions.
#defines can be used in the CSV table for easy definitions of constants.
#pragma's are used to control the parsing and generation process.

## CSV format - input for variable preprocessor

`Handle;cmd;access;storage;vector;datatype;<datatype specific>`

int16, int32:
`default value, Min, Max`

Float:
`default value, min, max, decimal places`

string:
`modifier, default value`

enum:
`references to variables`

<variable>:<value>:<ASCII>
example: VAR_ON=1=SYM_ON

## Pragmas
* section {var|string}
Set the current section.
At the moment there is only one section, ie. var.
The section string needs to be implemented.

Start value is: var.

* prefix
Defines the Hnd-Prefix.
Only lines where column 1 starts with this prefix are used.
Start value is: VAR_

# Tasks

- [x] variable preprocessor
- [x] variable preprocessor, pragmas (section, prefix)
- [ ] variable preprocessor, Windows exe
- [x] int16, int32, float variables
- [x] string variables
- [x] string (constant) variables
- [x] pool for constant strings
- [ ] enum variables
- [ ] binary
- [ ] strings for later use in a programm
- [ ] cmake
- [ ] test suite (in progress)
- [ ] sample programm for embedded (msp430)

