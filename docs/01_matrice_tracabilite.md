# Matrice de traçabilité — énoncé → section → preuve

Sources : énoncé « Wearable_Technology-travaildomicile2026_avec_Simulation.pdf » (p. 1 à 4), cours « Antennes-textiles+tdcorrigé.pdf » (p. 1 à 9).
Statut : **Exigence** = tirée de l'énoncé · **Hypothèse** = ajoutée par l'étudiant · **Calculé** = résultat de calcul · **Mesuré** = obtenu par essai.

| # | Exigence de l'énoncé (page) | Section du rapport | Preuve de vérification | Statut actuel |
|---|---|---|---|---|
| E1 | Acquisition synchrone physiologique + biomécanique (p. 1, §1) | §2 Architecture | Diagramme de synchronisation + horodatage commun | À rédiger |
| E2 | Analyse locale (Edge computing) (p. 1, §1) | §4 Edge AI | Essai sur données annotées (à réaliser) | À mesurer |
| E3 | Communication sans fil, antenne textile intégrée (p. 1, §1) | §5 Télécoms | S11, diagramme (chambre anéchoïque ou simulation CST) | À mesurer |
| E4 | Ergonomie, sécurité biologique, gestion thermique (p. 1, §1) | §6 Énergie/intégration | Mesure d'échauffement contact peau ; DAS | À mesurer |
| E5 | Autonomie > 15 h, profil A (p. 1, §2) | §6 Bilan énergétique | Calcul (§6) puis mesure de courant sur prototype | Calculé / à mesurer |
| E6 | Masse du module < 60 g (énoncé p. 1, §2 et p. 3 Étape 1.1) | §1 CdC | Pesée du module complet | À mesurer |
| E7 | Latence d'alerte (p. 1 Étape 1.1) | §1 CdC, §4 | Horodatage capteur → buzzer (oscilloscope ou log) | À mesurer |
| E8 | Résistance à la sueur, IP54/IP67 (p. 3 Étape 1.1) | §1 CdC, §7 | Test d'immersion / brouillard salin (à planifier) | À mesurer |
| E9 | Fonctionnement en cas de perte de liaison (p. 1 §2) | §5, §8 Démonstrateur | Essai Wokwi « NET=DOWN » (E-T5) | Non exécuté |
| E10 | Filtrage d'artefacts, classification effort/fatigue (p. 1 §1) | §4 | Comparaison LMS / auto-encodeur (axe 2 non retenu) | Hors axe |
| E11 | Antenne 2,45 GHz, substrat textile (p. 3 Étape 2.1) | §5, §7 Innovation | Calcul dimensions (calc_patch) + sensibilité | Calculé |
| E12 | DAS < 2,0 W/kg (p. 3 Étape 2.1 ; cours p. 3 : 2 W/kg sur 10 g, Europe/Australie) | §5 | Simulation DAS (CST) + mesure banc (cours p. 3, fig. 3) | À simuler |
| E13 | Bilan de puissance, matrice de modes, batterie Li-Po/LiFePO4 + PMIC (p. 3 Étape 2.2) | §6 | Calcul batterie | Calculé |
| E14 | Schéma synoptique (I2C, SPI, UART, I2S, liaisons différentielles, alimentation) (p. 4 §1.4) | §2 | Schéma bloc | À dessiner |
| E15 | Démonstrateur Wokwi : ESP32, DHT22, potentiomètre, buzzer, MQTT (p. 4) | §8 + `wokwi-gilet/` | Essais E-T1 à E-T5 (README) | Code prêt, essais non exécutés |
| E16 | Seuil pouls strictement > 140 bpm (p. 4) | `sketch.ino` (`BPM_MAX`) | Essais à 140 et 141 bpm | Code prêt |
| E17 | Dépôt GitHub avec rapport, code, données, README (p. 4 §3) | `README.md`, arborescence | Lien GitHub | À publier |
| E18 | Rapport 10 à 12 pages, 7 rubriques (p. 3 §4) | Rapport PDF | Export PDF, nombre de pages | À rédiger |
| E19 | Innovation : calcul dimensions patch, sensibilité humidité/courbure, essai comparatif (p. 1 §Innovation de l'énoncé) | §7 | `docs/02_calcul_patch.md` | Calculé (sans humidité/courbure mesurées) |

## Remarques de cohérence
- **Seuil DAS** : l'énoncé fixe « < 2,0 W/kg » ; le cours (p. 3) donne 2 W/kg sur 10 g pour l'Europe et 1,6 W/kg sur 1 g pour les États-Unis. Le rapport retient 2 W/kg sur 10 g, et le justifie.
- **Matériaux** : l'énoncé et le cours (p. 4) retiennent le même substrat (feutrine, εr = 1,22, tan δ = 0,016, h = 2 mm) et le même conducteur (Zelt, σ = 1,749 × 10⁵ S/m).
- **Hypothèse de plage de température** du démonstrateur : [35,0 ; 38,5] °C, présentée comme hypothèse de démonstration, configurable dans `sketch.ino`.
