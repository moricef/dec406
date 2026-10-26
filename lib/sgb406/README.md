# sgb406 — décodeur de balises 406 MHz de deuxième génération (SGB)

Bibliothèque C qui décode une salve SGB (C/S T.018, DSSS OQPSK, BCH(250,202))
à partir d'échantillons IQ. C'est la chaîne de démodulation de dec406, sans
modification, avec une fonction d'entrée unique. Elle ne dépend que de la
bibliothèque C standard et de `libm`. Licence MIT.

## Compilation

Android : ajouter dans le `CMakeLists.txt` de l'application

    add_subdirectory(<chemin>/sgb406)
    target_link_libraries(<votre bibliothèque JNI> sgb406)

PC (avec le programme de test `sgb406_test`) :

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build

## Utilisation

    #include "sgb406.h"

    sgb406_result r;
    int st = sgb406_decode(iq, n_samples, fs, f_offset_hz, &r);

`iq` contient les échantillons I et Q entrelacés en `float` (I0, Q0, I1, Q1…),
à n'importe quelle échelle, et `n_samples` le nombre d'échantillons complexes.
Le tampon doit contenir toute la salve, qui dure 1 s. Le plus simple est de
passer 2 à 2,5 s centrées sur la salve détectée.

`fs` est la fréquence d'échantillonnage. La chaîne a été validée à 2,4576 MS/s
et fonctionne aussi à 1,92 MS/s (50 échantillons par chip), sur les mêmes
salves réelles.

`f_offset_hz` est l'écart entre le centre de la salve et la fréquence
d'accord du récepteur. La recherche de fréquence couvre ±16 kHz autour de
cette valeur, une estimation à quelques kHz près suffit donc.

La valeur rendue est aussi dans `r.status` :

| Valeur | Sens |
|---|---|
| `SGB406_OK` | trame synchronisée, BCH juste ; `r.bch_errors` donne le nombre de bits corrigés |
| `SGB406_BCH_FAIL` | trame synchronisée mais BCH non corrigible ; `r.hex` contient les bits reçus |
| `SGB406_NO_SYNC` | aucun préambule SGB trouvé |
| `SGB406_BAD_PARAM` | tampon nul ou trop court (moins de 0,5 s) |

Les autres champs de `sgb406_result` :

| Champ | Contenu |
|---|---|
| `hex` | les 250 bits en 63 caractères hexadécimaux, convention T.018 (2 bits à zéro en tête, premier bit à 1 pour une trame d'auto-test) |
| `self_test` | 1 si la salve utilise le code d'étalement d'auto-test |
| `z_score` | score de synchronisation du préambule ; plus il est élevé, plus le signal est propre |
| `t_sync` | début de la fenêtre retenue, en secondes depuis le début du tampon |
| `text` | décodage complet en clair (identifiant 23 hex, TAC, pays, type, position, champ tournant…) |

La fonction n'est pas réentrante. Il faut l'appeler depuis un seul fil
d'exécution, distinct de celui de l'acquisition.

Sur un PC (Ryzen 7 5800H), un tampon de 2,5 s à 1,92 MS/s prend environ 7 s.
Sur un téléphone il faut s'attendre à plus.

## Fichiers de test

`sgb_firmin_20261026T*.sigmf-data` : quatre salves reçues le 26 octobre 2026
à environ 80 km de Toulouse, avec une RTL-SDR (R820T) sur antenne
omnidirectionnelle. SigMF `ci16_le`, 2,4576 MS/s, centré sur 406,050 MHz,
3 s par fichier, la salve commençant environ 1 s après le début du fichier.
Ce sont les octets de la clé convertis en entiers 16 bits, sans autre
traitement. La trame décodée et la fréquence de la salve sont dans
l'annotation du fichier `.sigmf-meta`.

| Fichier | Fréquence | 23 Hex ID | TAC | Mode |
|---|---|---|---|---|
| `…195027Z_90s`, `…195027Z_240s` | 406,047 MHz | `9C97FFFE1A8F00000000000` | 65535 | auto-test |
| `…211313Z_16s` | 406,045 MHz | `9C77FFFA18CF00000000000` | 65534 | auto-test |
| `…211313Z_127s` | 406,054 MHz | `9C77FFFE1A7F00000000000` | 65535 | normal |

Ce sont des balises système du CNES. Toutes se décodent sans erreur BCH, à
2,4576 MS/s comme après rééchantillonnage à 1,92 MS/s pour les deux premières.

Test sur PC :

    build/sgb406_test sgb_firmin_20261026T195027Z_90s.sigmf-data -i -f -3000 -v

`-i` pour les entiers 16 bits (sans option : `float` complexes), `-s` pour la
fréquence d'échantillonnage (2457600 par défaut), `-f` pour l'écart de
fréquence, `-v` pour le texte décodé.
