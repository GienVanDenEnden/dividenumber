dividenumber
============

Integer factorization with brute force methode

Description
-----------
Commandline tool for integer factorization.

The result is saved in the file out_result.txt .

By pressing 'S', the current state is saved and the program is ended.
By using the option '--continue' the program resumed the saved state.


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
  By the first numbers found, stop searching.
   
.\dividenumber --findall 12345678901234567893
   Find all the numbers.
```

License
-------
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
