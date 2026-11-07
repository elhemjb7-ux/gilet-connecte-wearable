---
title: "Gilet connecté pour le trail longue distance — Dossier de conception système (SDD)"
subtitle: "Module Wearable Technologies · 5e année Génie Électronique Embarqué & Télécoms · ESPRIM 2026-2027"
author: "Étudiant : [NOM Prénom]  ·  Enseignant : Anis Kammoun"
date: "Novembre 2026"
geometry: margin=2.2cm
fontsize: 10pt
lang: fr
toc: true
toc-depth: 2
colorlinks: true
header-includes:
  - \usepackage{fancyhdr}
  - \pagestyle{fancy}
  - \fancyhead[L]{Wearable Technology — SDD gilet trail}
  - \fancyhead[R]{ESPRIM 2026-2027}
  - \fancyfoot[C]{\thepage}
---

# Légende des statuts

Pour distinguer les types d'information, ce rapport utilise les marqueurs suivants :

| Marqueur | Signification |
|---|---|
| **[EX]** | Exigence, issue de l'énoncé (référence de page indiquée) |
| **[HY]** | Hypothèse ajoutée par l'étudiant, à valider |
| **[CA]** | Valeur calculée (formule et application numérique données) |
| **[ME]** | Mesure. **Aucune mesure n'est encore disponible** dans ce rapport. |

Sources : énoncé « Wearable Technology — travail à domicile 2026 » (p. 1 à 4, dont l'extension Wokwi en p. 4) ; cours « Les antennes textiles : résumé de cours et correction du TD » (p. 1 à 10) ; cours « Module Wearable Technologies » (diapositives, A. Mersani).

# 1. Fiche synoptique et cahier des charges

## 1.1 Profil retenu et besoin

Profil **A — endurance extrême (trail / marathon)**, choisi parmi les profils de l'énoncé [EX, énoncé p. 1, « Choix 1 »]. L'axe d'innovation personnel est l'**axe 1 — bio-électromagnétisme et e-textile** [EX, énoncé p. 2, « Choix 2 »].

Le coureur a besoin d'une surveillance continue de sa charge interne (fréquence cardiaque, respiration, température cutanée) et de sa charge externe (mouvement, impacts), avec une alerte locale fiable même hors couverture réseau.

## 1.2 Exigences vérifiables

| ID | Exigence | Valeur | Source | Méthode de vérification |
|---|---|---|---|---|
| R1 | Autonomie | **> 15 h** de fonctionnement continu | [EX] énoncé p. 1, profil A | Mesure de courant par mode + test de décharge sur prototype |
| R2 | Masse du module électronique | **< 60 g** | [EX] énoncé p. 3, Étape 1.1 | Pesée du module complet (batterie incluse) |
| R3 | Latence d'alerte (capteur → buzzer/notification) | **≤ 5 s** [HY], visée ≈ 3 s | [EX] énoncé p. 3, Étape 1.1 (métrique à fixer) | Horodatage sur log + oscilloscope |
| R4 | Étanchéité à la sueur | **IP54 minimum**, IP67 visé [HY] | [EX] énoncé p. 3, Étape 1.1 | Test de projection / immersion (norme IEC 60529) |
| R5 | Fonctionnement en perte de liaison | Alerte locale active, journalisation locale | [EX] énoncé p. 4 (« L'alerte locale doit continuer à fonctionner hors connexion ») | Essai E-T4 (Wokwi) puis essai terrain |
| R6 | Sécurité biologique : DAS | **< 2,0 W/kg** (10 g) | [EX] énoncé p. 3, Étape 2.1 ; cours p. 3 | Simulation puis mesure sur fantôme (voir §5.4) |
| R7 | Seuil d'alerte pouls | **> 140 bpm** (strict) | [EX] énoncé p. 4 | Essais à 140 et 141 bpm (Wokwi) |
| R8 | Confort thermique | Pas d'échauffement cutané perceptible ; température de surface du module < 40 °C [HY] | [HY] | Thermocouple sur peau pendant 60 min d'effort |
| R9 | Lavage | Module amovible, gilet lavable sans électronique [HY] | [HY] | Essais de lavage (cycle normé) |

**Remarque sur R3.** L'énoncé demande de « préciser la latence d'alerte » sans valeur imposée. La valeur de 5 s est une hypothèse. La latence réelle est fixée par la durée d'une fenêtre d'inférence (§4.2).

# 2. Architecture système

## 2.1 Schéma bloc fonctionnel

La figure 1 présente l'architecture retenue. Les liaisons analogiques sont en trait plein, les bus numériques en tirets, et les rails d'alimentation en pointillés.

```
 Textile (corps)                         Module électronique (poche amovible, < 60 g)
 ┌──────────────────┐   ECG diff.   ┌───────────────────────────────────────────────┐
 │ Électrodes ECG   │──────────────▶│ AFE ECG (amplif., CMRR, CAN 24 bits)          │
 │ textile (x2/x3)  │               └──────────────┬────────────────────────────────┘
 ├──────────────────┤  tension      ┌──────────────▼────────────────────────────────┐
 │ Capteur élongat. │──────────────▶│ Conditionnement analogique + CAN (ADC MCU)    │
 │ thoracique       │               └──────────────┬────────────────────────────────┘
 ├──────────────────┤  I2C          ┌──────────────▼────────────────────────────────┐
 │ IMU 6/9 axes     │╌╌╌╌╌╌╌╌╌╌╌╌╌╌▶│ MCU nRF5340 (noyau applic. + réseau BLE)       │
 ├──────────────────┤  I2C/1-wire   │  - horodatage commun, fenêtrage               │
 │ Temp. cutanée    │╌╌╌╌╌╌╌╌╌╌╌╌╌╌▶│  - prétraitement, inférence TinyML (INT8)      │
 ├──────────────────┤  I2C          │  - journalisation locale (flash)              │
 │ PPG (optionnel)  │╌╌╌╌╌╌╌╌╌╌╌╌╌╌▶└──────┬─────────────────┬──────────────┬───────┘
 └──────────────────┘                      │ BLE 5.x          │ GPIO         │ SPI
                                           ▼                  ▼              ▼
                                  Téléphone / montre     Buzzer + LED   Flash/journal
                                  (passerelle, relais)   (alerte locale)
                                           │
                                           ▼ 4G / Wi-Fi
                                  Cloud (MQTT/TLS) — tableau de bord coach
Alimentation : Li-Po 300 mAh → PMIC (charge sécurisée, LDO 3,3 V pour MCU/capteurs,
rail analogique 3,3 V filtré séparé). Rails : VBAT (3,0–4,2 V), 3V3_D, 3V3_A.
```

*Figure 1 — Schéma bloc du gilet (valeurs de rails [HY] ; à confirmer par la fiche PMIC retenue).*

## 2.2 Synchronisation des acquisitions

Toutes les voies physiologiques et biomécaniques sont échantillonnées depuis une **horloge unique du MCU** [HY]. Chaque échantillon reçoit un horodatage à partir du même compteur. Pour les capteurs à sortie numérique (IMU, température), le MCU lit la donnée à l'instant de déclenchement et l'associe à l'horodatage courant. Pour l'ECG, le CAN de l'AFE est cadencé par une horloge fournie au MCU, de sorte que les fréquences sont liées. L'erreur de synchronisation visée est **inférieure à 10 ms** [HY], soit une fraction de la période du pouls. Cette valeur doit être **vérifiée** par un signal de test commun (impulsion envoyée à toutes les voies). [ME : non réalisé]

## 2.3 Interfaces et bus

| Interface | Usage | Remarque |
|---|---|---|
| I2C (400 kHz) | IMU, capteur de température | Pull-ups à calculer (≈ 4,7 kΩ [HY]) |
| SPI | AFE ECG | Fréquence de l'AFE à reporter de sa fiche |
| UART (ou SWD) | Débogage | Connecteur de test uniquement |
| BLE 5.x | Liaison vers téléphone | Voir §5 |
| Analogique différentiel | ECG (3 électrodes) | Entrée protégée (ESD) |

# 3. Instrumentation

## 3.1 Électrodes ECG textiles

**Choix** : électrodes sèches en tissu conducteur (Shieldex ou équivalent [HY]), placées sur la sangle thoracique. Une électrode de référence (jambe droite) et deux électrodes de mesure en configuration Einthoven I [HY].

**Dimensionnement** [CA]. Pour une électrode sèche de surface *S*, l'impédance de contact est dominée par la capacité peau-électrode *C* = ε₀ε_r S / d. En prenant *d* = 50 µm, ε_r ≈ 100 (couche de sueur, [HY]) et *S* = 25 cm² (5 × 5 cm) :

$$C = \frac{8{,}85\times10^{-12} \times 100 \times 25\times10^{-4}}{50\times10^{-6}} \approx 44\ \text{nF}$$

L'impédance à 1 Hz vaut *Z* = 1/(2π·1 Hz·44 nF) ≈ **3,6 MΩ**. Cette valeur élevée impose une entrée à très forte impédance (≥ 1 GΩ [HY]) dans l'AFE, sinon l'ECG est atténué. Elle justifie aussi le choix d'une surface d'au moins 25 cm². **[ME] : impédance réelle à mesurer sur un patient.**

**Sensibilité à la sueur.** La sueur augmente ε_r et réduit *d*, donc *C* augmente et *Z* diminue. C'est favorable au signal, mais augmente le bruit de mouvement (voir §3.4).

## 3.2 Capteur d'élongation thoracique

Un ruban élastique conducteur (capteur résistif ou capacitif [HY]) mesure la variation de périmètre thoracique pendant la respiration. La fréquence respiratoire est extraite par filtrage passe-bande 0,1–1 Hz [HY]. Le capteur est placé sous la sangle, au niveau du thorax inférieur, là où l'amplitude respiratoire est maximale.

## 3.3 Capteur de température cutanée

Capteur numérique (I2C) ou thermistance NTC [HY], placé au contact de la peau sous le dos (zone peu exposée au mouvement). **Précision visée** : ±0,2 °C [HY]. La mesure est à prendre comme **tendance**, car le contact cutané varie avec la transpiration.

## 3.4 IMU

Centrale 6 axes (accéléromètre + gyroscope) [HY], placée au centre du dos, rigidement fixée au gilet. **Fréquence d'échantillonnage** : 100 Hz pour la biomécanique, 25 Hz en veille [HY]. **Plage** : ±8 g, ±1000 °/s [HY].

## 3.5 Conditionnement analogique et CMRR

L'AFE ECG (type ADS1292R de Texas Instruments [HY], à confirmer) combine amplification (gain 6 à 12 [HY]), filtre passe-bande 0,5–40 Hz [HY] et conversion sigma-delta 24 bits. Le **taux de réjection du mode commun (CMRR)** doit être supérieur à 100 dB pour limiter l'interférence 50 Hz [HY]. La valeur réelle est à reporter de la fiche constructeur. [ME] : non mesurée.

**Fréquence d'échantillonnage ECG** : 250 Hz [HY], suffisante pour le rythme cardiaque (Nyquist à 125 Hz).

# 4. Électronique et Edge AI

## 4.1 Choix du microcontrôleur

Le **nRF5340** (Nordic Semiconductor) est retenu [HY]. Critère décisif : **BLE 5.x intégré à faible consommation**, nécessaire pour la liaison avec le téléphone. Le compromis accepté est la puissance de calcul modeste (Cortex-M33 à 128 MHz [HY]) : le modèle d'inférence doit donc être très léger. Les alternatives (STM32U5, ESP32-S3) sont écartées car elles n'ont pas de BLE à aussi faible consommation, ou consomment davantage en veille.

## 4.2 Tâche d'inférence locale

**Tâche retenue** : classer une fenêtre de 10 s en trois classes : *rythme normal*, *tachycardie soutenue* (> 140 bpm pendant la fenêtre), *artefact de mouvement* [HY].

**Entrées du modèle** :
- 1 voie ECG, 250 Hz × 10 s = 2 500 échantillons ;
- 3 voies accélération (IMU), 100 Hz × 10 s = 1 000 échantillons par voie ;
- fenêtre glissante de 3 s, pas de 1 s (recouvrement de 2 s) [HY] : soit 750 échantillons ECG et 300 échantillons par voie d'accélération.

**Prétraitement** : filtre passe-bande 0,5–40 Hz, puis normalisation par fenêtre. Extraction de caractéristiques : intervalles RR et leur écart-type, énergie des fenêtres d'accélération.

**Modèle** : petit réseau 1D-CNN (2 couches de convolution, 8 et 16 filtres, 1 couche dense) [HY], quantifié en **INT8**.

**Budget mémoire et temps** [CA et HY] :
- paramètres : environ 6 000 poids → ≈ 6 kB en INT8 ;
- activations : environ 10 kB pour une fenêtre de 3 s ;
- mémoire totale visée : < 64 kB RAM sur les 256 kB disponibles [HY] ;
- temps d'inférence estimé : ≈ 40 ms par fenêtre [HY], exécutée toutes les secondes. **[ME] : non mesuré.**

**Critère de déclenchement** : alerte dès qu'une fenêtre est classée *tachycardie soutenue* [HY]. Une décision sur une seule fenêtre évite d'ajouter une seconde fenêtre de latence. La latence de détection est donc d'environ la durée d'une fenêtre, soit **≈ 3,04 s** (3 s de fenêtre + 40 ms d'inférence) [CA], ce qui respecte R3 (≤ 5 s). Le risque de fausse alerte augmente avec un critère à une fenêtre ; il sera mesuré (§4.3).

## 4.3 Vérification de l'alerte sur données annotées

La vérification s'appuie sur des bases publiques annotées (par exemple MIT-BIH Arrhythmia Database, PhysioNet [HY]) pour l'ECG, et sur des enregistrements d'effort réalisés par l'étudiant pour le mouvement. On calcule la **sensibilité** (détections correctes parmi les tachycardies réelles) et la **spécificité** (rejets corrects parmi les rythmes normaux) sur un jeu de test séparé.

**Performances restant à mesurer** : sensibilité, spécificité, latence réelle, consommation de l'inférence, et comportement face à l'artefact de mouvement pendant la course. Aucune de ces valeurs n'est disponible à la date de ce rapport. [ME : non réalisé]

# 5. Télécoms et antenne

## 5.1 Liaison retenue

**BLE 5.x** entre le module et le téléphone, qui sert de passerelle vers le cloud via 4G ou Wi-Fi [HY]. Critère décisif : faible consommation et disponibilité sur tous les téléphones. Le compromis accepté est une portée limitée (quelques mètres), suffisante puisque le coureur porte son téléphone sur lui [HY]. Pour le cas de la perte de liaison, le module stocke localement et l'alerte reste active (R5).

Le **téléphone** joue le rôle de passerelle : il relaie les données vers le cloud, affiche les alertes et stocke l'historique. Le **module** ne dépend pas du téléphone pour la détection d'alerte.

## 5.2 Antenne textile patch à 2,45 GHz

**Substrat** : feutrine, εr = 1,22, tan δ = 0,016, h = 2 mm [EX, cours p. 4 : « Matériaux adoptés »]. Critère décisif : faible permittivité et faibles pertes. La feutrine a une bande passante suffisante avec son épaisseur de 2 mm.

**Conducteur** : électro-textile Zelt, σ = 1,749 × 10⁵ S/m [EX, cours p. 4]. Le conducteur est choisi pour sa faible résistance de surface et sa tenue mécanique.

**Dimensions initiales** du patch [CA] selon les formules de Hammerstad-Jensen (**hypothèse : les formules ne figurent pas dans les supports fournis**) :

1. Largeur : W = c / (2 f₀) · √(2/(εr+1)) = 3,00·10⁸ / (2 × 2,45·10⁹) × √(2/2,22) ≈ **58,1 mm**

2. Permittivité effective : ε_eff = (εr+1)/2 + (εr−1)/2 · (1+12h/W)^(−1/2) ≈ **1,2025**

3. Extension ΔL = 0,412 h · [(ε_eff+0,3)(W/h+0,264)] / [(ε_eff−0,258)(W/h+0,8)] ≈ **1,287 mm**

4. Longueur : L = c / (2 f₀ √ε_eff) − 2ΔL = 55,8 mm − 2,57 mm ≈ **53,3 mm**

**Plan de masse et placement** [HY] : plan de masse de dimensions au moins égales au patch plus une marge de λ/4 sur chaque côté (λ ≈ 122 mm dans l'air, soit environ 111 mm dans la feutrine). Le patch est placé sur la face arrière du gilet, côté dos, où l'effet du corps est le plus contrôlé. L'alimentation est une sonde coaxiale ou une alimentation par encoche (inset feed) [HY].

## 5.3 Effet du corps et de l'humidité

Le corps humain a une permittivité très élevée (≈ 52 pour le muscle, valeur de l'énoncé p. 3). Plus le tissu est proche du patch, plus le champ est perturbé. Trois effets sont attendus :
- **décalage de la fréquence de résonance** (le corps augmente la capacité effective) ;
- **baisse du rendement** (pertes dans les tissus) ;
- **modification du diagramme**, qui se déforme vers le corps, et réduction du gain vers l'avant.

La transpiration augmente la permittivité de la feutrine et son tan δ. L'effet est étudié au §7.

## 5.4 DAS : cible et méthode de vérification

**Cible** : **DAS < 2,0 W/kg** pour 10 g de tissu [EX, énoncé p. 3]. Le cours (p. 3) donne la même valeur pour l'Europe et l'Australie (2 W/kg sur 10 g), et 1,6 W/kg sur 1 g pour les États-Unis et le Canada. Le rapport retient la limite européenne, puisque le projet se situe en Tunisie [EX, cours p. 3 : code des télécommunications, art. 8, homologation].

**Méthode** :
1. **Simulation** dans CST Studio (logiciel de l'enseignement, énoncé p. 4 du cours « Séance 1 ») avec un modèle de corps (fantôme) à 1 mm de l'antenne, puissance d'entrée réglée au maximum [HY].
2. **Mesure** : banc DAS avec fantôme de tête ou de torse, sonde et robot (cours p. 3, figure 3) [EX]. La mesure sur fantôme est la référence réglementaire.

**[ME] : non réalisé.** Le résultat de la simulation n'est pas disponible à la date de ce rapport.

# 6. Énergie et intégration

## 6.1 Matrice de consommation par mode

Les valeurs ci-dessous sont des **hypothèses de dimensionnement** [HY]. Elles doivent être remplacées par les valeurs des fiches constructeur (nRF5340, ADS1292R, IMU) et par des mesures au ampèremètre.

| Mode | Courant (mA) | Tension (V) | Part du temps | Fréquence d'activation |
|---|---|---|---|---|
| Sommeil profond (MCU + capteurs en veille) | 0,05 | 3,3 | 58 % | Permanent |
| Acquisition ECG/PPG/IMU + traitement | 12 | 3,3 | 30 % | 250 Hz / 100 Hz en continu |
| Inférence Edge AI (fenêtre 3 s, ~40 ms) | 25 | 3,3 | 4 % | Toutes les 1 s |
| Transmission BLE en rafale | 45 | 3,3 | 8 % | Toutes les 10 s, 80 ms |

## 6.2 Calcul de la capacité de batterie

Courant moyen [CA] :

$$I_{moy} = \sum_i I_i \cdot f_i = 0{,}05\cdot0{,}58 + 12\cdot0{,}30 + 25\cdot0{,}04 + 45\cdot0{,}08 = 8{,}23\ \text{mA}$$

Énergie pour 15 h, avec rendement du PMIC η = 0,85 [HY] :

$$C_{min} = \frac{I_{moy} \cdot t}{\eta} = \frac{8{,}23 \times 15}{0{,}85} \approx 145\ \text{mAh}$$

Avec une marge explicite de **30 %** [HY] pour vieillissement, température et incertitude sur les courants, la capacité requise est d'environ **189 mAh**. **Batterie retenue : Li-Po 300 mAh** [HY], soit un facteur de 1,6 par rapport à ce besoin, ce qui donne une autonomie d'environ 30 h dans les hypothèses (300 mAh × 0,85 / 8,23 mA). L'autonomie réelle est à **mesurer** [ME : non réalisé].

## 6.3 Recharge et sécurité

Recharge par PMIC avec charge en courant constant puis en tension constante (CC/CV) [HY], à 0,5 C maximum. Protection contre la surcharge, la décharge profonde et la température (sonde NTC). La recharge se fait **hors du gilet** : le module est retiré et placé sur un chargeur, pour éviter tout échauffement de la batterie contre la peau.

## 6.4 Échauffement au contact de la peau

Le module dissipe en moyenne quelques dizaines de mW (8,23 mA × 3,3 V ≈ 27 mW en moyenne, pics à environ 150 mW pendant les rafales BLE [CA]). La dissipation est faible, mais le module est posé directement sur le dos. La température de surface visée est inférieure à 40 °C [HY, R8]. **[ME] : non mesuré.** La protection passe par un capot en silicone et un écart d'environ 5 mm [HY].

## 6.5 Retrait du module et lavage

Le module est **amovible** : il se clipse dans une poche étanche par connecteur magnétique IP67 [HY]. Le gilet est lavé sans électronique. Ce choix évite de soumettre les composants électroniques aux cycles de lavage, et rend le gilet lavable en machine (cycle à 30 °C, sans essorage). Le compromis accepté est un plus grand nombre de composants à assembler et une interface à entretenir.

# 7. Innovation personnelle — axe 1 : antenne et humidité

## 7.1 Sensibilité à la permittivité

À géométrie fixe (W = 58,1 mm, L = 53,3 mm), on recalcule le patch pour chaque valeur de εr [CA]. Les valeurs de la colonne de droite donnent la longueur qu'il faudrait pour rester à 2,45 GHz.

| εr du substrat | W (mm) | ε_eff | ΔL (mm) | L (mm) pour 2,45 GHz | Lecture |
|---|---|---|---|---|---|
| 1,22 (sec) | 58,11 | 1,2025 | 1,287 | 53,26 | Référence |
| 1,30 | 57,09 | 1,2759 | 1,252 | 51,70 | Humidité modérée [HY] |
| 1,40 | 55,89 | 1,3673 | 1,215 | 49,93 | Humidité élevée [HY] |
| 1,60 | 53,70 | 1,5494 | 1,157 | 46,87 | Scénario extrême [HY] |

## 7.2 Décalage de fréquence à géométrie fixe

À géométrie fixe (L = 53,3 mm), la fréquence varie approximativement en 1/√ε_eff [CA]. Passer de ε_eff = 1,2025 (sec) à 1,3673 (εr = 1,40) donne :

$$f \approx 2{,}45 \times \sqrt{\frac{1{,}2025}{1{,}3673}} \approx 2{,}30\ \text{GHz}$$

soit environ −6 % (≈ −150 MHz). Il s'agit d'une estimation d'ordre de grandeur, à confirmer par simulation.

## 7.3 Séparer humidité et courbure

L'effet de l'humidité et celui de la courbure doivent être étudiés séparément [EX, énoncé p. 1, « Innovation personnelle »]. Le modèle analytique ne traite que l'humidité (εr) ; la courbure n'y est pas représentée.

**Tableau de sensibilité** (plan d'essais) :

| Facteur | Niveaux | Grandeur mesurée | Contribution attendue |
|---|---|---|---|
| Humidité | Sec / humide (sprayé, pesée de la teneur en eau) | f₀, S11, gain | Décalage vers le bas, perte de rendement |
| Courbure | Plat / cylindre de rayon 80 mm [HY] | f₀, S11, diagramme | Décalage et déformation du diagramme |

**Essai comparatif proposé** [HY] :
1. Mesure S11 de l'antenne à plat, à sec.
2. Mesure S11 sur support courbe (cylindre de 80 mm de rayon), à sec.
3. Mesure S11 à plat, sur feutrine humidifiée à teneur en eau connue.
4. Mesure S11 sur support courbe, humide.

L'écart entre les étapes 1 et 2 isole la **contribution de la courbure**. L'écart entre 1 et 3 isole celle de l'**humidité**. L'étape 4 teste l'effet combiné.

## 7.4 Ce que le modèle analytique permet de conclure

Le modèle permet de **dimensionner** le patch et d'estimer l'ordre de grandeur de la sensibilité à εr. Il ne permet **pas** de conclure sur :
- le gain et le diagramme réels (le modèle suppose un plan infini et un plan de masse idéal) ;
- l'effet de la courbure (non modélisé) ;
- les pertes dues à la transpiration, qui dépendent de tan δ et pas seulement de εr.

Les résultats utilisables sont donc : **une variation de εr de 1,22 à 1,40 déplace la résonance d'environ −150 MHz à géométrie fixe**, sous réserve de validation par mesure.

# 8. Démonstrateur Wokwi

Le démonstrateur est décrit dans le dépôt, dans `wokwi-gilet/`. Il utilise un ESP32, un DHT22, un potentiomètre (pouls simulé), un buzzer et une publication MQTT. Les seuils sont : pouls strictement supérieur à 140 bpm, et température hors de la plage [35,0 ; 38,5] °C [HY, hypothèse de démonstration].

**Ce que Wokwi démontre** : la logique de seuils, l'indépendance de l'alerte locale vis-à-vis du réseau, le format JSON avec unités, la reconnexion logicielle.

**Ce que Wokwi ne démontre pas** : la qualité du signal ECG, l'antenne, le DAS et l'autonomie. Pour l'ECG, la validation passe par un enregistrement comparé à un ECG de référence. Pour l'antenne, par la mesure du S11 au VNA et du diagramme en chambre anéchoïque. Pour le DAS, par la mesure sur fantôme. Pour l'autonomie, par la mesure de courant par mode puis un essai de décharge.

**État des essais** : le code et le plan d'essais sont prêts. **Les essais Wokwi n'ont pas été exécutés** à la date de ce rapport. Les colonnes « résultat observé » restent vides, comme le demande l'énoncé.

# 9. Traçabilité et limites

La matrice complète (exigence → section → preuve) est dans `docs/01_matrice_tracabilite.md`. Les principales limites sont :

- **Formules de patch** : les formules de Hammerstad-Jensen ne figurent pas dans les supports fournis ; elles doivent être confirmées avec l'enseignant.
- **Valeurs de composants** : les courants et les caractéristiques des AFE, IMU et PMIC sont des hypothèses à remplacer par les fiches constructeur. Les références exactes des fiches sont à ajouter en bibliographie.
- **Aucune mesure** dans ce rapport : ECG, S11, DAS, autonomie, latence et précision de la température restent à mesurer.
- **DAS** : la cible retenue est celle de l'Europe (2 W/kg sur 10 g), à justifier au regard de l'énoncé.

# Références

1. Énoncé « Wearable Technology — travail à domicile 2026 », ESPRIM, pages 1 à 4.
2. Cours « Les antennes textiles : résumé de cours et correction du TD », pages 1 à 10.
3. Diapositives « Module : Wearable Technologies », A. Mersani, ESPRIM 2025-2026.
4. D. M. Pozar, *Microwave Engineering* — formules de conception des antennes microruban (à citer précisément si retenues).
5. Hammerstad E., Jensen O., « Accurate models for microstrip computer-aided design », IEEE MTT-S, 1980.
6. Fiches constructeur : nRF5340 (Nordic Semiconductor), ADS1292R (Texas Instruments), IMU 6 axes retenue [à compléter].
7. Base de données MIT-BIH Arrhythmia, PhysioNet. [À vérifier avant la remise.]
