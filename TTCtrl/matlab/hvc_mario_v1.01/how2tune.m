In S:\Progetti\Engineering\Keck_NGWFC\Matlab\KeckTools\maintenance\servotuning
1. init % inizializza lo specchio e chiude il loop nella posizione attuale
2. methodInit % inizializza il metodo di ottimizzazione. Vengono scritte nel workspace OPTIONS, p, methodParameter. Utilizza pid_K, pid_I, pid_D, pid_N
3. optimize('method')
3a. manualTune % questo script esegue il tuning manuale. provare prima questo

L'ottimizzatore utilizza il parametro method che e' una funzione con parametri p, parameter e che resituisce il merit factor, ad esempio:
S:\Progetti\Engineering\Keck_NGWFC\Matlab\KeckTools\maintenance\servotuning\DTTOptimizePID.m
p sono i parametri che vengono ottimizzati
parameter sono i parametri fissi

