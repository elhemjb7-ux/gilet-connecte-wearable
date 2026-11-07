# Calcul des dimensions initiales du patch à 2,45 GHz

## Données
| Grandeur | Valeur | Origine |
|---|---|---|
| Fréquence de résonance f₀ | 2,45 GHz | Énoncé p. 3, Étape 2.1 (bande ISM) |
| Substrat : feutrine | εr = 1,22 ; tan δ = 0,016 ; h = 2 mm | Cours p. 4 (matériaux adoptés) |
| Conducteur : Zelt | σ = 1,749 × 10⁵ S/m ; 0,0635 mm | Cours p. 4 |
| Vitesse de la lumière | c = 3,00 × 10⁸ m/s | Constante |

**Note importante** : les formules de dimensionnement d'un patch (largeur, extension ΔL) ne figurent pas dans les supports fournis (cours p. 1 à 9 et séance 1). Le calcul ci-dessous utilise les formules **de Hammerstad-Jensen**, communes en conception de patch microruban. Elles sont **une hypothèse ajoutée**, à confirmer avec l'enseignant avant la remise.

## Formules (hypothèse, Hammerstad-Jensen)
1. Largeur : W = c / (2 f₀) · √(2 / (εr + 1))
2. Permittivité effective : ε_eff = (εr+1)/2 + (εr−1)/2 · (1 + 12 h / W)^(−1/2)
3. Extension de longueur : ΔL = 0,412 h · [(ε_eff + 0,3)(W/h + 0,264)] / [(ε_eff − 0,258)(W/h + 0,8)]
4. Longueur : L = c / (2 f₀ √ε_eff) − 2 ΔL

## Application numérique (feutrine sèche, εr = 1,22)
- W = 3,00e8 / (2 × 2,45e9) × √(2 / 2,22) = 0,0612 × 0,9493 = **58,1 mm**
- W/h = 29,05
- ε_eff = 1,11 + ... → **ε_eff = 1,2025** (calcul : 1,11 + 0,11 × (1 + 12·2/58,1)^(−1/2))
- ΔL = 0,412 × 2 × [(1,5025)(29,31)] / [(0,9445)(29,85)] = **1,287 mm**
- L = 3,00e8 / (2 × 2,45e9 × √1,2025) − 2 × 1,287 = 0,0612 / 1,0966 − 2,574 = 0,05581 − 0,002574 = **53,3 mm**

Dimensions initiales retenues : **W = 58,1 mm, L = 53,3 mm** (valeurs calculées, pas mesurées).

## Sensibilité à la permittivité (proxy humidité)
Hypothèse : l'humidité augmente εr du substrat ; la fréquence de résonance est fixée à 2,45 GHz, donc le modèle recalcule les dimensions à εr constante. La sensibilité **décalée en fréquence** est obtenue en gardant W, L fixes et en recalculant f₀ à partir de ε_eff.

| εr (substrat) | W (mm) | ε_eff | ΔL (mm) | L (mm) (pour 2,45 GHz) | Commentaire |
|---|---|---|---|---|---|
| 1,22 (sec) | 58,11 | 1,2025 | 1,287 | 53,26 | Référence |
| 1,30 | 57,09 | 1,2759 | 1,252 | 51,70 | Humidité modérée (hypothèse) |
| 1,40 | 55,89 | 1,3673 | 1,215 | 49,93 | Humidité élevée (hypothèse) |
| 1,60 | 53,70 | 1,5494 | 1,157 | 46,87 | Scénario extrême (hypothèse) |
| 1,80 | 51,74 | 1,7306 | 1,113 | 44,31 | Scénario extrême (hypothèse) |

Lecture : à géométrie fixe (W = 58,1 mm, L = 53,3 mm), la fréquence varie approximativement en 1/√ε_eff. Passer de ε_eff = 1,2025 (sec) à 1,3673 (εr = 1,40) donne f ≈ 2,45 × √(1,2025/1,3673) ≈ **2,30 GHz**, soit environ −6 % (≈ −150 MHz). Estimation d'ordre de grandeur, à confirmer par simulation ou mesure.

## Ce que le modèle analytique permet de conclure
- Il donne un **ordre de grandeur** des dimensions et de leur sensibilité à εr.
- Il **ne prédit pas** l'effet de la courbure du torse ni le rendement réel : le modèle suppose un plan infini, un substrat homogène et un plan de masse idéal.
- La transpiration modifie εr **et** les pertes (tan δ). Le modèle analytique ne traite que εr ; les pertes doivent être mesurées (S11 à plat puis sur support courbe, à sec puis humide).
- Les conclusions utilisables sont donc : une hausse de εr de 1,22 à 1,40 déplace la résonance d'environ −150 MHz à géométrie fixe — **à vérifier par mesure**.
