# Gilet connecté — travail individuel Wearable Technologies (ESPRIM, 5e année EELT, 2026-2027)

Projet : gilet connecté pour le trail longue distance (profil A), axe 1 : antenne patch textile à 2,45 GHz et effets de l'humidité et de la courbure.
Enseignant : Anis Kammoun (énoncé) · Cours : Ameni Mersani.

## Arborescence
```
gilet-wearable-wt/
├── README.md                     ce fichier
├── docs/
│   ├── 01_matrice_tracabilite.md énoncé → section → preuve
│   └── 02_calcul_patch.md        dimensions du patch, hypothèses, sensibilité
├── wokwi-gilet/
│   ├── sketch.ino                firmware ESP32 (seuils, MQTT, alerte locale)
│   ├── diagram.json              schéma de câblage Wokwi
│   └── libraries.txt             bibliothèques Arduino
├── data/                         résultats d'essais (à compléter, voir §Essais)
└── rapport/                      rapport PDF (à déposer)
```

## Lancer la simulation Wokwi
1. Ouvrir https://wokwi.com, créer un projet « ESP32 » (ou importer le dossier `wokwi-gilet/`).
2. Remplacer `sketch.ino`, `diagram.json` et `libraries.txt` par les fichiers du dossier.
3. Vérifier que `libraries.txt` est pris en compte (PubSubClient, DHT sensor library, ArduinoJson).
4. Cliquer sur **Start the simulation**.
5. Ouvrir le moniteur série (115200 bauds). Le log affiche le pouls, la température et l'état d'alerte toutes les 2 s.
6. Observer les messages MQTT : abonnez-vous au topic `gilet-demo/athlete01/telemetry` avec un client MQTT (par ex. MQTT Explorer) sur `test.mosquitto.org:1883`. Ce broker est public : ne pas y envoyer de données personnelles.

**Lien Wokwi à insérer ici** : `[À COMPLÉTER — https://wokwi.com/projects/...]`

## Brochage
| Composant | Broche ESP32 | Remarque |
|---|---|---|
| DHT22 DATA | GPIO 15 | Pull-up interne Wokwi |
| DHT22 VCC / GND | 3V3 / GND | |
| Potentiomètre (pouls simulé) SIG | GPIO 34 (ADC1_CH6) | Entrée seule ; 40 à 200 bpm |
| Potentiomètre VCC / GND | 3V3 / GND | |
| Buzzer (+) | GPIO 25 | Buzzer actif |
| Buzzer (−) | GND | |

## Commandes de test (moniteur série)
| Commande | Effet |
|---|---|
| `BPM=140` / `BPM=141` | Force le pouls, ignore le potentiomètre |
| `BPM=AUTO` | Retour au potentiomètre |
| `TEMP=39.5` | Force la température hors plage |
| `TEMP=AUTO` | Retour au DHT22 |
| `NET=DOWN` / `NET=UP` | Simule la perte puis le rétablissement de la liaison MQTT |

## Hypothèses de démonstration (non cliniques)
- Seuil de pouls : alerte si `hr_bpm > 140` (strict), valeur de l'énoncé.
- Plage de température : `[35,0 ; 38,5] °C`, hypothèse, modifiable dans `sketch.ino`.
- Potentiomètre = capteur de pouls simulé, échelle linéaire 40 à 200 bpm (hypothèse de démonstration).

## Message JSON publié (exemple, unités incluses)
```json
{
  "device_id": "gilet-demo-01",
  "timestamp_ms": 15230,
  "hr_bpm": 141,
  "temp_c": 36.8,
  "humidity_pct": 48.2,
  "alert": true,
  "alert_reason": "pouls_eleve",
  "units": { "hr": "bpm", "temp": "degC", "humidity": "%RH", "timestamp": "ms_since_boot" }
}
```
Ce message est un **exemple de format**, non un résultat d'essai.

## Plan d'essais (E-T1 à E-T5)
Colonne « résultat observé » : **à remplir uniquement après exécution réelle**.

| ID | Manipulation | Résultat attendu | Preuve à recueillir | Résultat observé |
|---|---|---|---|---|
| E-T1 | `BPM=140`, température dans la plage | Buzzer OFF, `alert=false` | Capture du log série | Non exécuté |
| E-T2 | `BPM=141`, température dans la plage | Buzzer ON, `alert_reason=pouls_eleve` | Capture log + état du buzzer | Non exécuté |
| E-T3 | `BPM=AUTO`, `TEMP=39.5` (pouls normal) | Buzzer ON, `alert_reason=temperature_hors_plage` | Capture log | Non exécuté |
| E-T4 | Durée de l'essai E-T2 avec Wi-Fi/MQTT coupé (`NET=DOWN`) | Buzzer ON et logs continus | Capture log, aucune publication | Non exécuté |
| E-T5 | `NET=DOWN` puis `NET=UP` | Reconnexion, reprise des publications | Capture du topic MQTT | Non exécuté |

## Ce que Wokwi démontre, et ce qu'il ne démontre pas
- **Démontre** : la logique de seuils, l'indépendance de l'alerte locale vis-à-vis du réseau, le format et les unités des messages MQTT, la reconnexion logicielle.
- **Ne démontre pas** : la qualité du signal ECG textile, les performances de l'antenne, le DAS, ni l'autonomie. Il n'y a pas de matériel réel ni de couplage au corps.
- **Validation sur le gilet réel** : ECG (électrodes textiles, comparaison à un ECG de référence), antenne (S11 au VNA, diagramme en chambre anéchoïque, puis simulation CST), DAS (simulation puis mesure sur fantôme, cours p. 3, fig. 3), autonomie (mesure de courant par mode, cf. calcul §6 du rapport).

## Statut
- Code et documents : prêts.
- Essais Wokwi : **non exécutés** à la date de rédaction.
- Rapport PDF : à rédiger (10 à 12 pages, 7 rubriques).
- Lien GitHub : à publier.
