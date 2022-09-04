# Algorithms and Data Structures - Final Project 2021-2022   (30L)
__WordChecker__: The aim of the project is to implement a game similar to wordle that is also conscious of the limitations imposed by each guess

  * The game starts by receiving a dictionary of fixed length words, followed by a sequence of games.
  * Each game contains the following commands:
      - _+nuova_partita_    : Precedes the start of a game, followed by the word to guess and the amout of guesses allowed.
      - _+inserisci_inizio_ : Followed by a list of words to be added to the dictionary, ended by _+inserisci_fine_. Can also be found between games.
      - _+stampa_filtrate_  : Prints in lexicographical order all words from the dictionary compatible with the limitations learned from previous guesses.
  * With each guess, more information is learned about the reference word, in the form of exact and minimum occurrences of a certain character, and positions in which a certain character must or must not appear. After each guess, the amount of words still compatible with all the bounds must be printed.

__Implementation__:
  * Input is redirected from a file to stdin. Test files with correct output are available on [federico123579's repo](https://github.com/federico123579/APi2022-Project-tools) (they were a great help to me) and much larger tests are on my [OneDrive](https://polimi365-my.sharepoint.com/:u:/g/personal/10726194_polimi_it/EesHKg7pkuxIotu0G6qn9h4B2j39QcH9fF3gUnHmLQaxdg?e=KQAFr2). To test the program simply input this from the src directory or run gcc on the monolithic submission.c (use debug build for profiling)
  ```
  make all
  ./release/build < (test_path).(test_name).txt > dump.txt
  diff dump.txt (test_path).(test_name).output.txt
  ```
  * In prior commits I have (broken) implementations using RBTrees and Hash Tables, which both seemed slow at first glance but have been proven capable of passing the project.
  * __Compressed Ternary Search Tree__ : The use of a trie-like structure allows for very efficient filtering of the dictionary, since branches can be pruned without having to descend to the leaves, and requires no compromise on insertion and search times. A simple trie would not pass due to size limits, and for the same reasons the tree must be compressed at the leaves (this means that while branches always represent a single letter, leaves can represent a suffix)
  * Note that, due to word sizes in the exams tests, the program assumes words to be at most 256-character long and have at most 127 occurrences of the same character. This clearly isn't always the case, but the use of longer integers would not in any way interfere with performance.
  
      - UPTO18-S1 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0.634s &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 17.6 MiB
      - UPTO30-S1 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 16.670s &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 130.0 MiB\
        UPTO30-S2 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 15.140s &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 130.0 MiB\
        UPTO30-S3 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 19.473s &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 130.0 MiB\
        UPTO30-S4 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 21.132s &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 95.8 MiB
      - CUMLAUDE &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 48.555s &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 170.0 MiB
