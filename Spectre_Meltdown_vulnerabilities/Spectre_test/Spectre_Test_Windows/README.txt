Spectre vulnerability experimental test for Windows.
Based on the Spectre paper : https://spectreattack.com/spectre.pdf
Jean-François DEL NERO (06 January 2018)
Press Return to quit, F1 to toggle debug outputs.

Changes applied from the original source code :
- CPU without the rdtscp instruction support added (rdtsc used instead on these machines... to be improved).
- Looping mode.
- Debug/detailed mode can be enabled/disabled.
