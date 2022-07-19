# Prova Finale di Algoritmi e Strutture Dati - a.a. 2021-2022   (30L)
__WordChecker__: Lo scopo del progetto è implementare un gioco simile a Wordle che sia cosciente delle limitazioni imposte ad ogni tentativo:
  * Il gioco inizia ricevendo in ingresso un dizionario di parole a lunghezza fissa, seguito da una sequenza di partite.
  * Ogni partita contiene i seguenti comandi:
      - _+nuova_partita_    : Delimita l'inizio di una partita, seguito dalla stringa da indovinare e il numero di tentativi
      - _+inserisci_inizio_ : Seguito da lista di parole da aggiungere, chiusa da +inserisci_fine. Può trovarsi all'interno di una partita o trs una partita e la successiva.
      - _+stampa_filtrate_  : Stampa in ordine lessicografico tutte le parole del dizionario compatibili con le limitazioni apprese dopo ogni tentativo.
  * Come in wordle, ad ogni tentativo di indovinare si apprende, per ogni lettera, qualora questa lettera fosse contenuta nella parola da indovinare. Nella valutazione
    viene anche inclusa informazione riguardante il numero minimo ed esatto di occorrenze di ciascuna lettera mediante l'uso di '|'.
      - ref   = abcabcabcabc
      - guess = bbaabccbccbc
      - eval  = /+|+++|++/++
      
__Implementazione__:
  * L'input viene ricevuto da file, il folder tests è preso dalla repo di [federico123579](https://github.com/federico123579/APi2022-Project-tools) che ringrazio. Per
    testare il programma e valutare l'output usare i seguenti comandi nella cartella della repo:
    
  ```
  make all
  ./release/build < tests/(input_dir)/(input_file).txt > tests dump.txt
  diff tests/dump.txt tests/(input_dir)/(input_file).output.txt
  ```
  * Nella repo sono presenti folder per implementazioni mediante hash table e rbtree come dizionari che si sono rivelate troppo lente/ingombranti. Queste implementazioni
    dovrebbero essere funzionanti ma non ne garantisco la correttezza, in quanto abbandonate una volta capito fosse impossibile passare con esse.

  * __Trie Dinamico parzialmente compresso__  :    L'uso di un trie come dizionario rende estremamente efficace il filtraggio dopo ogni tentativo, mantenendo ricerca ed inserimento in tempo costante. La scelta di usare
   una struttura simil-lista dinamica nasce dalle limitazioni di spazio, e per lo stesso motivo il trie viene anche compresso parzialmente, solo alle foglie (ovvero non
   esistono nodi interni, o "rami", che rappresentino più di una lettera, ma le foglie possono contenere un intero suffisso della parola). Risultati:

      - UPTO18-S1 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0.634s &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 17.6 MiB
      - UPTO30-S1 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 16.670s &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 130.0 MiB\
        UPTO30-S2 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 15.140s &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 130.0 MiB\
        UPTO30-S3 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 19.473s &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 130.0 MiB\
        UPTO30-S4 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 21.132s &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 95.8 MiB
      - CUMLAUDE &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 48.555s &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 170.0 MiB
