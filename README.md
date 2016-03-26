dividenumber
============

Integer factorization with brute force methode

Description
-----------

Commandline tool for integer facorization with brute force methode.

Installation
------------
```
OS             Linux
Compile        make
```

Options
-------
* `--help`                Show help and exit
* `--version`             Show version and exit
* `--continue`            Continue with the last saved state
* `--nothreaddisplay`     No thread state display
* `--findall`             Find all the numbers, otherwise the first valid numbers
* `--nothreads`           Do not use threads
* `--maxthreads <number>` Set the maximum number of threads

Examples
--------
```
.\dividenumber 12312343
  By the first dividers found, stop searching
   
.\dividenumber --findall 12312343
   Find all the dividers
```

License
-------
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
