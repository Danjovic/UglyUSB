#!/usr/bin/env python

# ===  UGLY COMPILER ===
#
#  Duckyscript to UglyPlayer converter by Danjovic
#  http://hackaday.io/danjovic
#
#  For my father, in memoriam - April 30th, 2017
#
# Based on python-duckencode by crushedice2000
# https://github.com/crushedice2000/python-duckencode
#
# Released under GPL V2.0
# Developed using Python 2.7.3

"""
Usage: {} <input> <output>

<input> should be a DuckieScript file and <output> will be a bin file
"""

# ------------------------------------------------------------------------------
# libraries
# ------------------------------------------------------------------------------
import os
import sys
import fileinput
import textwrap
from data import keycodes
from data import charcodes

# ------------------------------------------------------------------------------
# variables
eeprom = []      # eeprom contents

# ------------------------------------------------------------------------------
# functions
# ------------------------------------------------------------------------------
def info(type, msg, **kwargs):
    lineno = fileinput.lineno()
    exit = kwargs.get('exit', None)
    path = os.path.basename(input_file)
    types = ['error', 'warning', 'info', 'screw up risk']
    message = '{}:{}: {}: {}'.format(path, lineno, types[type], msg)
    print(message)
    if exit:
        print('{}: compilation aborted.'.format(path))
        sys.exit(exit)


def getkey(keys):
    global commands,eeprom
    keys = keys.split()
    modifier_keys = 0
    normal_keys = []
    arguments = []

    for key in keys:
        if not key.isupper() and len(key) is not 1:
            info(1, 'you should type all the special keys in uppercase')
        key = key.upper()

        try:
            key = [keycodes.get(code) for code in keycodes if key in code][0]
        except IndexError:
            info(0, 'unrecognized key: {}'.format(key), exit=5)

        key_code = format(key[0], '#04x') # format keycode

        if key[1]:
            modifier_keys |= key[0]
            eeprom.append('0xAF') # modifier, then hold next key
            eeprom.append(key_code) # add key

        else:
            normal_keys.append(key[0])
            eeprom.append(key_code)

    if len(normal_keys) > 6:
        info(0, 'maximum number of non-modifier keys per line is 6', exit=4)

    eeprom.append('0xAE') # safeguard
    eeprom.append('0x00') # end of command
    return True


# ------------------------------------------------------------------------------
# main program
# ------------------------------------------------------------------------------
if len(sys.argv) is not 3:
    print(__doc__.format(sys.argv[0]))
    sys.exit(1)
elif not os.path.isfile(sys.argv[1]):
    print()
    print('Error: You must specify a valid input file')
    print(__doc__.format(sys.argv[0]))
    sys.exit(2)
else:
    eeprom_file = sys.argv[2]
    input_file = sys.argv[1]



for line in fileinput.input([input_file]):
#    base = line.split(maxsplit=1)
    base = line.split(" ",1)
    base = map(str.strip, base)
    base = list(base)

    if len(base) is 2:
        command, options = base
    elif len(base) is 1:
        command = base[0]
        options = None
    else:
        continue


    if not command.isupper() and \
           command not in ('#', '//', ';', '@', '%') and \
           len(command) > 0:                               # skip blank lines
        info(1, 'commands should be typed in uppercase  ')
        command = command.upper()

    if len(command) == 0:
        continue

    # Remarks don't go to eeprom
    if command in ('REM', 'COMMENT', '#', '//', ';'):
        cmdtype = -1
        continue

    # Set default delay
    elif command in ('DEFAULT_DELAY', 'DEFAULTDELAY'):
        if not options.isdigit():
            info(0, '{} only accepts integers'.format(command), exit=3)
        eeprom.append('0xDF')
        delay_msb = (delay>>8) & 0xff
        delay_lsb = (delay & 0xff)
        eeprom.append(format(delay_lsb,'#04X'))
        eeprom.append(format(delay_msb,'#04X'))
        eeprom.append('0x00') # end of command

    elif command in ('SLEEP', 'DELAY', 'WAIT'):
        if not options.isdigit():
            info(0, '{} only accepts integers'.format(command), exit=3)
        delay2 = int(options)
        eeprom.append('0xDE')
        delay_msb = (delay2>>8) & 0xff
        delay_lsb = (delay2 & 0xff)
        eeprom.append(format(delay_lsb,'#04X'))
        eeprom.append(format(delay_msb,'#04X'))
        eeprom.append('0x00') # end of command

    elif command in ('REPEAT', 'REPLAY'):
        if not options.isdigit():
            info(0, '{} only accepts integers'.format(command), exit=3)
        #last_command = commands.splitlines()[-1]
        replay_count = int(options)
        if len(eeprom)>0:           # as far repeat is not 1st instruction
            eeprom.pop()            # remove zero before repeat
        eeprom.append('0xA7')
        replay_count_msb = (replay_count>>8) & 0xff
        replay_count_lsb = (replay_count & 0xff)
        eeprom.append(format(replay_count_lsb,'#04X'))
        eeprom.append(format(replay_count_msb,'#04X'))
        eeprom.append('0x00') # end of command


    elif command in ('STRING', 'TEXT', 'PRINT'):
        for c in options:
            # check if character is shifted or not shifted
            if charcodes[c][1]:
                eeprom.append('0xAF')   # hold token
                eeprom.append('0xE1')   # shift code
            character=format(charcodes[c][0],'#04X') # add the character code
            eeprom.append(character) # add the character code
        eeprom.append('0x00') # end of command

    else:
        cmdtype = 3
        if not getkey(line):
            info(0, 'unrecognized command: {}'.format(command), exit=2)


# Add end of script
eeprom.append('0xFF')


# generate the output files
binary_code=''
for b in eeprom:
    binary_code+=chr(eval(b))


try:
    with open(eeprom_file, 'wb') as output: # wb to write binary file avoid
        output.write(binary_code)           # substitution of LF for CR+LF
except Exception as e:
    info(0, '{}'.format(e), exit=6)



