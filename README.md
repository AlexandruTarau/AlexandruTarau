This repository contains the source code of my version of the game "Wordle" written in C using ncurses for the graphic interface.

The game consists in having to guess a 5-letter word chosen randomly from a list.
For each guess, the program will highlight the letters with:
  - Green, if the letter in is the correct place;
  - Yellow, if the letter exists in the word but not in that place and is not already found(green);
  - Black, if it is not in the word.

It is possible to delete letters using "Backspace".
The player can also restart the game by pressing the key "R" and access the menu by entering ":".
