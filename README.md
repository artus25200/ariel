# Ariel
Un language de programmation interpreté à syntaxe flexible que j'ai créé pour améliorer mes competences en programmation, notamment dans le language C.

## Syntaxe
### Code
```c
{
  print "Ceci est un programme d'exemple", newline;
  print "Ce language a une syntaxe flexible :", newline;
  print("- Vous pouvez mettre ou ne pas mettre de parentheses", newline);
  { print "- Vous pouvez créer des blocs de code comme ceci", newline; };
  print "- Vous pouvez faire des calculs de math : ", 3 * 4 + 19 / 7 * (1 + 3),
      newline;
  print "- Vous pouvez" + " additionner et " + "multiplier " * 3 +
      "des chaines de caracteres" + newline;
};
```
### Résultat
```console
$ ./cariel syntaxe.ariel
Ceci est un programme d'exemple
Ce language a une syntaxe flexible :
- Vous pouvez mettre ou ne pas mettre de parentheses
- Vous pouvez créer des blocs de code comme ceci
- Vous pouvez faire des calculs de math : 22.8571
- Vous pouvez additionner et multiplier multiplier multiplier des chaines de caracteres
```
